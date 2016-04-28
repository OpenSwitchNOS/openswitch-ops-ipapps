/*
 * Copyright (C) 2016 Hewlett Packard Enterprise Development LP
 * All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may
 *   not use this file except in compliance with the License. You may obtain
 *   a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 *
 * File: dhcpv6_relay_config.c
 *
 */

/*
 * Configuration maintainer for dhcpv6-relay feature.
 *
 */

#include "dhcpv6_relay.h"
#include "hash.h"


VLOG_DEFINE_THIS_MODULE(dhcpv6_relay_config);

#ifdef FTR_DHCPV6_RELAY
/*
 * Function      : dhcpv6_relay_get_server_entry
 * Responsiblity : Lookup the hash map for a specific server entry
 * Parameters    : ipv6_address - server IPv6 address
 * Return        : DHCPV6_RELAY_SERVER_T* - pointer to the server entry
 */
DHCPV6_RELAY_SERVER_T* dhcpv6_relay_get_server_entry(char* ipv6_address,
                                        char* egressIfName)
{
    uint32_t hash_ip, hash_ifName;
    DHCPV6_RELAY_SERVER_T *serverIP = NULL;
    hash_ip = hash_string(ipv6_address, 0);
    hash_ifName = hash_string(egressIfName, 0);

    CMAP_FOR_EACH_WITH_HASH(serverIP, cmap_node, hash_2words(hash_ip, hash_ifName),
                           &dhcpv6_relay_ctrl_cb_p->serverHashMap) {
        if ((0 == strncmp(ipv6_address, serverIP->ipv6_address,
            strlen(ipv6_address))) &&
            (0 == strncmp(egressIfName, serverIP->egressIfName,
            strlen(egressIfName)))) {
            return serverIP;
        }
    }

    return NULL;
}

/*
 * Function      : dhcpv6_relay_add_server_entry
 * Responsiblity : Add a server entry to server hash table
 * Parameters    : ipv6_address - server IPv6 address
 *                 egressIfName - Outgoing Interface name
 * Return        : DHCPV6_RELAY_SERVER_T* - pointer to the newly created server entry
 */
DHCPV6_RELAY_SERVER_T* dhcpv6_relay_add_server_entry(char* ipv6_address,
                                        char* egressIfName)
{
    DHCPV6_RELAY_SERVER_T *serverIP;
    uint32_t hash_ip, hash_ifName;

    serverIP = (DHCPV6_RELAY_SERVER_T *) malloc (sizeof(DHCPV6_RELAY_SERVER_T));
    if (NULL == serverIP) {
        VLOG_ERR("Failed to allocate memory for the server entry for "
                 "ipv6: %s", ipv6_address);
        return NULL;
    }

    serverIP->ipv6_address = xstrdup(ipv6_address);
    serverIP->egressIfName = xstrdup(egressIfName);
    serverIP->ref_count  = 1;      /*Reference count starts with 1*/

    hash_ip = hash_string(ipv6_address, 0);
    hash_ifName = hash_string(egressIfName, 0);

    cmap_insert(&dhcpv6_relay_ctrl_cb_p->serverHashMap, (struct cmap_node *)serverIP,
                hash_2words(hash_ip, hash_ifName));

    return serverIP;
}

/*
 * Function      : dhcpv6_relay_store_address
 * Responsiblity : Add a server reference to an interface
 * Parameters    : intfNode - Interface entry
 *                 ipv6_address - server IPv6 address
 *                 egressIfName - outgoing interface name
 * Return        : true - if the entry is successfully added
 *                 false - otherwise
 */
bool dhcpv6_relay_store_address(DHCPV6_RELAY_INTERFACE_NODE_T *intfNode,
                        char* ipv6_address, char *egressIfName)
{
    DHCPV6_RELAY_SERVER_T  *server;

    if(intfNode->addrCount >= MAX_SERVERS_PER_INTERFACE)
    {
        VLOG_ERR("Maximum server configuration limit reached on interface"
                 " %s (count : %d)", intfNode->portName, intfNode->addrCount);
        return false;
    }

    sem_wait(&dhcpv6_relay_ctrl_cb_p->waitSem);

    /* If address count is non zero but the address is NULL, return error */
    if((0 != intfNode->addrCount) && (NULL == intfNode->serverArray))
    {
        VLOG_ERR("Address count is [%d], but server IPv6 address ref array "
                 "is NULL for Interface [%s] while storing a server ref",
                 intfNode->addrCount, intfNode->portName);
        sem_post(&dhcpv6_relay_ctrl_cb_p->waitSem);
        return false;
    }

    /* There is no server table pointers for this interface, we will allocate
     * memory for this array of server entry pointers */
    else if(intfNode->serverArray == NULL)
    {
        intfNode->addrCount = 0;
        /* FIXME: Static allocation needs to be avoided.
         *     1. one proposal is to have 4 server arrays which
         *        are allocated on need basis
         *     2. Have statically allocated array of server pointer and maintain
         *        free/allocated list */
        intfNode->serverArray = (DHCPV6_RELAY_SERVER_T **)
                                    malloc(MAX_SERVERS_PER_INTERFACE
                                    * sizeof(DHCPV6_RELAY_SERVER_T *));
        if (NULL == intfNode->serverArray) {
            VLOG_ERR("Failed to allocate server array for interface : %s",
                    intfNode->portName);
            sem_post(&dhcpv6_relay_ctrl_cb_p->waitSem);
            return false;
        }

        /* Join multicast group for this interface */
        if (dhcpv6_relay_ctrl_cb_p->dhcpv6_relaySockFd == -1)
        {
            VLOG_ERR(" Socket is not yet created");
            return false;
        }
    }

    VLOG_INFO("Attempting to add server entry (ipv6: %s"
              "on interface : %s", ipv6_address, intfNode->portName);
    /* Checks whether Server IP entry exists or not */
    server = dhcpv6_relay_get_server_entry(ipv6_address, egressIfName);
    if(NULL != server)
    {
        /* Increment server entry reference count */
        server->ref_count++;
        VLOG_INFO("Matching server found, incremented ref count. "
               "server : %s (refcount :%d)", server->ipv6_address,
               server->ref_count);
    }
    else
    {
        /* No matching server entry, create new */
        if((server = dhcpv6_relay_add_server_entry(
                       ipv6_address, egressIfName)) == NULL)
        {
            VLOG_ERR("Error while adding a new server entry");
            sem_post(&dhcpv6_relay_ctrl_cb_p->waitSem);
            return false;
        }
    }

    /* Update IP reference table (per interface) */
    intfNode->serverArray[intfNode->addrCount] = server;
    /* Increment the address count in interface table */
    intfNode->addrCount++;

    VLOG_INFO("Server entry successfully updated for interface : %s, "
              " current address_count : %d", intfNode->portName,
              intfNode->addrCount);

    sem_post(&dhcpv6_relay_ctrl_cb_p->waitSem);

    return true;
}

/*
 * Function      : dhcpv6_relay_push_deleted_server_ref_to_end
 * Responsiblity : Move the deleted entry from the server array to the end
 * Parameters    : intfNode - Interface entry
 *                 deleted_index - Index in the server array of the interface
 * Return        : none
 */
void dhcpv6_relay_push_deleted_server_ref_to_end(DHCPV6_RELAY_INTERFACE_NODE_T *intfNode,
                                        int deleted_index)
{

    uint8_t currentAddrCount = 0;
    DHCPV6_RELAY_SERVER_T *server;
    DHCPV6_RELAY_SERVER_T **serverArray = intfNode->serverArray;

    currentAddrCount = intfNode->addrCount;

    /* Move the NULL at the end of the list */
    server = serverArray[currentAddrCount];
    serverArray[currentAddrCount] = serverArray[deleted_index] = NULL;
    serverArray[deleted_index] = server;
}

/*
 * Function      : dhcpv6_relay_remove_server_ref_entry
 * Responsiblity : Remove reference to a server entry from an interface entry
 * Parameters    : intfNode - Interface entry
 *                 ipv6_address - server IPv6 address
 *                 deleted_index - Index in the server array
 * Return        : true - if the entry is successfully dereferenced
 *                 false - entry not found
 */
bool dhcpv6_relay_remove_server_ref_entry(DHCPV6_RELAY_INTERFACE_NODE_T *intfNode,
                                char* ipv6_address, char* egressIfName,
                                int *deleted_index)
{
    uint8_t index = 0, hash_ip, hash_ifName;
    DHCPV6_RELAY_SERVER_T **serverArray = NULL;
    DHCPV6_RELAY_SERVER_T *server;

    serverArray = intfNode->serverArray;
    /* Find the entry in Server IP Table & free the memory (structFree) if
     * ref_count is zero */
    for ( ; index < intfNode->addrCount; index++)
    {
        if ((0 == strncmp(serverArray[index]->ipv6_address, ipv6_address,
            strlen(serverArray[index]->ipv6_address))) &&
            (0 == strncmp(serverArray[index]->egressIfName, egressIfName,
            strlen(serverArray[index]->egressIfName)))
            )
            /* check if address exist in table */
        {
            VLOG_INFO("Address to be deleted found in interface : %s",
                      intfNode->portName);
            server = serverArray[index];
            assert(server->ref_count);
            /* Decrement server reference Count */
            server->ref_count--;
            /* If reference count becomes zero */
            if(0 >= server->ref_count)
            {

                hash_ip = hash_string(ipv6_address, 0);
                hash_ifName = hash_string(egressIfName, 0);
                VLOG_INFO("server reference count reached 0. Freeing entry");
                cmap_remove(&dhcpv6_relay_ctrl_cb_p->serverHashMap,
                            (struct cmap_node *)server,
                            hash_2words(hash_ip, hash_ifName));
                /* assign outgoing interface to NULL */
                if (NULL != server->egressIfName)
                    free(server->egressIfName);
                if (NULL != server->ipv6_address)
                    free(server->ipv6_address);
                free(server);
            }

            *deleted_index = index;
            /*Make the IP ref Table entry NULL*/
            serverArray[index] = NULL;
            return true;
        }
    }
    return false;
}


/*
 * Function      : dhcpv6_relay_remove_address
 * Responsiblity : Remove a server reference from an interface
 * Parameters    : intfNode - Interface entry
 *                 ipaddress - server IPv6 address
 *                 egressIfName - outgoing interface name
 * Return        : true - if the server reference is removed
 *                 false - otherwise
 */
bool dhcpv6_relay_remove_address(DHCPV6_RELAY_INTERFACE_NODE_T *intfNode,
                        char* ipv6_address, char* egressIfName)
{
    DHCPV6_RELAY_SERVER_T **serverArray = NULL;
    bool retVal = false;
    int deleted_index;  /*IP ref table's deleted index*/
    struct shash_node *node;

    VLOG_INFO("Attempting to delete server : %s on "
              "interface : %s", ipv6_address, intfNode->portName);

    sem_wait(&dhcpv6_relay_ctrl_cb_p->waitSem);

    /* Server IP Reference table pointer */
    serverArray = intfNode->serverArray;

    retVal = dhcpv6_relay_remove_server_ref_entry(intfNode, ipv6_address,
                                        egressIfName, (int*)&deleted_index);
    if(false == retVal)
    {
        /* Release the semaphore & return */
        sem_post(&dhcpv6_relay_ctrl_cb_p->waitSem);
        VLOG_ERR("Server entry not found on the interface");
        return false;
    }

    /* Decrement interface server reference count */
    intfNode->addrCount --;

    VLOG_INFO("Interface server reference count after decrement : %d",
              intfNode->addrCount);

    /* Push the NULL entry at the end of the list
     * If address count is zero & deleted index is also zero, no need to swap
     * the deleted entry, because the entire list is NULL now.*/
    if(!((0 == deleted_index) && (0 == intfNode->addrCount)))
    {
        dhcpv6_relay_push_deleted_server_ref_to_end(intfNode, deleted_index);
    }

    /* Check whether this was the only configured server IP for the interface.
     * In that case we must free the memory for entire server table
     * for this interface. First entry NULL means, entire list is NULL.
     * because NULL entries are
     * always pushed to the end of the list */
    if ((NULL == serverArray[0]) && (intfNode->addrCount ==0))
    {
        VLOG_INFO("All configuration on the interface : %s are removed."
                  " Freeing interface entry", intfNode->portName);

        /* Delete the entire IP reference table */
        free(serverArray);
        /* Make interface table entry NULL, as no helper IP is configured */
        intfNode->serverArray = NULL;
        node = shash_find(&dhcpv6_relay_ctrl_cb_p->intfHashTable,
                      intfNode->portName);
        if (NULL != node)
        {
            shash_delete(&dhcpv6_relay_ctrl_cb_p->intfHashTable, node);
        }
        else
        {
            VLOG_ERR("Interface node not found in hash table : %s",
                 intfNode->portName);
        }
        if (NULL != intfNode->portName)
            free(intfNode->portName);

        free(intfNode);
    }
    sem_post(&dhcpv6_relay_ctrl_cb_p->waitSem);
    return true;
}


/*
 * Function      : dhcpv6_relay_create_intfnode
 * Responsiblity : Allocate memory for interface entry
 * Parameters    : pname - interface name
 * Return        : DHCPV6_RELAY_INTERFACE_NODE_T* - Interface node
 */
DHCPV6_RELAY_INTERFACE_NODE_T *dhcpv6_relay_create_intferface_node(char *pname)
{
    DHCPV6_RELAY_INTERFACE_NODE_T *intfNode = NULL;

    /* There is no server configuration available create one */
    intfNode = (DHCPV6_RELAY_INTERFACE_NODE_T *)
                    calloc(1, sizeof(DHCPV6_RELAY_INTERFACE_NODE_T));
    if (NULL == intfNode)
    {
        VLOG_ERR("Failed to allocate interface node for : %s", pname);
        return NULL;
    }

    intfNode->portName = xstrdup(pname);
    if (NULL == intfNode->portName)
    {
       VLOG_ERR("Failed to allocate memory for portName : %s", pname);
       return NULL;
    }

    intfNode->addrCount = 0;
    intfNode->serverArray = NULL;
    shash_add(&dhcpv6_relay_ctrl_cb_p->intfHashTable, pname, intfNode);
    VLOG_INFO("Allocated interface table record for port : %s", pname);

    return intfNode;
}

/*
 * Function      : dhcpv6_relay_handle_row_delete
 * Responsiblity : Process delete event for one or more ports records from
 *                 DHCP-Relay table
 * Parameters    : idl - idl reference
 * Return        : none
 */
void dhcpv6_relay_handle_row_delete(struct ovsdb_idl *idl)
{
    const struct ovsrec_dhcp_relay *rec = NULL;
    DHCPV6_RELAY_INTERFACE_NODE_T *intf = NULL;
    struct shash_node *node = NULL, *next = NULL;
    int iter = 0;
    DHCPV6_RELAY_SERVER_T servers[MAX_SERVERS_PER_INTERFACE];
    bool found = false;

    /* Walk the server configuration hash table per "port" to
     * see if the corresponding record is deleted */
    SHASH_FOR_EACH_SAFE(node, next, &dhcpv6_relay_ctrl_cb_p->intfHashTable) {
        found = false;
        /* Iterate through dhcp relay table to find a match */
        OVSREC_DHCP_RELAY_FOR_EACH(rec, idl) {
            if ((NULL != rec->port) &&
                !strncmp(rec->port->name, node->name,
                         strlen(rec->port->name))) {
                found = true;
                break;
            }
        }

        if (false == found) {
            intf = (DHCPV6_RELAY_INTERFACE_NODE_T *)node->data;
            memset(servers, 0, sizeof(servers));
            /* Delete the interface entry from hash table */
            for (iter = 0; iter < intf->addrCount; iter++) {
                dhcpv6_relay_remove_address(intf, servers[iter].ipv6_address,
                                      servers[iter].egressIfName);
            }
        }
    }
    return;
}


/*
 * Function      : dhcpv6_relay_ucast_entries_removed
 * Responsiblity : Check for ucast entries deleted in dhcp relay table
 * Parameters    : rec - DHCP-Relay OVSDB table record
 *                 intfNode - Interface entry
 *                 servers - 2D array to hold entries deleted
 * Return        : none
 */
void dhcpv6_relay_ucast_entries_removed(DHCPV6_RELAY_INTERFACE_NODE_T *intfNode,
                        const struct ovsrec_dhcp_relay *rec,
                        char servers[][INET6_ADDRSTRLEN])
{
    char *ipv6_address = NULL;
    char id[sizeof(struct in6_addr)];
    char (*arrayPtr)[INET6_ADDRSTRLEN];
    int iter, iter1, retVal;
    bool found;

    arrayPtr = servers;
    /* Collect the servers that are removed from the list */
    for (iter = 0; iter < intfNode->addrCount; iter++) {
        found = false;
        ipv6_address = xstrdup(intfNode->serverArray[iter]->ipv6_address);
        for (iter1 = 0; iter1 < rec->n_ipv6_ucast_server; iter1++) {
            retVal = inet_pton(AF_INET6, rec->ipv6_ucast_server[iter1], &id);
            if (!retVal) {
                VLOG_ERR("Invalid IP seen during server update : %s",
                        rec->ipv6_ucast_server[iter1]);
                continue;
            }
            if (0 == strncmp(ipv6_address, id, strlen(ipv6_address))) {
                found = true;
                break;
            }
        }

        if (false == found) {
            strncpy(*arrayPtr, ipv6_address, strlen(ipv6_address));
            arrayPtr++;
        }
    }
    free(ipv6_address);
}


/*
 * Function      : dhcpv6_relay_ucast_entries_added
 * Responsiblity : Check for ucast entries added in dhcp relay table
 * Parameters    : rec - DHCP-Relay OVSDB table record
 *                 intfNode - Interface entry
 *                 servers - 2D array to hold entries added
 * Return        : none
 */

void dhcpv6_relay_ucast_entries_added(DHCPV6_RELAY_INTERFACE_NODE_T *intfNode,
                        const struct ovsrec_dhcp_relay *rec,
                        char servers[][INET6_ADDRSTRLEN])
{
    char *ipv6_address = NULL;
    char id[sizeof(struct in6_addr)];
    char (*arrayPtr)[INET6_ADDRSTRLEN];
    int iter, iter1, retVal;
    bool found;

    arrayPtr = servers;
    for (iter = 0; iter < rec->n_ipv6_ucast_server; iter++) {
        found = false;
        retVal = inet_pton(AF_INET6, rec->ipv6_ucast_server[iter], &id);
        if (!retVal) {
            VLOG_ERR("Invalid IP seen during server update : %s",
                    rec->ipv6_ucast_server[iter]);
            continue;
        }

        for (iter1 = 0; iter1 < intfNode->addrCount; iter1++) {
            ipv6_address = xstrdup(intfNode->serverArray[iter]->ipv6_address);
            if (0 == strncmp(ipv6_address, id, strlen(ipv6_address))) {
                found = true;
                break;
            }
        }

        if (false == found) {
            strncpy(*arrayPtr, ipv6_address, strlen(ipv6_address));
            arrayPtr++;
        }
    }
    free(ipv6_address);
}

#if 0 /* changes to be pushed once schema changes are pushed */

/*
 * Function      : dhcpv6_relay_mcast_entries_added
 * Responsiblity : Check for mcast entries added in dhcp relay table
 * Parameters    : rec - DHCP-Relay OVSDB table record
 *                 intfNode - Interface entry
 *                 servers - 2D array to hold server ipv6 addresses added.
 *                 egressArr - 2D array to hold egress interface names added.
 * Return        : none
 */

void dhcpv6_relay_mcast_entries_added(DHCPV6_RELAY_INTERFACE_NODE_T *intfNode,
                        const struct ovsrec_dhcp_relay *rec,
                        char servers[][INET6_ADDRSTRLEN],
                        char egressArr[][IF_NAMESIZE+1])
{
    char *ipv6_address = NULL, *egressIfName = NULL, *outIfName = NULL;
    char id[sizeof(struct in6_addr)];
    char (*arrayPtr)[INET6_ADDRSTRLEN];
    char (*ifPtr)[IF_NAMESIZE+1];
    int iter, retVal;
    bool found;
    const struct smap_node *smap_node = NULL;

    arrayPtr = servers;
    ifPtr = egressArr;
    SMAP_FOR_EACH(smap_node, &rec->ipv6_mcast_server) {
        found = false;
        retVal = inet_pton(AF_INET6, smap_node->key, &id);
        if (!retVal) {
            VLOG_ERR("Invalid IP seen during server update : %s",
                    smap_node->key);
            continue;
        }

        /* get the first egressifName */
        outIfName = strtok(smap_node->value, " ");
        while(outIfName != NULL) {
            for (iter = 0; iter < intfNode->addrCount; iter++) {
                ipv6_address = xstrdup(intfNode->serverArray[iter]->ipv6_address);
                egressIfName = xstrdup(intfNode->serverArray[iter]->egressIfName);

                if ((0 == strncmp(ipv6_address, id, strlen(ipv6_address))) &&
                    (0 == strncmp(egressIfName, outIfName, strlen(egressIfName)))) {
                    found = true;
                    break;
                }
                if (false == found) {
                    strncpy(*arrayPtr, ipv6_address, strlen(ipv6_address));
                    strncpy(*ifPtr, outIfName, strlen(outIfName));
                    arrayPtr++;
                    ifPtr++;
                }
            }
            outIfName = strtok(NULL, " ");
        }
    }
    free(ipv6_address);
    free(egressIfName);
}

/*
 * Function      : dhcpv6_relay_mcast_entries_removed
 * Responsiblity : Check for ucast entries deleted in dhcp relay table
 * Parameters    : rec - DHCP-Relay OVSDB table record
 *                 intfNode - Interface entry
 *                 servers - 2D array to hold server ipv6 addresses added.
 *                 egressArr - 2D array to hold egress interface names added.
 * Return        : none
 */
void dhcpv6_relay_mcast_entries_removed(DHCPV6_RELAY_INTERFACE_NODE_T *intfNode,
                        const struct ovsrec_dhcp_relay *rec,
                        char servers[][INET6_ADDRSTRLEN],
                        char egressArr[][IF_NAMESIZE+1])
{
    char *ipv6_address = NULL, *egressIfName = NULL, *outIfName = NULL;
    char id[sizeof(struct in6_addr)];
    char (*arrayPtr)[INET6_ADDRSTRLEN];
    char (*ifPtr)[IF_NAMESIZE+1];
    int iter, iter1, retVal;
    bool found;
    const struct smap_node *smap_node = NULL;
    char tempServers[MAX_SERVERS_PER_INTERFACE][INET6_ADDRSTRLEN];
    char tempEgressArr[MAX_SERVERS_PER_INTERFACE][IF_NAMESIZE+1];

    memset(tempServers, 0, sizeof(tempServers));
    memset(tempEgressArr, 0, sizeof(tempEgressArr));

    ifPtr = tempEgressArr;
    arrayPtr = tempServers;
    /* collect all the values from ovsdb */
    SMAP_FOR_EACH(smap_node, &rec->ipv6_mcast_server) {
        retVal = inet_pton(AF_INET6, smap_node->key, &id);
        if (!retVal) {
            VLOG_ERR("Invalid IP seen during server update : %s",
                 smap_node->key);
            continue;
        }

        /* get the first egressifName */
        outIfName = strtok(smap_node->value, " ");
        while(outIfName != NULL) {
            strncpy(*arrayPtr, id, strlen(id));
            strncpy(*ifPtr, outIfName, strlen(outIfName));
            arrayPtr++;
            ifPtr++;
            outIfName = strtok(NULL, " ");
        }
    }

    ifPtr = egressArr;
    arrayPtr = servers;
    /* Collect the mcast servers that are removed from the list */
    for (iter = 0; iter < intfNode->addrCount; iter++) {
        found = false;
        ipv6_address = xstrdup(intfNode->serverArray[iter]->ipv6_address);
        egressIfName = xstrdup(intfNode->serverArray[iter]->egressIfName);
        for (iter1 = 0; tempServers[iter1] != 0; iter1++) {
            if ((0 == strncmp(ipv6_address, tempServers[iter1], strlen(ipv6_address))) &&
                (0 == strncmp(egressIfName, tempEgressArr[iter1], strlen(egressIfName)))) {
                found = true;
                break;
            }
        }
        if (false == found) {
            strncpy(*arrayPtr, ipv6_address, strlen(ipv6_address));
            strncpy(*ifPtr, egressIfName, strlen(egressIfName));
            arrayPtr++;
            ifPtr++;
        }
    }
    free(ipv6_address);
    free(egressIfName);
}
#endif

/*
 * Function      : dhcpv6_relay_handle_config_change
 * Responsiblity : Handle a record change in DHCP-Relay table
 * Parameters    : rec - DHCP-Relay OVSDB table record
 *                 idl_seqno - idl change identifier
 * Return        : none
 */
void dhcpv6_relay_handle_config_change(
              const struct ovsrec_dhcp_relay *rec, uint32_t idl_seqno)
{
    char *portName = NULL, *egressIfName = NULL;
    int iter;
    struct shash_node *node;
    DHCPV6_RELAY_INTERFACE_NODE_T *intfNode = NULL;
    char servers[MAX_SERVERS_PER_INTERFACE][INET6_ADDRSTRLEN];
#if 0 /* changes to be pushed once schema changes are pushed */
    char egressArr[MAX_SERVERS_PER_INTERFACE][IF_NAMESIZE+1];
#endif

    if ((NULL == rec) ||
        (NULL == rec->port) ||
        (NULL == rec->vrf)) {
        return;
    }

    portName = rec->port->name;

    /* Do lookup for the interface entry in hash table */
    node = shash_find(&dhcpv6_relay_ctrl_cb_p->intfHashTable, portName);
    if (NULL == node) {
       /* Interface entry not found, create one */
       if (NULL == (intfNode = dhcpv6_relay_create_intferface_node(portName)))
       {
           return;
       }
    }
    else
    {
        intfNode = (DHCPV6_RELAY_INTERFACE_NODE_T *)node->data;
    }

    if (OVSREC_IDL_IS_COLUMN_MODIFIED(ovsrec_dhcp_relay_col_ipv6_ucast_server,
                               idl_seqno)) {

        memset(servers, 0, sizeof(servers));
        dhcpv6_relay_ucast_entries_removed(intfNode, rec, servers);
        /* Delete the ucast ipv6 servers that were removed in the config update */
        for (iter = 0; servers[iter] != 0; iter++) {
            dhcpv6_relay_remove_address(intfNode, servers[iter], egressIfName);
        }

        memset(servers, 0, sizeof(servers));
        dhcpv6_relay_ucast_entries_added(intfNode, rec, servers);
        /* Create the newly added ucast ipv6 servers */
        for (iter = 0; servers[iter] != 0; iter++) {
            dhcpv6_relay_store_address(intfNode, servers[iter], egressIfName);
        }
    }
#if 0 /* changes to be pushed once schema changes are pushed */

    if (OVSREC_IDL_IS_COLUMN_MODIFIED(ovsrec_dhcp_relay_col_ipv6_mcast_server,
                           idl_seqno)) {
        memset(servers, 0, sizeof(servers));
        memset(egressArr, 0, sizeof(egressArr));
        dhcpv6_relay_mcast_entries_removed(intfNode, rec, servers, egressArr);
        /* Delete the mcast ipv6 servers that were removed in the config update */
        for (iter = 0; servers[iter] != 0; iter++) {
            dhcpv6_relay_remove_address(intfNode, servers[iter], egressArr[iter]);
        }

        memset(servers, 0, sizeof(servers));
        memset(egressArr, 0, sizeof(egressArr));
        dhcpv6_relay_mcast_entries_added(intfNode, rec, servers, egressArr);
        /* Create the newly added mcast ipv6 servers */
        for (iter = 0; servers[iter] != 0; iter++) {
            dhcpv6_relay_store_address(intfNode, servers[iter], egressArr[iter]);
        }
    }
#endif
    return;
}

#endif /* FTR_DHCPV6_RELAY */

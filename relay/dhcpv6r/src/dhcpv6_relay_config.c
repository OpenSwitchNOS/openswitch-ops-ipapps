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
#include <string.h>

VLOG_DEFINE_THIS_MODULE(dhcpv6_relay_config);

#ifdef FTR_DHCPV6_RELAY

#define GENERATE_KEY(HASH_IP, HASH_EGRESSNAME, HASH_KEY) \
    if (HASH_EGRESSNAME) \
        HASH_KEY = hash_2words(hash_string(HASH_IP, 0), \
        hash_string(HASH_EGRESSNAME, 0));\
      else \
        HASH_KEY = hash_2words(hash_string(HASH_IP, 0), 0);

/*
 * Function      : compare_server
 * Responsiblity : compare server ipv6 address and
 *                 outgoing egress ifNames
 * Parameters    : ip1, ip2 - ipv6 addresses
 *                 ifName1, ifName2 - egress interface
 * Return        : true - success
 *                 false - failure
 */
bool compare_server(char *ip1, char*ip2, char*ifName1, char*ifName2)
{
    if (0 == strncmp(ip1, ip2, strlen(ip1)))
    {
        if (ifName1 == NULL && ifName2 == NULL)
            return true;
        else if (ifName1 == NULL || ifName2 == NULL)
            return false;
        else
            if (0 == strncmp(ifName1, ifName2, strlen(ifName1)))
                return true;
    }
    return false;
}

/*
 * Function      : dhcpv6r_get_server_entry
 * Responsiblity : Lookup the hash map for a specific server entry
 * Parameters    : ipv6_address - server IPv6 address
 *                 egressIfName - Outgoing Interface name
 * Return        : DHCPV6_RELAY_SERVER_T* - pointer to the server entry
 */
DHCPV6_RELAY_SERVER_T* dhcpv6r_get_server_entry(char* ipv6_address,
                                        char* egressIfName)
{
    uint32_t hash_key;
    DHCPV6_RELAY_SERVER_T *serverIP = NULL;

    GENERATE_KEY(ipv6_address, egressIfName, hash_key);
    CMAP_FOR_EACH_WITH_HASH(serverIP, cmap_node, hash_key,
                           &dhcpv6_relay_ctrl_cb_p->serverHashMap) {
        if (compare_server(ipv6_address, serverIP->ipv6_address,
                egressIfName, serverIP->egressIfName))
            return serverIP;
    }
    return NULL;
}

/*
 * Function      : dhcpv6r_add_server_entry
 * Responsiblity : Add a server entry to server hash table
 * Parameters    : ipv6_address - server IPv6 address
 *                 egressIfName - Outgoing Interface name
 * Return        : DHCPV6_RELAY_SERVER_T* - pointer to the newly created server entry
 */
DHCPV6_RELAY_SERVER_T* dhcpv6r_add_server_entry(char* ipv6_address,
                                        char* egressIfName)
{
    DHCPV6_RELAY_SERVER_T *serverIP;
    uint32_t hash_key;

    serverIP = (DHCPV6_RELAY_SERVER_T *) malloc(sizeof(DHCPV6_RELAY_SERVER_T));
    if (NULL == serverIP) {
        VLOG_ERR("Failed to allocate memory for the server entry for "
                 "ipv6: %s", ipv6_address);
        return NULL;
    }

    serverIP->ipv6_address = xstrdup(ipv6_address);
    if (egressIfName)
        serverIP->egressIfName = xstrdup(egressIfName);
    else
        serverIP->egressIfName = NULL;
    serverIP->ref_count  = 1; /*Reference count starts with 1*/

    GENERATE_KEY(ipv6_address, egressIfName, hash_key);
    cmap_insert(&dhcpv6_relay_ctrl_cb_p->serverHashMap,
                (struct cmap_node *)serverIP, hash_key);

    return serverIP;
}

/*
 * Function      : dhcpv6r_store_address
 * Responsiblity : Add a server reference to an interface
 * Parameters    : intfNode - Interface entry
 *                 ipv6_address - server IPv6 address
 *                 egressIfName - outgoing interface name
 * Return        : true - if the entry is successfully added
 *                 false - otherwise
 */
bool dhcpv6r_store_address(DHCPV6_RELAY_INTERFACE_NODE_T *intfNode,
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
        intfNode->serverArray = (DHCPV6_RELAY_SERVER_T **)
                                    malloc(MAX_SERVERS_PER_INTERFACE
                                    * sizeof(DHCPV6_RELAY_SERVER_T *));
        if (NULL == intfNode->serverArray) {
            VLOG_ERR("Failed to allocate server array for interface : %s",
                    intfNode->portName);
            sem_post(&dhcpv6_relay_ctrl_cb_p->waitSem);
            return false;
        }
    }

    VLOG_INFO("Attempting to add server entry ipv6: %s"
              "on interface : %s", ipv6_address, intfNode->portName);
    /* Checks whether Server IP entry exists or not */
    server = dhcpv6r_get_server_entry(ipv6_address, egressIfName);
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
        if((server = dhcpv6r_add_server_entry(
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
 * Function      : dhcpv6r_push_deleted_server_ref_to_end
 * Responsiblity : Move the deleted entry from the server array to the end
 * Parameters    : intfNode - Interface entry
 *                 deleted_index - Index in the server array of the interface
 * Return        : none
 */
void dhcpv6r_push_deleted_server_ref_to_end(DHCPV6_RELAY_INTERFACE_NODE_T *intfNode,
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
 * Function      : dhcpv6r_remove_server_ref_entry
 * Responsiblity : Remove reference to a server entry from an interface entry
 * Parameters    : intfNode - Interface entry
 *                 ipv6_address - server IPv6 address
 *                 deleted_index - Index in the server array
 * Return        : true - if the entry is successfully dereferenced
 *                 false - entry not found
 */
bool dhcpv6r_remove_server_ref_entry(DHCPV6_RELAY_INTERFACE_NODE_T *intfNode,
                                char* ipv6_address, char* egressIfName,
                                int *deleted_index)
{
    uint8_t index = 0;
    uint32_t hash_key;
    DHCPV6_RELAY_SERVER_T **serverArray = NULL;
    DHCPV6_RELAY_SERVER_T *server;

    serverArray = intfNode->serverArray;
    /* Find the entry in Server IP Table & free the memory (structFree) if
     * ref_count is zero */
    for ( ; index < intfNode->addrCount; index++)
    {
        if (compare_server(serverArray[index]->ipv6_address, ipv6_address,
                serverArray[index]->egressIfName, egressIfName))
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
                GENERATE_KEY(ipv6_address, egressIfName, hash_key);
                VLOG_INFO("server reference count reached 0. Freeing entry");
                cmap_remove(&dhcpv6_relay_ctrl_cb_p->serverHashMap,
                            (struct cmap_node *)server, hash_key);
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
 * Function      : dhcpv6r_remove_address
 * Responsiblity : Remove a server reference from an interface
 * Parameters    : intfNode - Interface entry
 *                 ipaddress - server IPv6 address
 *                 egressIfName - outgoing interface name
 * Return        : true - if the server reference is removed
 *                 false - otherwise
 */
bool dhcpv6r_remove_address(DHCPV6_RELAY_INTERFACE_NODE_T *intfNode,
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

    retVal = dhcpv6r_remove_server_ref_entry(intfNode, ipv6_address,
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
        dhcpv6r_push_deleted_server_ref_to_end(intfNode, deleted_index);
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

        /* leave multicast group for this interface */
        (void) dhcpv6_relay_join_or_leave_mcast_group(intfNode, false);
        if (NULL != intfNode->portName)
            free(intfNode->portName);
        free(intfNode);
    }
    sem_post(&dhcpv6_relay_ctrl_cb_p->waitSem);
    return true;
}


/*
 * Function      : dhcpv6r_create_intfnode
 * Responsiblity : Allocate memory for interface entry
 * Parameters    : pname - interface name
 *                 ip6_address - ipv6 address on this interface
 * Return        : DHCPV6_RELAY_INTERFACE_NODE_T* - Interface node
 */
DHCPV6_RELAY_INTERFACE_NODE_T *dhcpv6r_create_intferface_node(char *pname, char* ip6_address)
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

    memset(intfNode->ip6_address, 0, sizeof(intfNode->ip6_address));
    intfNode->portName = xstrdup(pname);
    if (NULL == intfNode->portName)
    {
       VLOG_ERR("Failed to allocate memory for portName : %s", pname);
       return NULL;
    }

    strncpy(intfNode->ip6_address, ip6_address, strlen(ip6_address));
    intfNode->addrCount = 0;
    intfNode->serverArray = NULL;
    shash_add(&dhcpv6_relay_ctrl_cb_p->intfHashTable, pname, intfNode);
    VLOG_INFO("Allocated interface table record for port : %s", pname);

    /* Join Multicast group for this interface */
    if (dhcpv6_relay_ctrl_cb_p->dhcpv6r_recvSock == -1 ||
        !dhcpv6_relay_join_or_leave_mcast_group(intfNode, true))
    {
        VLOG_ERR("join multicast group for this interface failed %s", pname);
        /* free memory and return null */
        if (NULL != intfNode->portName)
            free(intfNode->portName);
        free(intfNode);

        return NULL;
    }
    return intfNode;
}

/*
 * Function      : dhcpv6r_handle_row_delete
 * Responsiblity : Process delete event for one or more ports records from
 *                 DHCP-Relay table
 * Parameters    : idl - idl reference
 * Return        : none
 */
void dhcpv6r_handle_row_delete(struct ovsdb_idl *idl)
{
    const struct ovsrec_dhcp_relay *rec = NULL;
    DHCPV6_RELAY_INTERFACE_NODE_T *intf = NULL;
    struct shash_node *node = NULL, *next = NULL;
    int iter = 0, count = 0;
    DHCPV6_RELAY_SERVER_T **serverArray = NULL;
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
            serverArray = intf->serverArray;
            count = intf->addrCount;
            /* Delete the interface entry from hash table */
            for (iter = 0; iter < count; iter++) {
                dhcpv6r_remove_address(intf, serverArray[iter]->ipv6_address,
                                      serverArray[iter]->egressIfName);
            }
        }
    }
}

/*
 * Function      : dhcpv6r_flush_removed_ucast_entries
 * Responsiblity : Check for ucast entries deleted in dhcp relay table
 *                 and remove from local cache
 * Parameters    : rec - DHCP-Relay OVSDB table record
 *                 intfNode - Interface entry
 * Return        : none
 */
void dhcpv6r_flush_removed_ucast_entries(DHCPV6_RELAY_INTERFACE_NODE_T *intfNode,
                        const struct ovsrec_dhcp_relay *rec)
{
    /*FIXME : need to check below indexing for large number of ip address */
    int iter = 0;
    struct ovsrec_dhcp_relay ip;
    memset(&ip, 0, sizeof(ip));

    ip.ipv6_ucast_server = (char**) calloc(INET6_ADDRSTRLEN, sizeof(char));

    /* Collect the servers that are removed from the list */
    for (iter = 0; iter < intfNode->addrCount; iter++) {
        ip.ipv6_ucast_server[0] = intfNode->serverArray[iter]->ipv6_address;

        if (ovsrec_dhcp_relay_index_find(&ipv6_ucast_cursor, &ip) == NULL) {
            if(!dhcpv6r_remove_address(intfNode, *(ip.ipv6_ucast_server), NULL))
                VLOG_ERR("unicast ipv6 entry deletion in local cache failed");
        }
    }
    free(ip.ipv6_ucast_server);
}

/*
 * Function      : dhcpv6r_get_ucast_entries_added
 * Responsiblity : Check for ucast entries added in dhcp relay table
 * Parameters    : rec - DHCP-Relay OVSDB table record
 *                 intfNode - Interface entry
 * Return        : none
 */

void dhcpv6r_get_ucast_entries_added(DHCPV6_RELAY_INTERFACE_NODE_T *intfNode,
                        const struct ovsrec_dhcp_relay *rec)
{
    char *ipv6_address = NULL;
    int iter = 0, iter1 = 0;
    bool found;

    for (iter = 0; iter < rec->n_ipv6_ucast_server; iter++) {
        found = false;
        for (iter1 = 0; iter1 < intfNode->addrCount; iter1++) {
            ipv6_address = intfNode->serverArray[iter1]->ipv6_address;
            if (compare_server
                (ipv6_address, rec->ipv6_ucast_server[iter], NULL, NULL))
            {
                found = true;
                break;
            }
        }
        if (false == found) {
            if (!dhcpv6r_store_address(intfNode, rec->ipv6_ucast_server[iter], NULL))
                VLOG_ERR("unicast ipv6 entry addition in local cache failed");
        }
    }
}

/*
 * Function      : dhcpv6r_get_mcast_entries_added
 * Responsiblity : Check for mcast entries added in dhcp relay table
 * Parameters    : rec - DHCP-Relay OVSDB table record
 *                 intfNode - Interface entry
 * Return        : none
 */

void dhcpv6r_get_mcast_entries_added(DHCPV6_RELAY_INTERFACE_NODE_T *intfNode,
                        const struct ovsrec_dhcp_relay *rec)
{
    char *ipv6_address = NULL, *egressIfName = NULL;
    int iter = 0, iter1 = 0, iter2 = 0;
    bool found;

    for (iter = 0; iter < rec->n_dhcp_relay_v6_mcast_servers; iter++) {
        found = false;
        for (iter1 = 0; iter1 < rec->dhcp_relay_v6_mcast_servers[iter]->n_ports; iter1++) {
            for (iter2 = 0; iter2 < intfNode->addrCount; iter2++) {
                ipv6_address = intfNode->serverArray[iter2]->ipv6_address;
                egressIfName = intfNode->serverArray[iter2]->egressIfName;

                if (compare_server
                (ipv6_address, rec->dhcp_relay_v6_mcast_servers[iter]->ipv6_mcast_server,
                egressIfName, rec->dhcp_relay_v6_mcast_servers[iter]->ports[iter1]->name))
                {
                    found = true;
                    break;
                }
            }
            if (false == found) {
                if (!dhcpv6r_store_address(intfNode, rec->dhcp_relay_v6_mcast_servers[iter]->ipv6_mcast_server,
                    rec->dhcp_relay_v6_mcast_servers[iter]->ports[iter1]->name))
                    VLOG_ERR("multicast ipv6 entry addition in local cache failed");
            }
        }
    }
}

/*
 * Function      : dhcpv6r_flush_removed_mcast_entries
 * Responsiblity : Check for ucast entries deleted in dhcp relay table
 *                 and remove from local cache
 * Parameters    : rec - DHCP-Relay OVSDB table record
 *                 intfNode - Interface entry
 * Return        : none
 */
void dhcpv6r_flush_removed_mcast_entries(DHCPV6_RELAY_INTERFACE_NODE_T *intfNode,
                        const struct ovsrec_dhcp_relay *rec)
{
    char *ipv6_address = NULL, *egressIfName = NULL;
    int iter = 0, iter1 = 0, iter2 = 0;
    bool found;

    /* Collect the servers that are removed from the list */
    for (iter = 0; iter < intfNode->addrCount; iter++) {
        found = false;
        ipv6_address = intfNode->serverArray[iter]->ipv6_address;
        egressIfName = intfNode->serverArray[iter]->egressIfName;
        for (iter1 = 0; iter1 < rec->n_dhcp_relay_v6_mcast_servers; iter1++) {
            for (iter2 = 0; iter2 < rec->dhcp_relay_v6_mcast_servers[iter1]->n_ports; iter2++) {
                if (compare_server
                    (ipv6_address, rec->dhcp_relay_v6_mcast_servers[iter1]->ipv6_mcast_server,
                    egressIfName, rec->dhcp_relay_v6_mcast_servers[iter1]->ports[iter2]->name))
                {
                    found = true;
                    break;
                }
            }
            if (false == found) {
                if(!dhcpv6r_remove_address(intfNode, ipv6_address,
                    rec->dhcp_relay_v6_mcast_servers[iter1]->ports[iter2]->name))
                VLOG_ERR("multicast ipv6 entry deletion in local cache failed");
            }
        }
    }
}


#if 0
/*
 * Function      : dhcpv6r_get_mcast_entries_added
 * Responsiblity : Check for mcast entries added in dhcp relay table
 * Parameters    : rec - DHCP-Relay OVSDB table record
 *                 intfNode - Interface entry
 * Return        : none
 */
void dhcpv6r_get_mcast_entries_added(DHCPV6_RELAY_INTERFACE_NODE_T *intfNode,
                        const struct ovsrec_dhcp_relay *rec)
{
    char *outIfName = NULL, *ipv6_address = NULL, *egressIfName = NULL;
    bool found;
    const struct smap_node *smap_node = NULL;
    int iter = 0;

    SMAP_FOR_EACH(smap_node, &rec->ipv6_mcast_server) {
        found = false;
        if (smap_node->key)
        {
            /* get the first egressifName */
            outIfName = strtok(smap_node->value, " ");
            while(outIfName != NULL) {
                for (iter = 0; iter < intfNode->addrCount; iter++) {
                    ipv6_address = intfNode->serverArray[iter]->ipv6_address;
                    egressIfName = intfNode->serverArray[iter]->egressIfName;
                    if (compare_server
                        (ipv6_address, smap_node->key, egressIfName, outIfName))
                    {
                        found = true;
                        break;
                    }
                }
                if (false == found) {
                    if (!dhcpv6r_store_address(intfNode, smap_node->key, outIfName))
                    VLOG_ERR("multicast ipv6 entry addition in local cache failed");

                }
                outIfName = strtok(NULL, " ");
            }
        }
    }
}

/*
 * Function      : dhcpv6r_flush_removed_mcast_entries
 * Responsiblity : Check for mcast entries deleted in dhcp relay table
 *                 and remove from local cache
 * Parameters    : rec - DHCP-Relay OVSDB table record
 *                 intfNode - Interface entry
 * Return        : none
 */
void dhcpv6r_flush_removed_mcast_entries(DHCPV6_RELAY_INTERFACE_NODE_T *intfNode,
                        const struct ovsrec_dhcp_relay *rec)
{
    char *outIfName = NULL, *ipv6_address = NULL, *egressIfName = NULL;
    bool found;
    const struct smap_node *smap_node = NULL;
    int size = 0, iter = 0, iter1 = 0;
    DHCPV6_RELAY_SERVER_T tempServers[MAX_SERVERS_PER_INTERFACE];

    memset(&tempServers, 0, sizeof(struct DHCPV6_RELAY_SERVER_T));
    /* collect all the values from ovsdb */
    SMAP_FOR_EACH(smap_node, &rec->ipv6_mcast_server) {
    if (smap_node->key)
        /* get the first egressifName */
        outIfName = strtok(smap_node->value, " ");
        while(outIfName != NULL) {
            tempServers[size].ipv6_address = xstrdup(smap_node->key);
            tempServers[size].egressIfName = xstrdup(outIfName);
            size++;
            outIfName = strtok(NULL, " ");
        }
    }

    /* Collect the mcast servers that are removed from the list */
    for (iter = 0; iter < intfNode->addrCount; iter++) {
        found = false;
        ipv6_address = intfNode->serverArray[iter]->ipv6_address;
        egressIfName = intfNode->serverArray[iter]->egressIfName;
        for (iter1 = 0; iter1 < size; iter1++) {
            if (compare_server(ipv6_address, tempServers[iter1].ipv6_address,
                egressIfName, tempServers[iter1].egressIfName))
            {
                found = true;
                break;
            }
        }
        if (false == found) {
            if(!dhcpv6r_remove_address(intfNode, ipv6_address, egressIfName))
                VLOG_ERR("multicast ipv6 entry deletion in local cache failed");
        }
    }
}
#endif
/*
 * Function      : dhcpv6_relay_handle_config_change
 * Responsiblity : Handle a record change in DHCP-Relay table
 * Parameters    : rec - DHCP-Relay OVSDB table record
 *                 idl_seqno - idl change identifier
 * Return        : none
 */
void dhcpv6r_handle_config_change(
              const struct ovsrec_dhcp_relay *rec, uint32_t idl_seqno)
{
    char *portName = NULL;
    struct shash_node *node;
    DHCPV6_RELAY_INTERFACE_NODE_T *intfNode = NULL;

    if ((NULL == rec) ||
        (NULL == rec->port) ||
        (NULL == rec->vrf) ||
        (NULL == rec->port->ip6_address)) {
        return;
    }

    portName = rec->port->name;

    /* FIXME : add code to read secondary address */

    /* Do lookup for the interface entry in hash table */
    node = shash_find(&dhcpv6_relay_ctrl_cb_p->intfHashTable, portName);
    if (NULL == node) {
       /* Interface entry not found, create one */
       if (NULL == (intfNode = dhcpv6r_create_intferface_node
                                (portName, rec->port->ip6_address)))
        {
            return;
        }
    }
    else
    {
        intfNode = (DHCPV6_RELAY_INTERFACE_NODE_T *)node->data;
    }

    /* If no change in both colmns return */
    if (!(OVSREC_IDL_IS_COLUMN_MODIFIED(ovsrec_dhcp_relay_col_ipv6_ucast_server,
                                   idl_seqno)) &&
        !(OVSREC_IDL_IS_COLUMN_MODIFIED(ovsrec_dhcp_relay_col_dhcp_relay_v6_mcast_servers,
                               idl_seqno)))
    {
        return;
    }

    if (OVSREC_IDL_IS_COLUMN_MODIFIED(ovsrec_dhcp_relay_col_ipv6_ucast_server,
                               idl_seqno)) {
        dhcpv6r_get_ucast_entries_added(intfNode, rec);
        dhcpv6r_flush_removed_ucast_entries(intfNode, rec);
    }

    if (OVSREC_IDL_IS_COLUMN_MODIFIED(ovsrec_dhcp_relay_col_dhcp_relay_v6_mcast_servers,
                           idl_seqno)) {
        dhcpv6r_get_mcast_entries_added(intfNode, rec);
        dhcpv6r_flush_removed_mcast_entries(intfNode, rec);
    }

}

#endif /* FTR_DHCPV6_RELAY */

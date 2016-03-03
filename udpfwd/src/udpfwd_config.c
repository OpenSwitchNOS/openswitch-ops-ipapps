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
 * File: udpfwd_config.c
 *
 */

/*
 * Configuration maintainer for dhcp-relay/udp-broadcast-forwarder features.
 *
 */


#include "udpfwd.h"
#include "hash.h"

VLOG_DEFINE_THIS_MODULE(udpfwd_config);

/*
 * Function      : udpfwd_get_server_entry
 * Responsiblity : Lookup the hash map for a specific server entry
 * Parameters    : ipaddress - server IP address
 *                 udpPort - destination udp port
 * Return        : UDPFWD_SERVER_T* - pointer to the server entry
 */
UDPFWD_SERVER_T* udpfwd_get_server_entry(IP_ADDRESS ipaddress, uint16_t udpPort)
{
   UDPFWD_SERVER_T *serverIP = NULL;
   uint32_t      hashVal = 0;

   hashVal = hash_int(ipaddress, (uint32_t)udpPort);
   CMAP_FOR_EACH_WITH_HASH(serverIP, cmap_node, hashVal,
                           &udpfwd_ctrl_cb_p->serverHashMap) {
       if ((serverIP->ip_address == ipaddress) &&
           (serverIP->udp_port == udpPort)) {
           return serverIP;
       }
   }

   return NULL;
}

/*
 * Function      : udpfwd_add_server_entry
 * Responsiblity : Add a server entry to server hash table
 * Parameters    : ipaddress - server IP address
 *                 udpPort - destination udp port
 * Return        : UDPFWD_SERVER_T* - pointer to the newly created server entry
 */
UDPFWD_SERVER_T* udpfwd_add_server_entry(IP_ADDRESS ipaddress, uint16_t udpPort)
{

    UDPFWD_SERVER_T *serverIP;

    /* FIXME: Add a check for global maximum server count */

    serverIP = (UDPFWD_SERVER_T *) malloc (sizeof(UDPFWD_SERVER_T));
    if (NULL == serverIP) {
        VLOG_ERR("Failed to allocate memory for the server entry for "
                 "ip: %x, port: %d", ipaddress, udpPort);
        return NULL;
    }

    serverIP->ip_address = ipaddress;
    serverIP->udp_port   = udpPort;
    serverIP->ref_count  = 1;      /*Reference count starts with 1*/

    cmap_insert(&udpfwd_ctrl_cb_p->serverHashMap, (struct cmap_node *)serverIP,
                hash_int(ipaddress, udpPort));

    return serverIP;
}

/*
 * Function      : udpfwd_push_deleted_server_ref_to_end
 * Responsiblity : Move the deleted entry from the server array to the end
 * Parameters    : intfNode - Interface entry
 *                 deleted_index - Index in the server array of the interface
 * Return        : none
 */
void udpfwd_push_deleted_server_ref_to_end(UDPFWD_INTERFACE_NODE_T *intfNode,
                int deleted_index)
{

   uint8_t       currentAddrCount = 0;
   UDPFWD_SERVER_T *server;
   UDPFWD_SERVER_T **serverArray = intfNode->serverArray;

   currentAddrCount = intfNode->addrCount;

   /* Move the NULL at the end of the list */
   server = serverArray[currentAddrCount];
   serverArray[currentAddrCount] = serverArray[deleted_index] = NULL;
   serverArray[deleted_index] = server;
}

/*
 * Function      : udpfwd_remove_server_ref_entry
 * Responsiblity : Remove reference to a server entry from an interface entry
 * Parameters    : intfNode - Interface entry
 *                 ipaddress - server IP address
 *                 udpPort - destination udp port
 *                 deleted_index - Index in the server array
 * Return        : true - if the entry is successfully dereferenced
 *                 false - entry not found
 */
bool udpfwd_remove_server_ref_entry(UDPFWD_INTERFACE_NODE_T *intfNode,
                                     IP_ADDRESS ipaddress, uint16_t udpPort,
                                     int *deleted_index)
{
    uint8_t index = 0;
    UDPFWD_SERVER_T **serverArray = NULL;
    UDPFWD_SERVER_T *server;

    serverArray = intfNode->serverArray;
    /* Find the entry in Server IP Table & free the memory (structFree) if
     * ref_count is zero */
    for ( ; index < intfNode->addrCount; index++)
    {
        if ((serverArray[index]->ip_address == ipaddress) &&
            (serverArray[index]->udp_port == udpPort))
        /* check if address & port exist in table */
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
                VLOG_INFO("server reference count reached 0. Freeing entry");
                cmap_remove(&udpfwd_ctrl_cb_p->serverHashMap,
                            (struct cmap_node *)server,
                            hash_int(ipaddress, udpPort));
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
 * Function      : udpfwd_store_address
 * Responsiblity : Add a server reference to an interface
 * Parameters    : intfNode - Interface entry
 *                 ipaddress - server IP address
 *                 udpPort - destination udp port
 * Return        : true - if the entry is successfully added
 *                 false - otherwise
 */
bool udpfwd_store_address(UDPFWD_INTERFACE_NODE_T *intfNode,
                          IP_ADDRESS ipaddress, uint16_t udpPort)
{
   UDPFWD_SERVER_T  *server;

   if(intfNode->addrCount >= MAX_UDP_BCAST_SERVER_PER_INTERFACE)
   {
        VLOG_ERR("Maximum udp server configuration limit reached on interface"
                  " %s (count : %d)", intfNode->portName, intfNode->addrCount);
        return false;
   }

   sem_wait(&udpfwd_ctrl_cb_p->waitSem);

   /* If address count is non zero but the addrses is NULL, return error */
   if((0 != intfNode->addrCount) && (NULL == intfNode->serverArray))
   {
         VLOG_ERR("Address count is [%d], but server IP address ref array "
                  "is NULL for Interface [%s] while storing a server ref",
                  intfNode->addrCount, intfNode->portName);
         sem_post(&udpfwd_ctrl_cb_p->waitSem);
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
      intfNode->serverArray = (UDPFWD_SERVER_T **)
                              malloc(MAX_UDP_BCAST_SERVER_PER_INTERFACE
                                     * sizeof(UDPFWD_SERVER_T *));
      if (NULL == intfNode->serverArray) {
          VLOG_ERR("Failed to allocate server array for interface : %s",
                   intfNode->portName);
          sem_post(&udpfwd_ctrl_cb_p->waitSem);
          return false;
      }
    }

    VLOG_INFO("Attempting to add server entry (ip: %d, udp_Port:%d), "
              "on interface : %s", ipaddress, udpPort, intfNode->portName);
    /* Checks whether Server IP entry exists or not */
    server = udpfwd_get_server_entry(ipaddress, udpPort);
    if(NULL != server)
    {
        /* Increment server entry reference count */
        server->ref_count++;
        VLOG_INFO("Matching server found, incremented ref count. "
               "server : %d (refcount :%d)", server->ip_address,
               server->ref_count);
    }
    else
    {
        /* No matching server entry, create new */
        if((server = udpfwd_add_server_entry(
                       ipaddress, udpPort)) == NULL)
        {
            VLOG_ERR("Error while adding a new server entry");
            sem_post(&udpfwd_ctrl_cb_p->waitSem);
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

    sem_post(&udpfwd_ctrl_cb_p->waitSem);

    return true;
}

/*
 * Function      : udpfwd_remove_address
 * Responsiblity : Remove a server reference from an interface
 * Parameters    : intfNode - Interface entry
 *                 ipaddress - server IP address
 *                 udpPort - destination udp port
 * Return        : true - if the server reference is removed
 *                 false - otherwise
 */
bool udpfwd_remove_address(UDPFWD_INTERFACE_NODE_T *intfNode,
                        IP_ADDRESS ipaddress, uint16_t udpPort)
{
    UDPFWD_SERVER_T **serverArray = NULL;
    bool          retVal = false;
    int           deleted_index;  /*IP ref table's deleted index*/
    struct shash_node *node;

    VLOG_INFO("Attempting to delete server : %d, udp_port : %d on "
              "interface : %s", ipaddress, udpPort, intfNode->portName);

    sem_wait(&udpfwd_ctrl_cb_p->waitSem);

    /* Server IP Reference table pointer */
    serverArray = intfNode->serverArray;

    retVal = udpfwd_remove_server_ref_entry(intfNode, ipaddress, udpPort,
                                            (int*)&deleted_index);
    if(false == retVal)
    {
        /* Release the semaphore & return */
        sem_post(&udpfwd_ctrl_cb_p->waitSem);
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
        udpfwd_push_deleted_server_ref_to_end(intfNode, deleted_index);
    }

    /* Check whether this was the only configured server IP for the interface.
     * In that case we must free the memory for entire server table
     * for this interface. First entry NULL means, entire list is NULL.
     * because NULL entries are
     * always pushed to the end of the list */
    if((NULL == serverArray[0]) && (intfNode->addrCount ==0))
    {
        VLOG_INFO("All server configuration on the interface : %s are removed."
                  " Freeing interface entry", intfNode->portName);
        /* Delete the entire IP reference table */
        free(serverArray);
        /* Make interface table entry NULL, as no helper IP is configured */
        intfNode->serverArray = NULL;
        node = shash_find(&udpfwd_ctrl_cb_p->intfHashTable,
                          intfNode->portName);
        if (NULL != node)
        {
            shash_delete(&udpfwd_ctrl_cb_p->intfHashTable, node);
        }
        else
        {
            VLOG_ERR("Interface node not found in hash table : %s",
                     intfNode->portName);
        }
        free(intfNode);
    }

    sem_post(&udpfwd_ctrl_cb_p->waitSem);
    return true;
}

/*
 * Function      : udpfwd_handle_dhcp_relay_row_delete
 * Responsiblity : Process delete event for one or more ports records from
 *                 DHCP-Relay table
 * Parameters    : idl - idl reference
 * Return        : none
 */
void udpfwd_handle_dhcp_relay_row_delete(struct ovsdb_idl *idl)
{
    const struct ovsrec_dhcp_relay *rec = NULL;
    UDPFWD_INTERFACE_NODE_T *intf;
    struct shash_node *node, *next;
    int iter, addrCount;
    bool found;

    /* Walk the server configuration hash table per "port" to
     * see if the corresponding record is deleted */
    SHASH_FOR_EACH_SAFE(node, next, &udpfwd_ctrl_cb_p->intfHashTable) {
        found = false;
        /* Iterate through dhcp relay table to find a match */
        OVSREC_DHCP_RELAY_FOR_EACH(rec, idl) {
            if ((NULL != rec->port) &&
                !strncmp(rec->port->name, node->name, IFNAME_LEN)) {
                found = true;
                break;
            }
        }

        if (false == found) {
            intf = (UDPFWD_INTERFACE_NODE_T *)node->data;
            addrCount = intf->addrCount;
            /* Delete the interface entry from hash table */
            for (iter = 0; iter < addrCount; iter++) {
                udpfwd_remove_address(intf,
                            intf->serverArray[iter]->ip_address,
                            intf->serverArray[iter]->udp_port);
            }
        }
    }

    return;
}

/*
 * Function      : udpfwd_handle_dhcp_relay_config_change
 * Responsiblity : Handle a record change in DHCP-Relay table
 * Parameters    : rec - DHCP-Relay OVSDB table record
 * Return        : none
 */
void udpfwd_handle_dhcp_relay_config_change(
              const struct ovsrec_dhcp_relay *rec)
{
    struct in_addr id;
    IP_ADDRESS ipaddress;
    char *portName = NULL;
    int iter, iter1;
    struct shash_node *node;
    UDPFWD_INTERFACE_NODE_T *intfNode = NULL;
    uint32_t servers[MAX_UDP_BCAST_SERVER_PER_INTERFACE];
    uint32_t *arrayPtr;
    int retVal;
    bool found;

    if ((NULL == rec) ||
        (NULL == rec->port) ||
        (NULL == rec->vrf)) {
        return;
    }

    portName = rec->port->name;

    /* Do lookup for the interface entry in hash table */
    node = shash_find(&udpfwd_ctrl_cb_p->intfHashTable, portName);
    if (NULL == node) {
       /* There is no server configuration available for the port,
        * create one */
       intfNode = (UDPFWD_INTERFACE_NODE_T *)
                        calloc(1, sizeof(UDPFWD_INTERFACE_NODE_T));
       strncpy(intfNode->portName, portName, IFNAME_LEN);
       intfNode->addrCount = 0;
       intfNode->serverArray = NULL;
       shash_add(&udpfwd_ctrl_cb_p->intfHashTable, portName, intfNode);
       VLOG_INFO("Allocated server table record for port : %s", portName);
    }
    else
    {
        intfNode = (UDPFWD_INTERFACE_NODE_T *) node->data;
    }

    memset(servers, 0, sizeof(servers));

    arrayPtr = (uint32_t*)servers;
    /* Collect the servers that are removed from the list */
    for (iter = 0; iter < intfNode->addrCount; iter++) {
        found = false;
        ipaddress = intfNode->serverArray[iter]->ip_address;

        for (iter1 = 0; iter1 < rec->n_ipv4_ucast_server; iter1++) {
            retVal = inet_aton (rec->ipv4_ucast_server[iter1], &id);
            if (!retVal || (id.s_addr == 0)) {
                VLOG_ERR("Invalid IP seen during server update : %s",
                        rec->ipv4_ucast_server[iter1]);
                continue;
            }
            if (ipaddress == (id.s_addr)) {
                found = true;
                break;
            }
        }

        if (false == found) {
            *arrayPtr = ipaddress;
            arrayPtr++;
        }
    }

    /* Delete the servers that were removed in the config update */
    for (iter = 0; servers[iter] != 0; iter++) {
        udpfwd_remove_address(intfNode, servers[iter], DHCPS_PORT);
    }

    memset(servers, 0, sizeof(servers));
    /* Collect the servers that are newly added */
    arrayPtr = (uint32_t*)servers;
    for (iter = 0; iter < rec->n_ipv4_ucast_server; iter++) {
        found = false;
        retVal = inet_aton(rec->ipv4_ucast_server[iter], &id);
        if (!retVal || (id.s_addr == 0)) {
            VLOG_ERR("Invalid IP seen during server update : %s",
                    rec->ipv4_ucast_server[iter]);
            continue;
        }

        for (iter1 = 0; iter1 < intfNode->addrCount; iter1++) {
            if (intfNode->serverArray[iter1]->ip_address ==
                       id.s_addr) {
                found = true;
                break;
            }
        }

        if (false == found) {
            *arrayPtr = id.s_addr;
            arrayPtr++;
        }
    }

    /* Create the newly added servers */
    for (iter = 0; servers[iter] != 0; iter++) {
        udpfwd_store_address(intfNode, servers[iter], DHCPS_PORT);
    }

    return;
}

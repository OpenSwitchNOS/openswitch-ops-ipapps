/* DNS Client CLI commands
 *
 * Copyright (C) 2016 Hewlett Packard Enterprise Development LP
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * File: dns_client_vty.c
 *
 * Purpose:  To add DNS Client CLI configuration and
 *           display commands.
 */
/* Commenting out the current code as there is a change in priorities,
   will come back to this and continue the development */
#include "dns_client_vty.h"

extern struct ovsdb_idl *idl;
VLOG_DEFINE_THIS_MODULE(vtysh_dns_client_cli);

/* FIXME - This support for global command is removed from the schema,
        but the schema has not been finalised. depending on the schema
        this API has to be kept or removed.
*/
/*-----------------------------------------------------------------------------
| Function       : dnsclient_globalconfig
| Responsibility : To enable/disable DNS Client.
| Parameters     :
|        enable  : If true, enable DNS Client and
|                  if false disable the DNS Client
| Return         : On success returns CMD_SUCCESS,
|                  On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
dnsclient_globalconfig(const char *enable)
{
    const struct ovsrec_system *ovs_row = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    struct smap smap_status_value;
    enum ovsdb_idl_txn_status txn_status;

    if (status_txn == NULL)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    ovs_row = ovsrec_system_first(idl);
    if (!ovs_row) {
        VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    smap_clone(&smap_status_value, &ovs_row->other_config);

    /* Update the latest config status. */
    smap_replace(&smap_status_value,
                 SYSTEM_OTHER_CONFIG_MAP_DNS_CLIENT_DISABLED, enable);

    ovsrec_system_set_other_config(ovs_row, &smap_status_value);
    smap_destroy(&smap_status_value);
    txn_status = cli_do_config_finish(status_txn);

    if (txn_status == TXN_SUCCESS || txn_status == TXN_UNCHANGED)
    {
        return CMD_SUCCESS;
    }
    else
    {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
        return CMD_OVSDB_FAILURE;
    }
}

/*-----------------------------------------------------------------------------
| Function       : get_dnsclient_row
| Responsibility : Get the dns client row corresponding to the default VRF
| Return         : On success returns dns client row specific to default VRF,
|                  On failure returns NULL
-----------------------------------------------------------------------------*/
const struct ovsrec_dns_client *
get_dnsclient_row (void)
{
    const struct ovsrec_dns_client *dns_client_row = NULL;

    OVSREC_DNS_CLIENT_FOR_EACH (dns_client_row, idl)
    {
        if (!strncmp(dns_client_row->vrf->name,
                     DEFAULT_VRF_NAME, strlen(DEFAULT_VRF_NAME)))
            return dns_client_row;
    }

    return NULL;
}

/*-----------------------------------------------------------------------------
| Function       : set_dnsclient_row
| Responsibility : Set the dns client row to the default VRF
| Parameters     :
|   status_txn   : Transaction status with OVSDB
| Return         : On success returns dns client row,
|                  On failure returns NULL
-----------------------------------------------------------------------------*/
const struct ovsrec_dns_client *
set_dnsclient_row (struct ovsdb_idl_txn *status_txn)
{
    const struct ovsrec_dns_client *dns_client_row = NULL;
    const struct ovsrec_vrf *vrf_row = NULL;

    dns_client_row = ovsrec_dns_client_insert(status_txn);
    if (!dns_client_row)
    {
        VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
        cli_do_config_abort(status_txn);
        return NULL;
    }

    vrf_row = get_default_vrf(idl);
    if (!vrf_row)
    {
        vty_out(vty, "Error: Could not fetch "
                     "default VRF data.%s", VTY_NEWLINE);
        VLOG_ERR("%s VRF table did not have any rows. "
                 "Ideally it should have just one entry.", __func__);
        cli_do_config_abort(status_txn);
        return NULL;
    }
    ovsrec_dns_client_set_vrf(dns_client_row, vrf_row);

    return dns_client_row;
}

/*-----------------------------------------------------------------------------
| Function       : dnsclient_domain_nameconfig
| Responsibility : Configure/Unconfigure a DNS Client
|                  domain name.
| Parameters     :
|        set     : If true, add a domain name to the OVSDB and
|                  if false, remove the domain name from the OVSDB
|        *argv   : argv from the CLI
| Return         : On success returns CMD_SUCCESS,
|                  On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
dnsclient_domain_nameconfig (bool set, const char *argv[])
{
    const struct ovsrec_dns_client *dns_client_row = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    enum ovsdb_idl_txn_status txn_status;
    struct smap smap;
    char *domain_name = NULL;

    if (status_txn == NULL)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    /* Validate the domain-name */
    if (isalpha((int) *argv[0]))
    {
        if (strlen((char*)argv[0]) > MAX_DOMAIN_NAME_LEN)
        {
            vty_out(vty, "Domain name should be less than %d characters%s",
                         MAX_DOMAIN_NAME_LEN, VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }
    }
    else
    {
        vty_out(vty, "Invalid Domain name%s", VTY_NEWLINE);
        cli_do_config_abort(status_txn);
        return CMD_SUCCESS;
    }

    /* lookup for the record in DNS Client table */
    dns_client_row = get_dnsclient_row();

    if (set)
    {
        /* Configure the DNS Client domain name */
        if (dns_client_row == NULL)
        {
            /*
             * First set of DNS domain name
             */
            dns_client_row = set_dnsclient_row(status_txn);
            if (!dns_client_row)
                return CMD_OVSDB_FAILURE;
        }
        /* Update the domain name */
        smap_clone(&smap, &dns_client_row->other_config);
        smap_replace(&smap, DOMAIN_NAME, (char *)argv[0]);
    }
    else
    {
        /* Unconfigure the DNS Client domain name */
        if (dns_client_row == NULL)
        {
            vty_out(vty, "DNS Client domain name is not present.%s",
                    VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }
        else
        {
            /* Fetch the existing record */
            smap_clone(&smap, &dns_client_row->other_config);
            /* Verify if the user entered domain name exists */
            domain_name = (char *)smap_get(&smap, DOMAIN_NAME);
            if (domain_name == NULL)
            {
                 vty_out(vty, "No domain name configuration is present.%s",
                         VTY_NEWLINE);
                 cli_do_config_abort(status_txn);
                 return CMD_SUCCESS;
            }

            if (!strncmp(domain_name, (char *)argv[0],
                strlen((char*)argv[0])))
            {
                /* domain name exists, so delete it */
                smap_remove(&smap, DOMAIN_NAME);
            }
            else
            {
                vty_out(vty, "domain name entry not found.%s",
                    VTY_NEWLINE);
                cli_do_config_abort(status_txn);
                return CMD_SUCCESS;
            }
        }
    }

    ovsrec_dns_client_set_other_config(dns_client_row, &smap);
    txn_status = cli_do_config_finish(status_txn);
    smap_destroy(&smap);

    if (txn_status == TXN_SUCCESS || txn_status == TXN_UNCHANGED)
    {
        return CMD_SUCCESS;
    }
    else
    {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
        return CMD_OVSDB_FAILURE;
    }
}

/*-----------------------------------------------------------------------------
| Function         : set_dnsclient_fields
| Responsibility   : To update the multiple fields in the DNS Client table.
| Parameters       :
|   dns_client_row : This will point to the dns client row
|   set            : Set/Unset a specific fields from the DNS Client table
|   value          : User input details
|   field          : Field to be updated in the DNS Client table
| Return           : If the field specified is updated successfully, return
|                    true, else returns false.
-----------------------------------------------------------------------------*/
bool
set_dnsclient_fields (const struct ovsrec_dns_client *dns_client_row,
                      bool set, char *value, DNS_CLIENT_FIELDS field)
{
    char **dnsClient, **dnsClientArray;
    bool entryFound = false;
    size_t i, n, maxLength = 0;
    size_t dnsclient_length = 0, dnsClientCount = 0;

    if (field == 0)
    {
        dnsClientCount = dns_client_row->n_domain_list;
        dnsClientArray = dns_client_row->domain_list;
        maxLength = MAX_DOMAIN_LIST_LEN;
    }
    else if (field == 1)
    {
        dnsClientCount = dns_client_row->n_name_servers;
        dnsClientArray = dns_client_row->name_servers;
        maxLength = MAX_NAME_SERV_LEN;
    }

    if (set)
    {
        dnsclient_length = dnsClientCount + 1;
        dnsClient = xmalloc(maxLength * dnsclient_length);

        /* Add a domain list entry to the table */
        if (dnsClientArray)
        {
            for (i = 0; i <= dnsClientCount-1; i++)
            {
                if (!strncmp(value, dnsClientArray[i], strlen(value)))
                {
                    /* Entry already present no need to set anything */
                    free(dnsClient);
                    return entryFound;
                }
                else
                    dnsClient[i] = dnsClientArray[i];
            }
        }

        /* Entry does not exist, so update in the column */
        dnsClient[dnsClientCount] = value;
        if (field == 0)
        {
            ovsrec_dns_client_set_domain_list
                (dns_client_row, dnsClient, dnsclient_length);
        }
        else if (field == 1)
        {
            ovsrec_dns_client_set_name_servers
                (dns_client_row, dnsClient, dnsclient_length);
        }
        entryFound = true;
    }
    else
    {
        dnsclient_length = dnsClientCount - 1;
        dnsClient = xmalloc(maxLength * dnsclient_length);

        /* Delete the domain list from the table */
        if (dnsClientArray)
        {
            for (i = n = 0; i <= dnsClientCount-1; i++)
            {
                if (!strncmp (value, dnsClientArray[i], strlen(value)))
                    entryFound = true;
                else
                    dnsClient[n++] = dnsClientArray[i];
            }
        }
        if (entryFound)
        {
            /* Pre existing entry, delete it in the column */
            if (field == 0)
            {
                ovsrec_dns_client_set_domain_list
                    (dns_client_row, dnsClient, n);
            }
            else if (field == 1)
            {
                ovsrec_dns_client_set_name_servers
                    (dns_client_row, dnsClient, n);
            }
        }
    }

    free(dnsClient);
    return entryFound;
}

/*-----------------------------------------------------------------------------
| Function       : dnsclient_domain_listconfig
| Responsibility : Configure/Unconfigure a DNS Client
|                  domain list.
| Parameters     :
|        set     : If true, add a domain list to the OVSDB and
|                  if false, remove the domain list from the OVSDB
|        *argv   : argv from the CLI
| Return         : On success returns CMD_SUCCESS,
|                  On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
dnsclient_domain_listconfig (bool set, const char *argv[])
{
    const struct ovsrec_dns_client *dns_client_row = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    enum ovsdb_idl_txn_status txn_status;
    DNS_CLIENT_FIELDS field = DOMAINLIST;

    if (status_txn == NULL)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    /* Validate the user input domain list */
    if (isalpha((int) *argv[0]))
    {
        if (strlen((char*)argv[0]) > MAX_DOMAIN_LIST_LEN)
        {
            vty_out(vty, "Domain list should be less than %d characters%s",
                         MAX_DOMAIN_LIST_LEN, VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }
    }
    else
    {
        vty_out(vty, "Invalid Domain list%s", VTY_NEWLINE);
        cli_do_config_abort(status_txn);
        return CMD_SUCCESS;
    }

    /* lookup for the record in DNS Client table */
    dns_client_row = get_dnsclient_row();

    if (set)
    {
        /* Configure the DNS Client domain list */
        if (dns_client_row == NULL)
        {
            /*
             * First set of DNS domain list
             */
            dns_client_row = set_dnsclient_row(status_txn);
            if (!dns_client_row)
                return CMD_OVSDB_FAILURE;
        }

        /* Verify if the maximum domain lists are configured */
        if (dns_client_row->n_domain_list >= MAX_DOMAIN_LIST)
        {
            vty_out(vty, "Maximum domain lists are configured, "
                         "Failed to configure %s%s", (char *)argv[0],
                          VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }

        /* Update the domain list */
        if (!set_dnsclient_fields
            (dns_client_row, set, (char *)argv[0], field))
        {
            vty_out(vty, "Domain list entry already present.%s",
                          VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }
    }
    else
    {
        /* Unconfigure the DNS Client domain list */
        if (dns_client_row == NULL)
        {
             vty_out(vty, "DNS Client domain list is not present.%s",
                     VTY_NEWLINE);
             cli_do_config_abort(status_txn);
             return CMD_SUCCESS;
        }

        /* Verify if any domain list is configured */
        if (dns_client_row->n_domain_list <= 0)
        {
            vty_out(vty, "No domain list is configured.%s",
                         VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }

        /* Update the domain list */
        if (!set_dnsclient_fields
            (dns_client_row, set, (char *)argv[0], field))
        {
            vty_out(vty, "Domain list entry is not present.%s",
                     VTY_NEWLINE);
             cli_do_config_abort(status_txn);
             return CMD_SUCCESS;
        }
    }

    txn_status = cli_do_config_finish(status_txn);

    if (txn_status == TXN_SUCCESS || txn_status == TXN_UNCHANGED)
    {
        return CMD_SUCCESS;
    }
    else
    {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
        return CMD_OVSDB_FAILURE;
    }
}

/*-----------------------------------------------------------------------------
| Function       : dnsclient_server_addressconfig
| Responsibility : Configure/Unconfigure server address to lookup
|                  for DNS Client.
| Parameters     :
|        set     : If true, add a server address to the OVSDB and
|                  if false, remove the server address from the OVSDB
|        *argv   : argv from the CLI
| Return         : On success returns CMD_SUCCESS,
|                  On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
dnsclient_server_addressconfig (bool set, const char *argv[])
{
    const struct ovsrec_dns_client *dns_client_row = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    enum ovsdb_idl_txn_status txn_status;
    DNS_CLIENT_FIELDS field = NAMESERVER;
    struct in_addr addr;
    struct in6_addr addrv6;
    bool isIPv4 = true;

    if (status_txn == NULL)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    /* Identify the user input name server IP family */
    if (inet_pton (AF_INET, (char *)argv[0], &addr)<= 0)
    {
        if(inet_pton(AF_INET6, (char *)argv[0], &addrv6) <= 0)
        {
            vty_out(vty, "Invalid IPv4 or IPv6 address%s",
                    VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }
        isIPv4 = false;
    }

    /* Validate the IP */
    if ((isIPv4) && (!IS_VALID_IPV4(htonl(addr.s_addr))))
    {
        vty_out(vty, "IPv4 :Broadcast, multicast and "
                     "loopback addresses are not allowed%s",
                     VTY_NEWLINE);
        cli_do_config_abort(status_txn);
        return CMD_SUCCESS;
    }

    if ((!isIPv4) && (!IS_VALID_IPV6(&addrv6)))
    {
        vty_out(vty, "IPv6 :Broadcast, multicast and "
                     "loopback addresses are not allowed%s",
                     VTY_NEWLINE);
        cli_do_config_abort(status_txn);
        return CMD_SUCCESS;
    }

    /* lookup for the record in DNS Client table */
    dns_client_row = get_dnsclient_row();

    if (set)
    {
        /* Configure the DNS Client server address */
        if (dns_client_row == NULL)
        {
            /*
             * First set of DNS domain name
             */
            dns_client_row = set_dnsclient_row(status_txn);
            if (!dns_client_row)
                return CMD_OVSDB_FAILURE;
        }

        /* Verify if the maximum name servers are configured */
        if (dns_client_row->n_name_servers >= MAX_NAME_SERVER)
        {
            vty_out(vty, "Maximum name servers are configured, "
                         "Failed to configure %s%s", (char *)argv[0],
                          VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }

        /* Update the name server */
        if (!set_dnsclient_fields
            (dns_client_row, set, (char *)argv[0], field))
        {
            vty_out(vty, "Name server entry already present.%s",
                          VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }
    }
    else
    {
        /* Unconfigure the DNS Client name server */
        if (dns_client_row == NULL)
        {
             vty_out(vty, "DNS Client name server is not present.%s",
                     VTY_NEWLINE);
             cli_do_config_abort(status_txn);
             return CMD_SUCCESS;
        }

        /* Verify if any name server is configured */
        if (dns_client_row->n_name_servers <= 0)
        {
            vty_out(vty, "No name serevr is configured.%s",
                         VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }

        /* Update the domain list */
        if (!set_dnsclient_fields
            (dns_client_row, set, (char *)argv[0], field))
        {
            vty_out(vty, "Name server entry is not present.%s",
                     VTY_NEWLINE);
             cli_do_config_abort(status_txn);
             return CMD_SUCCESS;
        }
    }

    txn_status = cli_do_config_finish(status_txn);

    if (txn_status == TXN_SUCCESS || txn_status == TXN_UNCHANGED)
    {
        return CMD_SUCCESS;
    }
    else
    {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
        return CMD_OVSDB_FAILURE;
    }
}

/*-----------------------------------------------------------------------------
| Function        : max_host_reached
| Responsibility  : Verify if the hosts configured has not exceeded the
|                   maximum allowed hosts count.
| Parameters      :
|  dns_client_row : This will point to the dns client row
| Return          : If number of hosts count has reached the maximum
|                   allowed hosts return true, else false
-----------------------------------------------------------------------------*/
bool
max_host_reached (const struct ovsrec_dns_client *dns_client_row, char *hostname)
{
    struct smap smapv6, smap;
    struct smap_node *node;
    char *hostpresent = NULL;
    bool isMaxCountReached = false;

    smap_clone(&smap, &dns_client_row->host_v4_address_mapping);
    smap_clone(&smapv6, &dns_client_row->host_v6_address_mapping);

    SMAP_FOR_EACH (node, &smapv6)
    {
        smap_replace(&smap, node->key, node->value);
    }

    hostpresent = (char *)smap_get(&smap, hostname);
    if (!hostpresent && smap_count(&smap) >= MAX_HOSTS)
    {
        isMaxCountReached = true;
    }

    smap_destroy(&smap);
    smap_destroy(&smapv6);

    return isMaxCountReached;
}

/*-----------------------------------------------------------------------------
| Function       : dnsclient_host_config
| Responsibility : Configure/Unconfigure hosts to lookup
|                  for DNS Client.
| Parameters     :
|        set     : If true, add a hostname to the OVSDB and
|                  if false, remove hostname from the OVSDB
|        *argv   : argv from the CLI
| Return         : On success returns CMD_SUCCESS,
|                  On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
dnsclient_host_config (bool set, const char *argv[])
{
    const struct ovsrec_dns_client *dns_client_row = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    enum ovsdb_idl_txn_status txn_status;
    struct smap smap;
    struct smap_node *node;
    struct in_addr addr;
    struct in6_addr addrv6;
    bool isIPv4 = true, hostFound = false;
    char *hostname =  NULL, *hostIP = NULL;
    char *curHostName = NULL, *curHostIP = NULL;

    if (status_txn == NULL)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    /* Fetch the hostname and host IP */
    hostname = (char *)argv[0];
    hostIP = (char *)argv[1];

    /* Validate the user input hostname */
    if (isalpha((int) *argv[0]))
    {
        if (strlen((char*)argv[0]) > MAX_HOST_NAME_LEN)
        {
            vty_out(vty, "Host name should be less than %d characters%s",
                         MAX_HOST_NAME_LEN, VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }
    }
    else
    {
        vty_out(vty, "Invalid host name%s", VTY_NEWLINE);
        cli_do_config_abort(status_txn);
        return CMD_SUCCESS;
    }

    /* Identify the user input host IP family */
    if (inet_pton (AF_INET, (char *)argv[1], &addr)<= 0)
    {
        if(inet_pton(AF_INET6, (char *)argv[1], &addrv6) <= 0)
        {
            vty_out(vty, "Invalid IPv4 or IPv6 address%s",
                    VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }
        isIPv4 = false;
    }

    /* Validate the host IP */
    if ((isIPv4) && (!IS_VALID_IPV4(htonl(addr.s_addr))))
    {
        vty_out(vty, "IPv4 :Broadcast, multicast and "
                     "loopback addresses are not allowed%s",
                     VTY_NEWLINE);
        cli_do_config_abort(status_txn);
        return CMD_SUCCESS;
    }

    if ((!isIPv4) && (!IS_VALID_IPV6(&addrv6)))
    {
        vty_out(vty, "IPv6 :Broadcast, multicast and "
                     "loopback addresses are not allowed%s",
                     VTY_NEWLINE);
        cli_do_config_abort(status_txn);
        return CMD_SUCCESS;
    }

    /* lookup for the record in DNS Client table */
    dns_client_row = get_dnsclient_row();

    if (set)
    {
        /* Configure the DNS Client hostname */
        if (dns_client_row == NULL)
        {
            /*
             * First set of DNS Client hostname
             */
            dns_client_row = set_dnsclient_row(status_txn);
            if (!dns_client_row)
                return CMD_OVSDB_FAILURE;
        }

        if (max_host_reached(dns_client_row, hostname))
        {
            vty_out(vty, "Maximum hosts are configured, "
                         "Failed to configure %s%s", (char *)argv[0],
                          VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }

        if (isIPv4)
            smap_clone(&smap, &dns_client_row->host_v4_address_mapping);
        else
            smap_clone(&smap, &dns_client_row->host_v6_address_mapping);

        /* Update the host entry */
        smap_replace(&smap, hostname, hostIP);
    }
    else
    {
        /* Unconfigure the DNS Client host name */
        if (dns_client_row == NULL)
        {
             vty_out(vty, "Static host configuration is not present.%s",
                     VTY_NEWLINE);
             cli_do_config_abort(status_txn);
             return CMD_SUCCESS;
        }

        /* Fetch the existing record */
        if (isIPv4)
            smap_clone(&smap, &dns_client_row->host_v4_address_mapping);
        else
            smap_clone(&smap, &dns_client_row->host_v6_address_mapping);

        SMAP_FOR_EACH (node, &smap)
        {
            curHostName = node->key;
            if(!strncmp(curHostName, hostname, strlen(hostname)))
            {
                curHostIP = node->value;
                hostFound = true;
                break;
            }
        }

        if (hostFound)
        {
            if (!strncmp(curHostIP, hostIP, strlen(hostIP)))
            {
                smap_remove(&smap, hostname);
            }
            else
            {
                vty_out(vty, "host entry not found.%s",
                             VTY_NEWLINE);
                cli_do_config_abort(status_txn);
                return CMD_SUCCESS;
            }
        }
        else
        {
            vty_out(vty, "host entry not found.%s",
                         VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }

    }

    if (isIPv4)
        ovsrec_dns_client_set_host_v4_address_mapping(dns_client_row, &smap);
    else
        ovsrec_dns_client_set_host_v6_address_mapping(dns_client_row, &smap);

    txn_status = cli_do_config_finish(status_txn);
    smap_destroy(&smap);

    if (txn_status == TXN_SUCCESS || txn_status == TXN_UNCHANGED)
    {
        return CMD_SUCCESS;
    }
    else
    {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
        return CMD_OVSDB_FAILURE;
    }
}

/*-----------------------------------------------------------------------------
| Function       : show_dns_client
| Responsibility : show the DNS Client configurations
| Return         : On success returns CMD_SUCCESS,
|                  On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
show_dns_client(void)
{
    const struct ovsrec_system *ovs_row = NULL;
    const struct ovsrec_dns_client *dns_client_row = NULL;
    struct smap smap;
    struct smap_node *node;
    char *dns_client_status = NULL, *hostname = NULL, *hostIP = NULL;
    int8_t i = 0;

    ovs_row = ovsrec_system_first(idl);
    if (!ovs_row)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        return CMD_OVSDB_FAILURE;
    }

    vty_out(vty, "%s", VTY_NEWLINE);
    /* Fetch the global DNS Client status */
    smap_clone(&smap, &ovs_row->other_config);
    dns_client_status = (char *)smap_get(&smap,
                        SYSTEM_OTHER_CONFIG_MAP_DNS_CLIENT_DISABLED);

    if (!dns_client_status)
    {
        dns_client_status = "Enabled";
    }
    else
    {
        if (!strncmp(dns_client_status,
                     "true", strlen(dns_client_status)))
            dns_client_status = "Disabled";
        else
            dns_client_status = "Enabled";
    }

    vty_out(vty, "DNS Client Mode%s: %s%s", "",
                 dns_client_status, VTY_NEWLINE);

    OVSREC_DNS_CLIENT_FOR_EACH (dns_client_row, idl)
    {
        /* Fetch the DNS Client domain name */
        smap_clone(&smap, &dns_client_row->other_config);
        dns_client_status = (char *)smap_get(&smap, DOMAIN_NAME);

        if (dns_client_status)
        {
            vty_out(vty, "Domain Name : %s%s", dns_client_status, VTY_NEWLINE);
        }

        /* Fetch the DNS Client domain  list */
        if (dns_client_row->n_domain_list > 0)
        {
            vty_out(vty, "DNS Domain list : ");
            for(i = 0; i < dns_client_row->n_domain_list; i++)
            {
                if (i != dns_client_row->n_domain_list - 1)
                    vty_out(vty, "%s, ", dns_client_row->domain_list[i]);
                else
                    vty_out(vty, "%s%s", dns_client_row->domain_list[i],
                            VTY_NEWLINE);
            }
        }

        /* Fetch the DNS Client name servers */
        if (dns_client_row->n_name_servers > 0)
        {
            vty_out(vty, "Name Server(s) : ");
            for(i = 0; i < dns_client_row->n_name_servers; i++)
            {
                if (i != dns_client_row->n_name_servers - 1)
                    vty_out(vty, "%s, ", dns_client_row->name_servers[i]);
                else
                    vty_out(vty, "%s%s", dns_client_row->name_servers[i],
                            VTY_NEWLINE);
            }
        }

        vty_out(vty, "%s", VTY_NEWLINE);
        vty_out(vty, "Host Name%4sAddress%s", "", VTY_NEWLINE);
        vty_out(vty, "-------------------------------%s", VTY_NEWLINE);

        /* Fetch the DNS Client static hosts */
        smap_clone(&smap, &dns_client_row->host_v4_address_mapping);
        SMAP_FOR_EACH(node, &smap)
        {
            hostname = node->key;
            hostIP = node->value;
            vty_out(vty, "%s%12s%s%s", hostname, "",
                         hostIP, VTY_NEWLINE);
        }

        smap_clone(&smap, &dns_client_row->host_v6_address_mapping);
        SMAP_FOR_EACH(node, &smap)
        {
            hostname = node->key;
            hostIP = node->value;
            vty_out(vty, "%s%12s%s%s", hostname, "",
                         hostIP, VTY_NEWLINE);
        }
    }

    smap_destroy(&smap);
    return CMD_SUCCESS;
}

/* FIXME - This support for global command is removed from the schema,
        but the schema has not been finalised. depending on the schema
        this API has to be kept or removed.
*/
/*-----------------------------------------------------------------------------
| Defun for DNS Client
| Responsibility: DNS Client enable
-----------------------------------------------------------------------------*/
DEFUN ( cli_dns_client_enable,
        cli_dns_client_enable_cmd,
        "ip dns",
        IP_STR
        DNS_STR )
{
      return dnsclient_globalconfig("false");
}

/* FIXME - This support for global command is removed from the schema,
        but the schema has not been finalised. depending on the schema
        this API has to be kept or removed.
*/
/*-----------------------------------------------------------------------------
| Defun for DNS Client
| Responsibility: DNS Client disable
-----------------------------------------------------------------------------*/
DEFUN ( cli_dns_client_disable,
        cli_dns_client_disable_cmd,
        "no ip dns",
        NO_STR
        IP_STR
        DNS_STR )
{
      return dnsclient_globalconfig("true");
}

/*-----------------------------------------------------------------------------
| Defun for DNS Client domain name
| Responsibility: Configure a DNS Client domain name
-----------------------------------------------------------------------------*/
DEFUN ( cli_dns_client_domain_name,
        cli_dns_client_domain_name_cmd,
        "ip dns domain-name WORD",
        IP_STR
        DNS_STR
        DOMAIN_NAME_STR
        DOMAIN_NAME_WORD_STR )
{
    return dnsclient_domain_nameconfig(true, argv);
}

/*-----------------------------------------------------------------------------
| Defun for DNS Client domain name
| Responsibility: Unconfigure a DNS Client domain name
-----------------------------------------------------------------------------*/
DEFUN ( cli_no_dns_client_domain_name,
        cli_no_dns_client_domain_name_cmd,
        "no ip dns domain-name WORD",
        NO_STR
        IP_STR
        DNS_STR
        DOMAIN_NAME_STR
        DOMAIN_NAME_WORD_STR)
{
    return dnsclient_domain_nameconfig(false, argv);
}

/*-----------------------------------------------------------------------------
| Defun for DNS Client domain list
| Responsibility: Configure a DNS Client domain list
-----------------------------------------------------------------------------*/
DEFUN ( cli_dns_client_domain_list,
        cli_dns_client_domain_list_cmd,
        "ip dns domain-list WORD",
        IP_STR
        DNS_STR
        DOMAIN_LIST_STR
        DOMAIN_LIST_WORD_STR )
{
    return dnsclient_domain_listconfig(true, argv);
}

/*-----------------------------------------------------------------------------
| Defun for DNS Client domain list
| Responsibility: Unconfigure a DNS Client domain list
-----------------------------------------------------------------------------*/
DEFUN ( cli_no_dns_client_domain_list,
        cli_no_dns_client_domain_list_cmd,
        "no ip dns domain-list WORD",
        NO_STR
        IP_STR
        DNS_STR
        DOMAIN_LIST_STR
        DOMAIN_LIST_WORD_STR )
{
    return dnsclient_domain_listconfig(false, argv);
}

/*-----------------------------------------------------------------------------
| Defun for DNS Client server address
| Responsibility: Configure a DNS Client server address
-----------------------------------------------------------------------------*/
DEFUN ( cli_dns_client_server_address,
        cli_dns_client_server_address_cmd,
        "ip dns server-address (A.B.C.D | X:X::X:X)",
        IP_STR
        DNS_STR
        SERVER_ADDR_STR
        SERVER_IPv4
        SERVER_IPv6 )
{
    return dnsclient_server_addressconfig(true, argv);
}

/*-----------------------------------------------------------------------------
| Defun for DNS Client server address
| Responsibility: Unconfigure a DNS Client server address
-----------------------------------------------------------------------------*/
DEFUN ( cli_no_dns_client_server_address,
        cli_no_dns_client_server_address_cmd,
        "no ip dns server-address (A.B.C.D | X:X::X:X)",
        NO_STR
        IP_STR
        DNS_STR
        SERVER_ADDR_STR
        SERVER_IPv4
        SERVER_IPv6 )
{
    return dnsclient_server_addressconfig(false, argv);
}

/*-----------------------------------------------------------------------------
| Defun for DNS Client hosts
| Responsibility: Configure a DNS Client host
-----------------------------------------------------------------------------*/
DEFUN ( cli_dns_client_host_name,
        cli_dns_client_host_name_cmd,
        "ip dns host WORD (A.B.C.D | X:X::X:X)",
        IP_STR
        DNS_STR
        HOST_STR
        HOST_NAME_STR
        SERVER_IPv4
        SERVER_IPv6 )
{
    return dnsclient_host_config(true, argv);
}

/*-----------------------------------------------------------------------------
| Defun for DNS Client hosts
| Responsibility: Unconfigure a DNS Client host
-----------------------------------------------------------------------------*/
DEFUN ( cli_no_dns_client_host_name,
        cli_no_dns_client_host_name_cmd,
        "no ip dns host WORD (A.B.C.D | X:X::X:X)",
        NO_STR
        IP_STR
        DNS_STR
        HOST_STR
        HOST_NAME_STR
        SERVER_IPv4
        SERVER_IPv6 )
{
    return dnsclient_host_config(false, argv);
}

/*-----------------------------------------------------------------------------
| Defun show DNS Client
| Responsibility: Show the DNS Client configurations
-----------------------------------------------------------------------------*/
DEFUN ( cli_show_dns_client,
        cli_show_dns_client_cmd,
        "show ip dns",
        SHOW_STR
        IP_STR
        DNS_STR )
{
    return show_dns_client();
}

/*******************************************************************
| Function       : dnsclient_ovsdb_init
| Responsibility : Initialise DNS Client table details.
| Parameters     :
|      idl       : Pointer to idl structure
 *******************************************************************/
static void
dnsclient_ovsdb_init(void)
{
    ovsdb_idl_add_table(idl, &ovsrec_table_system);
    ovsdb_idl_add_column(idl, &ovsrec_system_col_other_config);
    ovsdb_idl_add_table(idl, &ovsrec_table_dns_client);
    ovsdb_idl_add_column(idl, &ovsrec_dns_client_col_domain_list);
    ovsdb_idl_add_column(idl, &ovsrec_dns_client_col_host_v4_address_mapping);
    ovsdb_idl_add_column(idl, &ovsrec_dns_client_col_host_v6_address_mapping);
    ovsdb_idl_add_column(idl, &ovsrec_dns_client_col_other_config);
    ovsdb_idl_add_column(idl, &ovsrec_dns_client_col_name_servers);
    ovsdb_idl_add_column(idl, &ovsrec_dns_client_col_vrf);

    return;
}

/* Install DNS Client related vty commands */
void
cli_pre_init(void)
{
    vtysh_ret_val retval = e_vtysh_error;

    dnsclient_ovsdb_init();
        retval = install_show_run_config_context
                    (e_vtysh_dns_client_context,
                     &vtysh_dns_client_context_clientcallback,
                     NULL, NULL);
    if (e_vtysh_ok != retval)
    {
        vtysh_ovsdb_config_logmsg
                    (VTYSH_OVSDB_CONFIG_ERR,
                     "DNS client context unable "\
                     "to add config callback");
        assert(0);
    }

    return;
}

/* Install DNS Client related vty commands */
void
cli_post_init(void)
{
    install_element(CONFIG_NODE, &cli_dns_client_enable_cmd);
    install_element(CONFIG_NODE, &cli_dns_client_disable_cmd);
    install_element(CONFIG_NODE, &cli_dns_client_domain_name_cmd);
    install_element(CONFIG_NODE, &cli_no_dns_client_domain_name_cmd);
    install_element(CONFIG_NODE, &cli_dns_client_domain_list_cmd);
    install_element(CONFIG_NODE, &cli_no_dns_client_domain_list_cmd);
    install_element(CONFIG_NODE, &cli_dns_client_server_address_cmd);
    install_element(CONFIG_NODE, &cli_no_dns_client_server_address_cmd);
    install_element(CONFIG_NODE, &cli_dns_client_host_name_cmd);
    install_element(CONFIG_NODE, &cli_no_dns_client_host_name_cmd);
    install_element(ENABLE_NODE, &cli_show_dns_client_cmd);

    return;
}

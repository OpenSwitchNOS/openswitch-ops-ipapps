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
#include "dns_client_vty.h"

extern struct ovsdb_idl *idl;
VLOG_DEFINE_THIS_MODULE(vtysh_dns_client_cli);

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
                 "dns_client_disabled", enable);
//SYSTEM_OTHER_CONFIG_MAP_DNS_CLIENT_DISABLED

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
| Function         : dns_client_vrf_lookup
| Responsibility   : To lookup for the default VRF.
| Parameters       :
|        *vrf_name : VRF name that needs to be looked up.
| Return           : On success returns the VRF row,
|                    On failure returns NULL
-----------------------------------------------------------------------------*/
const struct
ovsrec_vrf* dns_client_vrf_lookup (const char *vrf_name)
{
    const struct ovsrec_vrf *vrf_row = NULL;

    OVSREC_VRF_FOR_EACH (vrf_row, idl)
    {
        if (strcmp(vrf_row->name, vrf_name) == 0) {
            return vrf_row;
        }
    }
    return NULL;
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
    const struct ovsrec_vrf *vrf_row = NULL;
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
    dns_client_row = ovsrec_dns_client_first(idl);

    if (set)
    {
        /* Configure the DNS Client domain name */
        if (dns_client_row == NULL)
        {
            /*
             * First set of DNS domain name
             */
            dns_client_row = ovsrec_dns_client_insert(status_txn);
            if (!dns_client_row)
            {
                VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
                cli_do_config_abort(status_txn);
                return CMD_OVSDB_FAILURE;
            }

            vrf_row = dns_client_vrf_lookup(DEFAULT_VRF_NAME);
            if (!vrf_row)
            {
                vty_out(vty, "Error: Could not fetch "
                             "default VRF data.%s", VTY_NEWLINE);
                VLOG_ERR("%s VRF table did not have any rows. "
                         "Ideally it should have just one entry.", __func__);
                cli_do_config_abort(status_txn);
                return CMD_OVSDB_FAILURE;
            }
            ovsrec_dns_client_set_vrf(dns_client_row, vrf_row);
        }
        /* Update the domain name */
        smap_clone(&smap, &dns_client_row->other_config);
        smap_replace(&smap, DNS_DOMAIN_NAME, (char *)argv[0]);
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
            domain_name = (char *)smap_get(&smap, DNS_DOMAIN_NAME);
            if (domain_name == NULL)
            {
                 vty_out(vty, "No domain name configuration is present.%s",
                         VTY_NEWLINE);
                 cli_do_config_abort(status_txn);
                 return CMD_SUCCESS;
            }

            if (!strcmp(domain_name, (char *)argv[0]))
            {
                /* domain name exists, so delete it */
                smap_remove(&smap, DNS_DOMAIN_NAME);
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
| Function         : update_dnsclient_fields
| Responsibility   : To update the multiple fields in the DNS Client table.
| Parameters       :
|   dns_client_row : This will point to the dns client row
|   set            : Set/Unset a specific fields from the DNS Client table
|   value          : User input details
|   field          : Field to be updated in the DNS Client table
-----------------------------------------------------------------------------*/
bool
update_dnsclient_fields
        (const struct ovsrec_dns_client *dns_client_row,
         bool set, char *value, DNS_CLIENT_FIELDS field)
{
    char **dnsClient, **dnsClientArray;
    bool entryFound = false;
    size_t i, n, maxLength = 0;
    size_t dnsclient_length = 0, dnsClientCount = 0;

    if (field == domList)
    {
        dnsClientCount = dns_client_row->n_domain_list;
        dnsClientArray = dns_client_row->domain_list;
        maxLength = MAX_DOMAIN_LIST_LEN;
    }
    else if (field == nameServ)
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
        for (i = 0; i < dnsClientCount; i++)
        {
            if (!strcmp(value, dnsClientArray[i]))
                entryFound = true;
            else
                dnsClient[i] = dnsClientArray[i];
        }

        dnsClient[dnsClientCount] = value;
        if (!entryFound)
        {
            /* Entry does not exist, so update in the column */
            if (field == domList)
            {
                ovsrec_dns_client_set_domain_list
                    (dns_client_row, dnsClient, dnsclient_length);
            }
            else if (field == nameServ)
            {
                ovsrec_dns_client_set_name_servers
                    (dns_client_row, dnsClient, dnsclient_length);
            }
        }
    }
    else
    {
        dnsclient_length = dnsClientCount - 1;
        dnsClient = xmalloc(maxLength * dnsclient_length);

        /* Delete the domain list from the table */
        for (i = n = 0; i < dnsClientCount; i++)
        {
            if (strcmp (value, dnsClientArray[i]) != 0)
                dnsClient[n++] = dnsClientArray[i];
            else
                entryFound = true;
        }
        if (entryFound)
        {
            /* Pre existing entry, delete it in the column */
            if (field == domList)
            {
                ovsrec_dns_client_set_domain_list
                    (dns_client_row, dnsClient, n);
            }
            else if (field == nameServ)
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
    const struct ovsrec_vrf *vrf_row = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    enum ovsdb_idl_txn_status txn_status;
    DNS_CLIENT_FIELDS field = domList;

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
    dns_client_row = ovsrec_dns_client_first(idl);

    if (set)
    {
        /* Configure the DNS Client domain list */
        if (dns_client_row == NULL)
        {
            /*
             * First set of DNS domain list
             */
            dns_client_row = ovsrec_dns_client_insert(status_txn);
            if (!dns_client_row)
            {
                VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
                cli_do_config_abort(status_txn);
                return CMD_OVSDB_FAILURE;
            }

            vrf_row = dns_client_vrf_lookup(DEFAULT_VRF_NAME);
            if (!vrf_row)
            {
                vty_out(vty, "Error: Could not fetch "
                             "default VRF data.%s", VTY_NEWLINE);
                VLOG_ERR("%s VRF table did not have any rows. "
                         "Ideally it should have just one entry.", __func__);
                cli_do_config_abort(status_txn);
                return CMD_OVSDB_FAILURE;
            }
            ovsrec_dns_client_set_vrf(dns_client_row, vrf_row);
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
        if (update_dnsclient_fields
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
        if (!update_dnsclient_fields
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
    const struct ovsrec_vrf *vrf_row = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    enum ovsdb_idl_txn_status txn_status;
    DNS_CLIENT_FIELDS field = nameServ;
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
    dns_client_row = ovsrec_dns_client_first(idl);

    if (set)
    {
        /* Configure the DNS Client server address */
        if (dns_client_row == NULL)
        {
            /*
             * First set of DNS domain name
             */
            dns_client_row = ovsrec_dns_client_insert(status_txn);
            if (!dns_client_row)
            {
                VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
                cli_do_config_abort(status_txn);
                return CMD_OVSDB_FAILURE;
            }

            vrf_row = dns_client_vrf_lookup(DEFAULT_VRF_NAME);
            if (!vrf_row)
            {
                vty_out(vty, "Error: Could not fetch "
                             "default VRF data.%s", VTY_NEWLINE);
                VLOG_ERR("%s VRF table did not have any rows. "
                         "Ideally it should have just one entry.", __func__);
                cli_do_config_abort(status_txn);
                return CMD_OVSDB_FAILURE;
            }
            ovsrec_dns_client_set_vrf(dns_client_row, vrf_row);
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
        if (update_dnsclient_fields
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
        if (!update_dnsclient_fields
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
| Function       : isHostPresent
| Responsibility : Check if the user input host is already present.
| Parameters     :
|  smap          : smap entry
|  *curDnsHost   : data sruct to hold the existing host details
|  hostname      : host name entrered by the user
| Return         : If true, host exists and curDnsHost has
|                  the existing host details
|                  If false, host does not exist and curDnsHost is NULL
-----------------------------------------------------------------------------*/
bool
isHostPresent (struct smap smap, dnsHost *curDnsHost, char *hostname)
{
    struct smap_node *node;
    bool hostFound = false;
    struct in_addr addr;
    char addr1[45], addr2[45], host_address[MAX_HOST_ADDR_LEN];
    char *token = NULL;

    SMAP_FOR_EACH (node, &smap)
    {
        if(!strcmp(xstrdup(node->key), hostname))
        {
            hostFound = true;
            break;
        }
    }

    if (hostFound)
    {
        curDnsHost->hostname = hostname;
        strncpy(host_address, (char *)smap_get(&smap, hostname),
                MAX_HOST_ADDR_LEN);

        /* Fetch the present IP address */
        strcpy(addr1, strtok(host_address, " "));
        token = addr1;
        /* Check if the hostname has both IPv4/v6 entries */
        while (token != NULL)
        {
            /* hostname has both IPv4 and IPv6 */
            /* fetch the IPv6 address */
            strcpy(addr2, token);
            token = strtok(NULL, " ");
        }

        if (!strcmp(addr1, addr2))
        {
            if (inet_pton (AF_INET, addr1, &addr) <= 0)
            {
                curDnsHost->hostIpv4 = NULL;
                curDnsHost->hostIpv6 = addr1;
            }
            else
            {
                curDnsHost->hostIpv4 = addr1;
                curDnsHost->hostIpv6 = NULL;
            }
        }
        else
        {
            curDnsHost->hostIpv4 = addr1;
            curDnsHost->hostIpv6 = addr2;
        }
    }
    else
    {
        /* No entries are found */
        curDnsHost->hostname = NULL;
        curDnsHost->hostIpv4 = NULL;
        curDnsHost->hostIpv6 = NULL;
    }

    return hostFound;
}

/*-----------------------------------------------------------------------------
| Function       : dnsclient_host_nameconfig
| Responsibility : Configure/Unconfigure static hostname to lookup
|                  for DNS Client.
| Parameters     :
|        set     : If true, add a hostname to the OVSDB and
|                  if false, remove hostname from the OVSDB
|        *argv   : argv from the CLI
| Return         : On success returns CMD_SUCCESS,
|                  On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
dnsclient_host_nameconfig (bool set, const char *argv[])
{
    const struct ovsrec_dns_client *dns_client_row = NULL;
    const struct ovsrec_vrf *vrf_row = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    enum ovsdb_idl_txn_status txn_status;
    struct smap smap;
    struct in_addr addr;
    struct in6_addr addrv6;
    bool isIPv4 = true;
    char *hostname =  NULL, *hostIP = NULL;
    char entry[MAX_HOST_ADDR_LEN];
    dnsHost presentDnshost;

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
    dns_client_row = ovsrec_dns_client_first(idl);

    if (set)
    {
        /* Configure the DNS Client hostname */
        if (dns_client_row == NULL)
        {
            /*
             * First set of DNS Client hostname
             */
            dns_client_row = ovsrec_dns_client_insert(status_txn);
            if (!dns_client_row)
            {
                VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
                cli_do_config_abort(status_txn);
                return CMD_OVSDB_FAILURE;
            }

            vrf_row = dns_client_vrf_lookup(DEFAULT_VRF_NAME);
            if (!vrf_row)
            {
                vty_out(vty, "Error: Could not fetch "
                             "default VRF data.%s", VTY_NEWLINE);
                VLOG_ERR("%s VRF table did not have any rows. "
                         "Ideally it should have just one entry.", __func__);
                cli_do_config_abort(status_txn);
                return CMD_OVSDB_FAILURE;
            }
            ovsrec_dns_client_set_vrf(dns_client_row, vrf_row);
        }

        smap_clone(&smap, &dns_client_row->static_host_address_mapping);
        /* Verify if the maximum static hostnames are configured */
        if (smap_count(&smap) >= MAX_HOST_NAME)
        {
            vty_out(vty, "Maximum hosts are configured, "
                         "Failed to configure %s %s%s", hostname,
                          hostIP, VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }

        /* Check if the host is present */
        if(isHostPresent(smap, &presentDnshost, hostname))
        {
            if (presentDnshost.hostIpv4 && presentDnshost.hostIpv6)
            {
                /* host has both IPv4/v6 configured */
                if (isIPv4)
                    /* update the IPv4 */
                    presentDnshost.hostIpv4 = hostIP;
                else
                    /* update the IPv6 */
                    presentDnshost.hostIpv6 = hostIP;

                /* update the new configuration entry */
                sprintf(entry, "%s %s",
                        presentDnshost.hostIpv4,
                        presentDnshost.hostIpv6);
                hostIP = entry;
            }
            else
            {
                /* host has either IPv4 or IPv6 configured */
                if (presentDnshost.hostIpv4)
                {
                    if (!isIPv4)
                    {
                        /* update the host IPv6 */
                        presentDnshost.hostIpv6 = hostIP;
                        sprintf(entry, "%s %s",
                                presentDnshost.hostIpv4,
                                presentDnshost.hostIpv6);
                        hostIP = entry;
                    }
                }
                else if (presentDnshost.hostIpv6)
                {
                    if (isIPv4)
                    {
                        /* update the host IPv4 */
                        presentDnshost.hostIpv4 = hostIP;
                        sprintf(entry, "%s %s",
                                presentDnshost.hostIpv4,
                                presentDnshost.hostIpv6);
                        hostIP = entry;
                    }
                }
            }
        }
        /* Update the static hostname */
        smap_replace(&smap, hostname, hostIP);
    }
    else
    {
        /* Unconfigure the DNS Client host name */
        if (dns_client_row == NULL)
        {
             vty_out(vty, "DNS Client static hostname is not present.%s",
                     VTY_NEWLINE);
             cli_do_config_abort(status_txn);
             return CMD_SUCCESS;
        }

        /* Fetch the existing record */
        smap_clone(&smap, &dns_client_row->static_host_address_mapping);
        if (smap_count(&smap) <= 0)
        {
            vty_out(vty, "No hosts are configured, "
                         "Failed to configure %s %s%s", hostname,
                          hostIP, VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }

        /* Verify if the user entered domain name exists */
        if (isHostPresent (smap, &presentDnshost, hostname))
        {
            if (presentDnshost.hostIpv4 && presentDnshost.hostIpv6)
            {
                /* host entry has both IPv4/v6 */
                if (isIPv4)
                {
                    /* replace the IPv4 host IP */
                    if (!strcmp(presentDnshost.hostIpv4, hostIP))
                    {
                        sprintf(entry, "%s", presentDnshost.hostIpv6);
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
                    /* replace the IPv6 host IP */
                    if (!strcmp(presentDnshost.hostIpv6, hostIP))
                    {
                        sprintf(entry, "%s", presentDnshost.hostIpv4);
                    }
                    else
                    {
                        vty_out(vty, "host entry not found.%s",
                                VTY_NEWLINE);
                        cli_do_config_abort(status_txn);
                        return CMD_SUCCESS;
                    }
                }
                hostIP = entry;
                smap_replace(&smap, hostname, hostIP);
            }
            else
            {
                /* either IPv4 or IPv6 host IP is present */
                if (presentDnshost.hostIpv4)
                {
                    if (isIPv4)
                    {
                        if (!strcmp(presentDnshost.hostIpv4, hostIP))
                        {
                            /* hostname exists, so delete it */
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
                else if (presentDnshost.hostIpv6)
                {
                    if (!isIPv4)
                    {
                        if (!strcmp(presentDnshost.hostIpv6, hostIP))
                        {
                            /* hostname exists, so delete it */
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

    ovsrec_dns_client_set_static_host_address_mapping(dns_client_row, &smap);
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
    char *dns_client_status = NULL, *domain_name = NULL, *token = NULL;
    int8_t i = 0;

    ovs_row = ovsrec_system_first(idl);
    if (!ovs_row)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        return CMD_OVSDB_FAILURE;
    }

    dns_client_row = ovsrec_dns_client_first(idl);
    if (!dns_client_row)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        return CMD_OVSDB_FAILURE;
    }

    vty_out(vty, "%s", VTY_NEWLINE);
    /* Fetch the global DNS Client status */
    smap_clone(&smap, &ovs_row->other_config);
    dns_client_status = (char *)smap_get(&smap, "dns_client_disabled");

    if (dns_client_status == NULL)
    {
        vty_out(vty, "DNS Mode%8s: Disabled %s", "", VTY_NEWLINE);
    }
    else
    {
        if (!strcmp(dns_client_status, "true")) {
            vty_out(vty, "DNS Mode%8s: Enabled %s", "", VTY_NEWLINE);
        } else if (!strcmp(dns_client_status, "false")) {
            vty_out(vty, "DNS Mode%8s: Disabled %s", "", VTY_NEWLINE);
        }
    }

    /* Fetch the DNS Client domain name */
    smap_clone(&smap, &dns_client_row->other_config);
    domain_name = (char *)smap_get(&smap, DNS_DOMAIN_NAME);

    if (domain_name == NULL)
    {
        vty_out(vty, "DNS Domain Name : default %s", VTY_NEWLINE);
    }
    else
    {
        vty_out(vty, "DNS Domain Name : %s%s", domain_name, VTY_NEWLINE);
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
        vty_out(vty, "DNS Server address(es) : ");
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
    vty_out(vty, "DNS Host Name%6sAddress%s", "", VTY_NEWLINE);
    vty_out(vty, "---------------------------%s", VTY_NEWLINE);

    /* Fetch the DNS Client static hosts */
    smap_clone(&smap, &dns_client_row->static_host_address_mapping);
    SMAP_FOR_EACH(node, &smap)
    {
        token = strtok(xstrdup(node->value), " ");
        while (token != NULL)
        {
            vty_out(vty, "%s%12s%s%s", xstrdup(node->key), "",
                    token, VTY_NEWLINE);
            token = strtok(NULL, " ");
        }
    }

    smap_destroy(&smap);
    return CMD_SUCCESS;
}

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
    return dnsclient_host_nameconfig(true, argv);
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
    return dnsclient_host_nameconfig(false, argv);
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
    ovsdb_idl_add_table(idl, &ovsrec_table_dns_client);
    ovsdb_idl_add_column(idl, &ovsrec_dns_client_col_domain_list);
    ovsdb_idl_add_column(idl, &ovsrec_dns_client_col_static_host_address_mapping);
    ovsdb_idl_add_column(idl, &ovsrec_dns_client_col_other_config);
    ovsdb_idl_add_column(idl, &ovsrec_dns_client_col_name_servers);
    ovsdb_idl_add_column(idl, &ovsrec_dns_client_col_vrf);

    return;
}

/* Install DNS Client related vty commands */
void
cli_pre_init(void)
{
    dnsclient_ovsdb_init();
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

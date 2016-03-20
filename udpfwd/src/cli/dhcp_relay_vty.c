/* dhcp-relay CLI commands
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
 * along with GNU Zebra; see the file COPYING.  If not, write to the FreeAC
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * File: dhcp_relay_vty.c
 *
 * Purpose: To add dhcp-relay configuration CLI commands.
 */

#include <sys/un.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <pwd.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "vtysh/lib/version.h"
#include "getopt.h"
#include "vtysh/command.h"
#include "vtysh/memory.h"
#include <stdbool.h>
#include <stdlib.h>
#include "vtysh/vtysh.h"
#include "vswitch-idl.h"
#include "ovsdb-idl.h"
#include "openvswitch/vlog.h"
#include "openswitch-idl.h"
#include "vtysh/prefix.h"
#include "vtysh/vtysh_ovsdb_if.h"
#include "vtysh/vtysh_ovsdb_config.h"
#include "dhcp_relay_vty.h"
#include "vtysh_ovsdb_dhcp_relay_context.h"
#include "udpfwd_vty_utils.h"
#include "udpfwd_common.h"

static int
show_dhcp_relay_config (void);
int8_t
dhcp_relay_config (dhcp_relay *);

extern struct ovsdb_idl *idl;
VLOG_DEFINE_THIS_MODULE(dhcp_relay_vty);

/*-----------------------------------------------------------------------------
| Function       : show_dhcp_relay_config
| Responsibility : To show the dhcp-relay configuration.
| Return         : Returns CMD_SUCCESS
-----------------------------------------------------------------------------*/
static int
show_dhcp_relay_config (void)
{
    const struct ovsrec_system *ovs_row = NULL;
    char *relay_disabled = NULL;
    char *hop_count_increment_disabled = NULL;
    char *relay_option82_enabled = NULL;
    char *relay_option82_validation_enabled = NULL;
    char *relay_option82_policy = NULL;
    char *relay_option82_remote_id = NULL;

    ovs_row = ovsrec_system_first(idl);
    if (ovs_row == NULL)
    {
        VLOG_ERR("%s SYSTEM table did not have any rows.%s",
                 VTY_NEWLINE, VTY_NEWLINE);
        return CMD_SUCCESS;
    }

    /* Display the dhcp-relay global configuration */
    vty_out (vty, "\n DHCP Relay Agent                 :");
    relay_disabled = (char *)smap_get(&ovs_row->dhcp_config,
                             SYSTEM_DHCP_CONFIG_MAP_V4RELAY_DISABLED);

    if (!relay_disabled)
    {
        relay_disabled = "enabled";
    }
    else
    {
        if (!strcmp(relay_disabled, "false"))
            relay_disabled = "enabled";
        else
            relay_disabled = "disabled";
    }

    if (relay_disabled)
    {
        vty_out(vty, "%s%s", relay_disabled, VTY_NEWLINE);
    }

    /* Display the dhcp-relay hop-count increment configuration */
    vty_out (vty, " DHCP Request Hop Count Increment :");
    hop_count_increment_disabled = (char *)smap_get(&ovs_row->dhcp_config,
                SYSTEM_DHCP_CONFIG_MAP_V4RELAY_HOP_COUNT_INCREMENT_DISABLED);

    if (!hop_count_increment_disabled)
    {
        hop_count_increment_disabled = "enabled";
    }
    else
    {
        if (!strcmp(hop_count_increment_disabled, "false"))
            hop_count_increment_disabled = "enabled";
        else
            hop_count_increment_disabled = "disabled";
    }

    if (hop_count_increment_disabled)
    {
        vty_out(vty, "%s%s", hop_count_increment_disabled, VTY_NEWLINE);
    }

    /* Display the dhcp-relay option 82 configuration */
    vty_out (vty, " Option 82                        :");
    relay_option82_enabled = (char *)smap_get(&ovs_row->dhcp_config,
                SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_ENABLED);
    if (!relay_option82_enabled)
    {
        relay_option82_enabled = "disabled";
    }
    else
    {
        if (!strcmp(relay_option82_enabled, "true"))
            relay_option82_enabled = "enabled";
        else
            relay_option82_enabled = "disabled";
    }

    if (relay_option82_enabled)
    {
        vty_out(vty, "%s%s", relay_option82_enabled, VTY_NEWLINE);
    }

    /* Display the dhcp-relay option 82 response validation configuration */
    vty_out (vty, " Response validation              :");
    relay_option82_validation_enabled = (char *)smap_get(&ovs_row->dhcp_config,
                SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_VALIDATION_ENABLED);
    if (!relay_option82_validation_enabled)
    {
        relay_option82_validation_enabled = "disabled";
    }
    else
    {
        if (!strcmp(relay_option82_validation_enabled, "true"))
            relay_option82_validation_enabled = "enabled";
        else
            relay_option82_validation_enabled = "disabled";
    }

    if (relay_option82_validation_enabled)
    {
        vty_out(vty, "%s%s", relay_option82_validation_enabled, VTY_NEWLINE);
    }

    /* Display the dhcp-relay option 82 policy configuration */
    vty_out (vty, " Option 82 handle policy          :");
    relay_option82_policy = (char *)smap_get(&ovs_row->dhcp_config,
                SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_POLICY);
    if (!relay_option82_policy)
    {
        relay_option82_policy = "replace";
    }

    if (relay_option82_policy)
    {
        vty_out(vty, "%s%s", relay_option82_policy, VTY_NEWLINE);
    }

    /* Display the dhcp-relay option 82 remote ID configuration */
    vty_out (vty, " Remote ID                        :");
    relay_option82_remote_id = (char *)smap_get(&ovs_row->dhcp_config,
                SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_REMOTE_ID);
    if (!relay_option82_remote_id)
    {
        relay_option82_remote_id = "mac";
    }

    if (relay_option82_remote_id)
    {
        vty_out(vty, "%s%s", relay_option82_remote_id, VTY_NEWLINE);
    }
    return CMD_SUCCESS;
}


/*-----------------------------------------------------------------------------
| Function       : dhcp_relay_config
| Responsibility : To configure dhcp-relay and option 82.
| Parameters     :
|     *dhcpRelay : Pointer containing user inputdhcp-relay details
| Return         : On success returns CMD_SUCCESS,
|                  On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
dhcp_relay_config (dhcp_relay *dhcpRelay)
{
    const struct ovsrec_system *ovs_row = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    struct smap smap_status_value;
    enum ovsdb_idl_txn_status txn_status;
    char *buff = NULL;
    const char *key1 = NULL, *key2 = NULL, *key3 = NULL;
    const char *key4 = NULL, *key5 = NULL, *key6 = NULL;

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

    /* Assiging the key pairs. */
    key1 = SYSTEM_DHCP_CONFIG_MAP_V4RELAY_DISABLED;
    key2 = SYSTEM_DHCP_CONFIG_MAP_V4RELAY_HOP_COUNT_INCREMENT_DISABLED;
    key3 = SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_ENABLED;
    key4 = SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_VALIDATION_ENABLED;
    key5 = SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_POLICY;
    key6 = SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_REMOTE_ID;

    smap_clone(&smap_status_value, &ovs_row->dhcp_config);

    /* Update the latest dhcp-relay config status. */
    if (dhcpRelay->status == NULL)
    {
        buff = (char *)smap_get(&ovs_row->dhcp_config,
                        SYSTEM_DHCP_CONFIG_MAP_V4RELAY_DISABLED);
        if (buff == NULL)
        {
            smap_add(&smap_status_value, key1, "false");
        }
    }
    else
    {
        smap_replace(&smap_status_value, key1, dhcpRelay->status);
    }

    /* Update the latest dhcp-relay hop-count increment status. */
    if (dhcpRelay->hopCountIncrement == NULL)
    {
        buff = (char *)smap_get(&ovs_row->dhcp_config,
               SYSTEM_DHCP_CONFIG_MAP_V4RELAY_HOP_COUNT_INCREMENT_DISABLED);
        if (buff == NULL)
        {
            smap_add(&smap_status_value, key2, "false");
        }
    }
    else
    {
        smap_replace(&smap_status_value, key2, dhcpRelay->hopCountIncrement);
    }

    /* Update the latest dhcp-relay option 82 config status. */
    if (dhcpRelay->option82Status == NULL)
    {
        buff = (char *)smap_get(&ovs_row->dhcp_config,
               SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_ENABLED);
        if (buff == NULL)
        {
            smap_add(&smap_status_value, key3, "false");
        }
    }
    else
    {
        smap_replace(&smap_status_value, key3, dhcpRelay->option82Status);
    }

    /* Update the latest dhcp-relay option 82 response validation status. */
    if (dhcpRelay->option82ValidationStatus == NULL)
    {
        buff = (char *)smap_get(&ovs_row->dhcp_config,
               SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_VALIDATION_ENABLED);
        if (buff == NULL)
        {
            smap_add(&smap_status_value, key4, "false");
        }
    }
    else
    {
        smap_replace(&smap_status_value, key4, dhcpRelay->option82ValidationStatus);
    }

    /* Update the latest dhcp-relay option 82 forward policy. */
    if (dhcpRelay->option82Policy == NULL)
    {
        buff = (char *)smap_get(&ovs_row->dhcp_config,
               SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_POLICY);
        if (buff == NULL)
        {
            smap_add(&smap_status_value, key5, "replace");
        }
    }
    else
    {
        smap_replace(&smap_status_value, key5, dhcpRelay->option82Policy);
    }

    /* Update the latest dhcp-relay option 82 remote ID. */
    if (dhcpRelay->option82RemoteID == NULL)
    {
        buff = (char *)smap_get(&ovs_row->dhcp_config,
               SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_REMOTE_ID);
        if (buff == NULL)
        {
            smap_add(&smap_status_value, key6, "mac");
        }
    }
    else
    {
        smap_replace(&smap_status_value, key6, dhcpRelay->option82RemoteID);
    }

    ovsrec_system_set_dhcp_config(ovs_row, &smap_status_value);
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
| Defun for dhcp-relay and hop count increment configuration
| Responsibility: Enable dhcp-relay or hop count increment
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_configuration,
      dhcp_relay_configuration_cmd,
      "dhcp-relay {hop-count-increment} ",
      DHCP_RELAY_STR
      HOP_COUNT_INCREMENT_STR)
{
    dhcp_relay dhcpRelay;
    memset(&dhcpRelay, 0, sizeof(dhcp_relay));

    dhcpRelay.hopCountIncrement = "false";

    return dhcp_relay_config(&dhcpRelay);
}

/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 keep or drop or replace policy or
| response validation configuration
| Responsibility: Enable dhcp-relay option 82 keep or drop or replace policy or
|                 response validation configuration
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_options_configuration,
      dhcp_relay_options_configuration_cmd,
      "dhcp-relay option 82 (keep|validate|drop|replace)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      KEEP_STR
      VALIDATE_STR
      DROP_STR
      REPLACE_STR)
{
    dhcp_relay dhcpRelay;
    memset(&dhcpRelay, 0, sizeof(dhcp_relay));

    dhcpRelay.option82Status = "true";
    if (!strcmp((char*)argv[0], "validate") == 0)
    {
        dhcpRelay.option82Policy = (char*)argv[0];
    }
    else
    {
        dhcpRelay.option82Policy = "replace";
        dhcpRelay.option82ValidationStatus = "true";
    }

    return dhcp_relay_config(&dhcpRelay);
}

/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 keep policy with ip or mac
| remote id configuration
| Responsibility: Enable dhcp-relay option 82 keep policy with ip or mac
|                 remote id configuration
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_keep_option_configuration,
      dhcp_relay_keep_option_configuration_cmd,
      "dhcp-relay option 82 keep (mac|ip)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      KEEP_STR
      MAC_STR
      OPTION_IP_STR)
{
    dhcp_relay dhcpRelay;
    memset(&dhcpRelay, 0, sizeof(dhcp_relay));

    dhcpRelay.option82Status = "true";
    dhcpRelay.option82Policy = "keep";
    dhcpRelay.option82ValidationStatus = "true";
    if(argv[0])
    {
        dhcpRelay.option82RemoteID = (char*)argv[0];
    }

    return dhcp_relay_config(&dhcpRelay);
}
/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 drop policy with response validation
| or ip or mac remote id configuration
| Responsibility: Enable dhcp-relay option 82 drop policy with response
|                 validation or ip or mac remote id
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_drop_option_configuration,
      dhcp_relay_drop_option_configuration_cmd,
      "dhcp-relay option 82 drop (validate|mac|ip)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      DROP_STR
      VALIDATE_STR
      MAC_STR
      OPTION_IP_STR)
{
    dhcp_relay dhcpRelay;
    memset(&dhcpRelay, 0, sizeof(dhcp_relay));

    dhcpRelay.option82Status = "true";
    dhcpRelay.option82Policy = "drop";

    if(argv[0])
    {
        if (strcmp((char*)argv[0], "validate") == 0)
        {
            dhcpRelay.option82ValidationStatus = "true";
        }
        else
        {
            dhcpRelay.option82RemoteID = (char*)argv[0];
        }
    }

    return dhcp_relay_config(&dhcpRelay);
}
/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 drop policy with response validation
| mac or ip remote id configuration
| Responsibility: Enable dhcp-relay option 82 with drop policy,
|                 response validation and mac or ip remote id
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_drop_validate_option_configuration,
      dhcp_relay_drop_validate_option_configuration_cmd,
      "dhcp-relay option 82 drop validate (mac|ip)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      DROP_STR
      VALIDATE_STR
      MAC_STR
      OPTION_IP_STR)
{
    dhcp_relay dhcpRelay;
    memset(&dhcpRelay, 0, sizeof(dhcp_relay));

    dhcpRelay.option82Status = "true";
    dhcpRelay.option82ValidationStatus = "true";
    dhcpRelay.option82Policy = "drop";

    if (argv[0])
    {
        dhcpRelay.option82RemoteID = (char*)argv[0];
    }
    else
    {
        dhcpRelay.option82RemoteID = "mac";
    }

    return dhcp_relay_config(&dhcpRelay);
}
/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 drop policy with mac remote id and
| response validation configuration
| Responsibility: Enable dhcp-relay option 82 with replace policy,
|                 mac remote id and response validation
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_drop_remote_id_mac_option_configuration,
      dhcp_relay_drop_remote_id_mac_option_configuration_cmd,
      "dhcp-relay option 82 drop mac (validate)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      DROP_STR
      MAC_STR
      VALIDATE_STR)
{
    dhcp_relay dhcpRelay;
    memset(&dhcpRelay, 0, sizeof(dhcp_relay));

    dhcpRelay.option82Status = "true";
    dhcpRelay.option82Policy = "drop";
    dhcpRelay.option82RemoteID = "mac";

    if (argv[0])
    {
        dhcpRelay.option82ValidationStatus = "true";
    }
    else
    {
        dhcpRelay.option82ValidationStatus = "false";
    }

    return dhcp_relay_config(&dhcpRelay);
}

/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 drop policy with ip remote id and
| response validation configuration
| Responsibility: Enable dhcp-relay option 82 with drop policy,
|                 ip remote id and response validation
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_drop_remote_id_ip_option_configuration,
      dhcp_relay_drop_remote_id_ip_option_configuration_cmd,
      "dhcp-relay option 82 drop ip (validate)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      DROP_STR
      OPTION_IP_STR
      VALIDATE_STR)
{
    dhcp_relay dhcpRelay;
    memset(&dhcpRelay, 0, sizeof(dhcp_relay));

    dhcpRelay.option82Status = "true";
    dhcpRelay.option82Policy = "drop";
    dhcpRelay.option82RemoteID = "ip";

    if (argv[0])
    {
        dhcpRelay.option82ValidationStatus = "true";
    }
    else
    {
        dhcpRelay.option82ValidationStatus = "false";
    }

    return dhcp_relay_config(&dhcpRelay);
}
/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 replace policy with response validation
| or ip or mac remote id configuration
| Responsibility: Enable dhcp-relay option 82 replace policy with response
|                 validation or ip or mac remote id
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_replace_option_configuration,
      dhcp_relay_replace_option_configuration_cmd,
      "dhcp-relay option 82 replace (validate|mac|ip)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      REPLACE_STR
      VALIDATE_STR
      MAC_STR
      OPTION_IP_STR)
{
    dhcp_relay dhcpRelay;
    memset(&dhcpRelay, 0, sizeof(dhcp_relay));

    dhcpRelay.option82Status = "true";
    dhcpRelay.option82Policy = "replace";

    if(argv[0])
    {
        if (strcmp((char*)argv[0], "validate") == 0)
        {
            dhcpRelay.option82ValidationStatus = "true";
        }
        else
        {
            dhcpRelay.option82RemoteID = (char*)argv[0];
        }
    }

    return dhcp_relay_config(&dhcpRelay);
}
/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 replace policy with response validation
| mac or ip remote id configuration
| Responsibility: Enable dhcp-relay option 82 with replace policy,
|                 response validation and mac or ip remote id
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_replace_validate_option_configuration,
      dhcp_relay_replace_validate_option_configuration_cmd,
      "dhcp-relay option 82 replace validate (mac|ip)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      REPLACE_STR
      VALIDATE_STR
      MAC_STR
      OPTION_IP_STR)
{
    dhcp_relay dhcpRelay;
    memset(&dhcpRelay, 0, sizeof(dhcp_relay));

    dhcpRelay.option82Status = "true";
    dhcpRelay.option82ValidationStatus = "true";
    dhcpRelay.option82Policy = "replace";

    if (argv[0])
    {
        dhcpRelay.option82RemoteID = (char*)argv[0];
    }

    return dhcp_relay_config(&dhcpRelay);
}
/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 replace policy with ip remote id and
| response validation configuration
| Responsibility: Enable dhcp-relay option 82 with replace policy,
|                 ip remote id and response validation
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_replace_remote_id_mac_option_configuration,
      dhcp_relay_replace_remote_id_mac_option_configuration_cmd,
      "dhcp-relay option 82 replace mac (validate)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      REPLACE_STR
      MAC_STR
      VALIDATE_STR)
{
    dhcp_relay dhcpRelay;
    memset(&dhcpRelay, 0, sizeof(dhcp_relay));

    dhcpRelay.option82Status = "true";
    dhcpRelay.option82Policy = "replace";
    dhcpRelay.option82RemoteID = "mac";

    if (argv[0])
    {
        dhcpRelay.option82ValidationStatus = "true";
    }

    return dhcp_relay_config(&dhcpRelay);
}

/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 replace policy with ip remote id and
| response validation configuration
| Responsibility: Enable dhcp-relay option 82 with replace policy,
|                 ip remote id and response validation
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_replace_remote_id_ip_option_configuration,
      dhcp_relay_replace_remote_id_ip_option_configuration_cmd,
      "dhcp-relay option 82 replace ip (validate)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      REPLACE_STR
      OPTION_IP_STR
      VALIDATE_STR)
{
    dhcp_relay dhcpRelay;
    memset(&dhcpRelay, 0, sizeof(dhcp_relay));

    dhcpRelay.option82Status = "true";
    dhcpRelay.option82Policy = "replace";
    dhcpRelay.option82RemoteID = "ip";

    if (argv[0])
    {
        dhcpRelay.option82ValidationStatus = "true";
    }

    return dhcp_relay_config(&dhcpRelay);
}
/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 response validation with drop
| or replace policy configuration
| Responsibility: Enable dhcp-relay option 82 validation with
|                 drop or replace policy
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_validate_policy_option_configuration,
      dhcp_relay_validate_policy_option_configuration_cmd,
      "dhcp-relay option 82 validate (drop|replace)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      VALIDATE_STR
      DROP_STR
      REPLACE_STR)
{
    dhcp_relay dhcpRelay;
    memset(&dhcpRelay, 0, sizeof(dhcp_relay));

    dhcpRelay.option82Status = "true";
    dhcpRelay.option82ValidationStatus = "true";
    if (argv[0])
    {
        dhcpRelay.option82Policy  = (char*)argv[0];
    }

    return dhcp_relay_config(&dhcpRelay);
}

/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 response validation with replace policy
| and mac or ip remote ID configuration
| Responsibility: Enable dhcp-relay option 82 validation with
|                 replace policy and mac or ip remote ID
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_validate_drop_option_configuration,
      dhcp_relay_validate_drop_option_configuration_cmd,
      "dhcp-relay option 82 validate drop (mac|ip)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      VALIDATE_STR
      DROP_STR
      MAC_STR
      OPTION_IP_STR)
{
    dhcp_relay dhcpRelay;
    memset(&dhcpRelay, 0, sizeof(dhcp_relay));

    dhcpRelay.option82Status = "true";
    dhcpRelay.option82ValidationStatus = "true";
    dhcpRelay.option82Policy = "drop";
    if (argv[0])
    {
        dhcpRelay.option82RemoteID = (char*)argv[0];
    }

    return dhcp_relay_config(&dhcpRelay);
}
/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 response validation with replace policy
| and mac or ip remote ID configuration
| Responsibility: Enable dhcp-relay option 82 validation with
|                 replace policy and mac or ip remote ID
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_validate_replace_option_configuration,
      dhcp_relay_validate_replace_option_configuration_cmd,
      "dhcp-relay option 82 validate replace (mac|ip)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      VALIDATE_STR
      REPLACE_STR
      MAC_STR
      OPTION_IP_STR)
{
    dhcp_relay dhcpRelay;
    memset(&dhcpRelay, 0, sizeof(dhcp_relay));

    dhcpRelay.option82Status = "true";
    dhcpRelay.option82ValidationStatus = "true";
    dhcpRelay.option82Policy = "replace";

    if (argv[0])
    {
        dhcpRelay.option82RemoteID = (char*)argv[0];
    }

    return dhcp_relay_config(&dhcpRelay);
}
/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 and dhcp-relay option 82 validation
| unconfiguration
| Responsibility: Disable dhcp-relay or hop-count-increment
-----------------------------------------------------------------------------*/
DEFUN(no_dhcp_relay_option_configuration,
      no_dhcp_relay_option_configuration_cmd,
      "no dhcp-relay option 82 {validate}",
      NO_STR
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      VALIDATE_STR)
{
    dhcp_relay dhcpRelay;
    memset(&dhcpRelay, 0, sizeof(dhcp_relay));

    if (argv[0])
    {
        dhcpRelay.option82ValidationStatus = "false";
    }
    else
    {
        dhcpRelay.option82Policy = "replace";
        dhcpRelay.option82RemoteID = "mac";
        dhcpRelay.option82Status = "false";
        dhcpRelay.option82ValidationStatus = "false";
    }

    return dhcp_relay_config(&dhcpRelay);
}
/*-----------------------------------------------------------------------------
| Defun for dhcp-relay and hop-count-increment unconfiguration
| Responsibility: Disable dhcp-relay or hop-count-increment
-----------------------------------------------------------------------------*/
DEFUN(no_dhcp_relay_configuration,
      no_dhcp_relay_configuration_cmd,
      "no dhcp-relay {hop-count-increment}",
      NO_STR
      DHCP_RELAY_STR
      HOP_COUNT_INCREMENT_STR)
{
    dhcp_relay dhcpRelay;
    memset(&dhcpRelay, 0, sizeof(dhcp_relay));

    if (argv[0])
    {
        dhcpRelay.hopCountIncrement = "true";
    }
    else
    {
        dhcpRelay.status = "true";
    }

    return dhcp_relay_config(&dhcpRelay);
}


/*-----------------------------------------------------------------------------
| Defun for show dhcp-relay
| Responsibility: Displays the dhcp-relay configurations
-----------------------------------------------------------------------------*/
DEFUN(show_dhcp_relay_configuration,
      show_dhcp_relay_configuration_cmd,
      "show dhcp-relay",
      SHOW_STR
      SHOW_DHCP_RELAY_STR)
{
     return show_dhcp_relay_config();
}

/*-----------------------------------------------------------------------------
| Defun ip helper-address configuration
| Responsibility: Set a helper-addresses for a dhcp-relay
-----------------------------------------------------------------------------*/
DEFUN(ip_helper_address_configuration,
      ip_helper_address_configuration_cmd,
      "ip helper-address A.B.C.D ",
      IP_STR
      HELPER_ADDRESS_STR
      HELPER_ADDRESS_INPUT_STR)
{
    udpfwd_server udpfwdServer;
    memset(&udpfwdServer, 0, sizeof(udpfwd_server));

    /* Validate the input parameters. */
    if (decode_server_param(&udpfwdServer, argv, DHCP_RELAY))
    {
        return udpfwd_helperaddressconfig(&udpfwdServer, SET);
    }
    else
    {
        return CMD_SUCCESS;
    }

}

/*-----------------------------------------------------------------------------
| Defun ip helper-address unconfiguration
| Responsibility: Unset a helper-addresses for a dhcp-relay
-----------------------------------------------------------------------------*/
DEFUN(no_ip_helper_address_configuration,
      no_ip_helper_address_configuration_cmd,
      "no ip helper-address A.B.C.D ",
      NO_STR
      IP_STR
      HELPER_ADDRESS_STR
      HELPER_ADDRESS_INPUT_STR)
{
    udpfwd_server udpfwdServer;
    memset(&udpfwdServer, 0, sizeof(udpfwd_server));

    /* Validate the input parameters. */
    if (decode_server_param(&udpfwdServer, argv, DHCP_RELAY))
    {
        return udpfwd_helperaddressconfig(&udpfwdServer, UNSET);
    }
    else
    {
        return CMD_SUCCESS;
    }
}

/*-----------------------------------------------------------------------------
| Defun for show ip helper-address
| Responsibility: Displays the helper-addresses of a dhcp-relay
-----------------------------------------------------------------------------*/
DEFUN(show_ip_helper_address_configuration,
      show_ip_helper_address_configuration_cmd,
      "show ip helper-address {interface (IFNAME | A.B )} ",
      SHOW_STR
      IP_STR
      SHOW_HELPER_ADDRESS_STR
      INTERFACE_STR
      IFNAME_STR
      SUBIFNAME_STR)
{
    return show_ip_helper_address_config(argv[0]);
}

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

int32_t
show_dhcp_relay_config (void);
int8_t
dhcp_relay_config (uint16_t *, FEATURE_CONFIG *);
void
update_feature_key(const char *, FEATURE_STATUS,
                        struct smap *, UDPFWD_FEATURE);
void
update_option_82_policy(char *, DHCP_RELAY_OPTION82_POLICY , struct smap *);
void
update_option_82_remote_id(char *, DHCP_RELAY_OPTION82_REMOTE_ID,
                           struct smap *);

void set_feature_status(uint16_t *, UDPFWD_FEATURE,
                        FEATURE_STATUS);
FEATURE_STATUS get_feature_status(uint16_t, UDPFWD_FEATURE);

extern struct ovsdb_idl *idl;
VLOG_DEFINE_THIS_MODULE(dhcp_relay_vty);

/*-----------------------------------------------------------------------------
| Function       : show_dhcp_relay_config
| Responsibility : To show the dhcp-relay configuration.
| Return         : Returns CMD_SUCCESS
-----------------------------------------------------------------------------*/
int32_t
show_dhcp_relay_config (void)
{
    const struct ovsrec_system *ovs_row = NULL;
    char *status = NULL;

    ovs_row = ovsrec_system_first(idl);
    if (ovs_row == NULL)
    {
        VLOG_ERR("%s SYSTEM table did not have any rows.%s",
                 VTY_NEWLINE, VTY_NEWLINE);
        return CMD_SUCCESS;
    }

    /* Display the dhcp-relay global configuration */
    vty_out (vty, "\n DHCP Relay Agent                 : ");
    status = (char *)smap_get(&ovs_row->dhcp_config,
                             SYSTEM_DHCP_CONFIG_MAP_V4RELAY_DISABLED);

    if (!status)
    {
        status = "enabled";
    }
    else
    {
        if (!strcmp(status, "false"))
            status = "enabled";
        else
            status = "disabled";
    }

    if (status)
    {
        vty_out(vty, "%s%s", status, VTY_NEWLINE);
    }

    /* Display the dhcp-relay hop-count increment configuration */
    status = NULL;
    vty_out (vty, " DHCP Request Hop Count Increment : ");
    status = (char *)smap_get(&ovs_row->dhcp_config,
                SYSTEM_DHCP_CONFIG_MAP_V4RELAY_HOP_COUNT_INCREMENT_DISABLED);

    if (!status)
    {
        status = "enabled";
    }
    else
    {
        if (!strcmp(status, "false"))
            status = "enabled";
        else
            status = "disabled";
    }

    if (status)
    {
        vty_out(vty, "%s%s", status, VTY_NEWLINE);
    }

    /* Display the dhcp-relay option 82 configuration */
    status = NULL;
    vty_out (vty, " Option 82                        : ");
    status = (char *)smap_get(&ovs_row->dhcp_config,
                SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_ENABLED);
    if (!status)
    {
        status = "disabled";
    }
    else
    {
        if (!strcmp(status, "true"))
            status = "enabled";
        else
            status = "disabled";
    }

    if (status)
    {
        vty_out(vty, "%s%s", status, VTY_NEWLINE);
    }

    /* Display the dhcp-relay option 82 response validation configuration */
    status = NULL;
    vty_out (vty, " Response validation              : ");
    status = (char *)smap_get(&ovs_row->dhcp_config,
                SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_VALIDATION_ENABLED);
    if (!status)
    {
        status = "disabled";
    }
    else
    {
        if (!strcmp(status, "true"))
            status = "enabled";
        else
            status = "disabled";
    }

    if (status)
    {
        vty_out(vty, "%s%s", status, VTY_NEWLINE);
    }

    /* Display the dhcp-relay option 82 policy configuration */
    status = NULL;
    vty_out (vty, " Option 82 handle policy          : ");
    status = (char *)smap_get(&ovs_row->dhcp_config,
                SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_POLICY);
    if (!status)
    {
        status = "replace";
    }

    if (status)
    {
        vty_out(vty, "%s%s", status, VTY_NEWLINE);
    }

    /* Display the dhcp-relay option 82 remote ID configuration */
    status = NULL;
    vty_out (vty, " Remote ID                        : ");
    status = (char *)smap_get(&ovs_row->dhcp_config,
                SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_REMOTE_ID);
    if (!status)
    {
        status = "mac";
    }

    if (status)
    {
        vty_out(vty, "%s%s", status, VTY_NEWLINE);
    }
    return CMD_SUCCESS;
}

/*-----------------------------------------------------------------------------
| Function          : update_feature_key
| Responsibility    : Update the configuration status of a feature
| Parameters        :
|      value        : Pointer to configuration bit map
|      status       : Config status of the feature
| smap_status_value : Pointer to smap structure
|      feature      : feature enum value
| Return            : None
-----------------------------------------------------------------------------*/
void update_feature_key(const char *value, FEATURE_STATUS status,
                        struct smap *smap_status_value,
                        UDPFWD_FEATURE feature)
{
    switch(feature)
    {
    case DHCP_RELAY:
        if (ENABLE == status)
            smap_replace(smap_status_value, value, "true");
        else
            smap_replace(smap_status_value, value, "false");
    break;

    case DHCP_RELAY_HOP_COUNT_INCREMENT:
        if (ENABLE == status)
            smap_replace(smap_status_value, value, "true");
        else
            smap_replace(smap_status_value, value, "false");
    break;

    case DHCP_RELAY_OPTION82:
        if (ENABLE == status)
            smap_replace(smap_status_value, value, "true");
        else
            smap_replace(smap_status_value, value, "false");
    break;

    case DHCP_RELAY_OPTION82_VALIDATE:
        if (ENABLE == status)
            smap_replace(smap_status_value, value, "true");
        else
            smap_replace(smap_status_value, value, "false");
    break;

    default:
    break;
    }
    return;
}

/*-----------------------------------------------------------------------------
| Function          : update_option_82_policy
| Responsibility    : Update the forward policy of option 82
| Parameters        :
|      value        : Pointer to option 82 forward policy
|      rid          : Policy enum value
| smap_status_value : Pointer to smap structure
| Return            : None
-----------------------------------------------------------------------------*/
void update_option_82_policy(char *value, DHCP_RELAY_OPTION82_POLICY policy,
                             struct smap *smap_status_value)
{
    switch(policy)
    {
    case KEEP:
        smap_replace(smap_status_value, value, policy_name[KEEP]);
    break;

    case DROP:
        smap_replace(smap_status_value, value, policy_name[DROP]);
    break;

    case REPLACE:
        smap_replace(smap_status_value, value, policy_name[REPLACE]);
    break;

    default:
    break;
    }
    return;
}

/*-----------------------------------------------------------------------------
| Function          : update_option_82_remote_id
| Responsibility    : Update the remote ID of option 82
| Parameters        :
|      value        : Pointer to option 82 remote ID key
|      rid          : remote id enum value
| smap_status_value : Pointer to smap structure
| Return            : None
-----------------------------------------------------------------------------*/
void update_option_82_remote_id(char *value,
                                DHCP_RELAY_OPTION82_REMOTE_ID rid,
                                struct smap *smap_status_value)
{
    switch(rid)
    {
    case REMOTE_ID_IP:
        smap_replace(smap_status_value, value, remote_id_name[REMOTE_ID_IP]);
    break;

    case REMOTE_ID_MAC:
        smap_replace(smap_status_value, value, remote_id_name[REMOTE_ID_MAC]);
    break;

    default:
    break;
    }
    return;
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
dhcp_relay_config (uint16_t *update_config, FEATURE_CONFIG *config_value)
{
    const struct ovsrec_system *ovs_row = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    struct smap smap_status_value;
    enum ovsdb_idl_txn_status txn_status;
    FEATURE_STATUS state;

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

    smap_clone(&smap_status_value, &ovs_row->dhcp_config);

     /* Check if dhcp-relay configuration needs to be udpated. */
    if (ENABLE == get_feature_status(*update_config, DHCP_RELAY)) {
        state = get_feature_status(config_value->config, DHCP_RELAY);
        update_feature_key(SYSTEM_DHCP_CONFIG_MAP_V4RELAY_DISABLED,
                           (state == ENABLE) ? DISABLE : ENABLE,
                           &smap_status_value, DHCP_RELAY);
        ovsrec_system_set_dhcp_config(ovs_row, &smap_status_value);
    }

    /*
     * Check if dhcp-relay hop-count increment configuration
     * needs to be udpated.
     */
    if (ENABLE == get_feature_status(*update_config, DHCP_RELAY_HOP_COUNT_INCREMENT)) {
        state = get_feature_status(config_value->config,
                                   DHCP_RELAY_HOP_COUNT_INCREMENT);
        update_feature_key(
               SYSTEM_DHCP_CONFIG_MAP_V4RELAY_HOP_COUNT_INCREMENT_DISABLED,
               (state == ENABLE) ? DISABLE : ENABLE, &smap_status_value,
               DHCP_RELAY_HOP_COUNT_INCREMENT);
        ovsrec_system_set_dhcp_config(ovs_row, &smap_status_value);
    }

    /* Check if dhcp-relay option 82 configuration needs to be udpated. */
    if (ENABLE == get_feature_status(*update_config, DHCP_RELAY_OPTION82)) {
        state = get_feature_status(config_value->config, DHCP_RELAY_OPTION82);
        update_feature_key(SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_ENABLED,
                           state, &smap_status_value,
                           DHCP_RELAY_HOP_COUNT_INCREMENT);
        ovsrec_system_set_dhcp_config(ovs_row, &smap_status_value);
    }

    /*
     * Check if dhcp-relay option 82 response validation configuration
     * needs to be udpated.
     */
    if (ENABLE == get_feature_status(*update_config, DHCP_RELAY_OPTION82_VALIDATE)) {
        state = get_feature_status(config_value->config,
                                   DHCP_RELAY_OPTION82_VALIDATE);
        update_feature_key(
               SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_VALIDATION_ENABLED,
                           state, &smap_status_value,
                           DHCP_RELAY_OPTION82_VALIDATE);
        ovsrec_system_set_dhcp_config(ovs_row, &smap_status_value);
    }

    /*
     * Check if dhcp-relay option 82 forward policy configuration
     * needs to be udpated.
     */
    if (config_value->policy < INVALID) {
        update_option_82_policy(
               SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_POLICY,
                           config_value->policy, &smap_status_value);
        ovsrec_system_set_dhcp_config(ovs_row, &smap_status_value);
    }
    /*
     * Check if dhcp-relay option 82 remote ID configuration
     * needs to be udpated.
     */
    if (config_value->r_id < REMOTE_ID_INVALID) {
        update_option_82_remote_id(
               SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_REMOTE_ID,
                           config_value->r_id, &smap_status_value);
        ovsrec_system_set_dhcp_config(ovs_row, &smap_status_value);
    }

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
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;
    config_value.policy = INVALID;
    config_value.r_id = REMOTE_ID_INVALID;

    if (argv[0])
    {
        /* Mark dhcp-relay hop count increment feature to be udpated */
        set_feature_status(&update_features,
                           DHCP_RELAY_HOP_COUNT_INCREMENT, ENABLE);

        /* Set new configuration value */
        set_feature_status(&(config_value.config),
                           DHCP_RELAY_HOP_COUNT_INCREMENT, ENABLE);
    }
    else
    {
        /* Mark dhcp-relay feature to be udpated */
        set_feature_status(&update_features, DHCP_RELAY, ENABLE);

        /* Set new configuration value */
        set_feature_status(&(config_value.config), DHCP_RELAY, ENABLE);
    }

    return dhcp_relay_config(&update_features, &config_value);
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
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;
    config_value.r_id = REMOTE_ID_INVALID;

    /* Mark dhcp-relay option 82 feature to be udpated */
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);

    /* Set new configuration value */
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);

    if (!strcmp((char*)argv[0], "validate"))
    {
       /* Mark dhcp-relay option 82 validation to be udpated */
       set_feature_status(&update_features,
                          DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

       /* Set new configuration values */
       set_feature_status(&config_value.config,
                          DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
       config_value.policy = INVALID;
    }
    else
    {
        /* Set new configuration value */
        if (!strcmp((char*)argv[0], "keep"))
            config_value.policy = KEEP;
        else if(!strcmp((char*)argv[0], "replace"))
            config_value.policy = REPLACE;
        else
            config_value.policy = DROP;
    }

    return dhcp_relay_config(&update_features, &config_value);
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
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;
    /* Mark dhcp-relay option 82 and validation feature to be udpated */
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&update_features,
                          DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&config_value.config,
                          DHCP_RELAY_OPTION82_VALIDATE, DISABLE);
    config_value.policy = KEEP;
    if(argv[0])
    {
        if (!strcmp((char*)argv[0], "ip"))
            config_value.r_id = REMOTE_ID_IP;
        else
            config_value.r_id = REMOTE_ID_MAC;
    }
    else
    {
        config_value.r_id = REMOTE_ID_INVALID;
    }

    return dhcp_relay_config(&update_features, &config_value);
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
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;
    /* Mark dhcp-relay option 82 feature to be udpated */
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    config_value.policy = DROP;

    if(argv[0])
    {
        if (!strcmp((char*)argv[0], "validate"))
        {
            /* Mark dhcp-relay option 82 feature to be udpated */
            set_feature_status(&update_features,
                               DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

            /* Set new configuration values */
            set_feature_status(&config_value.config,
                               DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
            config_value.r_id = REMOTE_ID_INVALID;
        }
        else
        {
            /* Set new configuration values */
            if (!strcmp((char*)argv[0], "ip"))
                config_value.r_id = REMOTE_ID_IP;
            else
                config_value.r_id = REMOTE_ID_MAC;
        }
    }

    return dhcp_relay_config(&update_features, &config_value);
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
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */

    FEATURE_CONFIG config_value;
    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;

    /* Mark dhcp-relay option 82 and validation features to be udpated */
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&update_features,
                       DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&config_value.config,
                       DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
    config_value.policy = DROP;

    if (argv[0])
    {
        if (!strcmp((char*)argv[0], "ip"))
            config_value.r_id = REMOTE_ID_IP;
        else
            config_value.r_id = REMOTE_ID_MAC;
    }
    else
    {
        config_value.r_id = REMOTE_ID_INVALID;
    }

    return dhcp_relay_config(&update_features, &config_value);
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
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;

    /* Mark dhcp-relay option 82 feature to be udpated */
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    config_value.policy = DROP;
    config_value.r_id = REMOTE_ID_MAC;

    if (argv[0])
    {
        /* Mark dhcp-relay option 82 feature to be udpated */
        set_feature_status(&update_features,
                           DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

        /* Set new configuration value */
        set_feature_status(&config_value.config,
                           DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
    }

    return dhcp_relay_config(&update_features, &config_value);
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
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;

    /* Mark dhcp-relay option 82 feature to be udpated */
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    config_value.policy = DROP;
    config_value.r_id = REMOTE_ID_IP;

    if (argv[0])
    {
        /* Mark dhcp-relay option 82 feature to be udpated */
        set_feature_status(&update_features,
                           DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

        /* Set new configuration value */
        set_feature_status(&config_value.config,
                           DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
    }

    return dhcp_relay_config(&update_features, &config_value);
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
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;
    /* Mark dhcp-relay option 82 feature to be udpated */
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    config_value.policy = REPLACE;

    if(argv[0])
    {
        if (!strcmp((char*)argv[0], "validate"))
        {
            /* Mark dhcp-relay option 82 validation feature to be udpated */
            set_feature_status(&update_features,
                               DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

            /* Set new configuration values */
            set_feature_status(&config_value.config,
                               DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
            config_value.r_id = REMOTE_ID_INVALID;
        }
        else
        {
            /* Set new configuration values */
            if (!strcmp((char*)argv[0], "ip"))
                config_value.r_id = REMOTE_ID_IP;
            else
                config_value.r_id = REMOTE_ID_MAC;
        }
    }

    return dhcp_relay_config(&update_features, &config_value);
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
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;
    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;

    /* Mark dhcp-relay option 82 and validation features to be udpated */
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&update_features,
                       DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&config_value.config,
                       DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
    config_value.policy = REPLACE;

    if (argv[0])
    {
        if (!strcmp((char*)argv[0], "ip"))
            config_value.r_id = REMOTE_ID_IP;
        else
            config_value.r_id = REMOTE_ID_MAC;
    }
    else
    {
        config_value.r_id = REMOTE_ID_INVALID;
    }

    return dhcp_relay_config(&update_features, &config_value);
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
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;

    /* Mark dhcp-relay option 82 feature to be udpated */
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    config_value.policy = REPLACE;
    config_value.r_id = REMOTE_ID_MAC;

    if (argv[0])
    {
        /* Mark dhcp-relay option 82 feature to be udpated */
        set_feature_status(&update_features,
                           DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

        /* Set new configuration value */
        set_feature_status(&config_value.config,
                           DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
    }

    return dhcp_relay_config(&update_features, &config_value);
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
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;

    /* Mark dhcp-relay option 82 feature to be udpated */
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    config_value.policy = REPLACE;
    config_value.r_id = REMOTE_ID_IP;

    if (argv[0])
    {
        /* Mark dhcp-relay option 82 validation feature to be udpated */
        set_feature_status(&update_features,
                           DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

        /* Set new configuration value */
        set_feature_status(&config_value.config,
                           DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
    }

    return dhcp_relay_config(&update_features, &config_value);
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
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;
    config_value.r_id = REMOTE_ID_INVALID;

    /* Mark dhcp-relay option 82 and validation features to be udpated */
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&update_features,
                       DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&config_value.config,
                       DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
    if (argv[0])
    {
        if(!strcmp((char*)argv[0], "replace"))
            config_value.policy = REPLACE;
        else
            config_value.policy = DROP;
    }

    return dhcp_relay_config(&update_features, &config_value);
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
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;

    /* Mark dhcp-relay option 82 and validation features to be udpated */
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&update_features,
                       DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&config_value.config,
                       DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
    config_value.policy = DROP;
    if (argv[0])
    {
        if (!strcmp((char*)argv[0], "ip"))
            config_value.r_id = REMOTE_ID_IP;
        else
            config_value.r_id = REMOTE_ID_MAC;
    }
    else
    {
        config_value.r_id = REMOTE_ID_INVALID;
    }

    return dhcp_relay_config(&update_features, &config_value);
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
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;

    /* Mark dhcp-relay option 82 and validation features to be udpated */
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&update_features,
                       DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&config_value.config,
                       DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
    config_value.policy = DROP;
    if (argv[0])
    {
        if (!strcmp((char*)argv[0], "ip"))
            config_value.r_id = REMOTE_ID_IP;
        else
            config_value.r_id = REMOTE_ID_MAC;
    }
    else
    {
        config_value.r_id = REMOTE_ID_INVALID;
    }

    return dhcp_relay_config(&update_features, &config_value);
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
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;
    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;

    if (argv[0])
    {
        /* Mark dhcp-relay option 82 feature to be udpated */
        set_feature_status(&update_features,
                           DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

        /* Set new configuration values */
        set_feature_status(&config_value.config,
                           DHCP_RELAY_OPTION82_VALIDATE, DISABLE);
        config_value.r_id = REMOTE_ID_INVALID;
        config_value.policy = INVALID;
    }
    else
    {
        /* Mark dhcp-relay option 82 and validation features to be udpated */
        set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);
        set_feature_status(&update_features, DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

        /* Set new configuration values */
        set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, DISABLE);
        set_feature_status(&config_value.config, DHCP_RELAY_OPTION82_VALIDATE, DISABLE);
        config_value.r_id = REMOTE_ID_MAC;
        config_value.policy = REPLACE;
    }

    return dhcp_relay_config(&update_features, &config_value);
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
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;
    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;
    config_value.r_id = REMOTE_ID_INVALID;
    config_value.policy = INVALID;

    if (argv[0])
    {
        /* Mark dhcp-relay hop count increment feature to be udpated */
        set_feature_status(&update_features,
                           DHCP_RELAY_HOP_COUNT_INCREMENT, ENABLE);

        /* Set new configuration value */
        set_feature_status(&(config_value.config),
                           DHCP_RELAY_HOP_COUNT_INCREMENT, DISABLE);
    }
    else
    {
        /* Mark dhcp-relay feature to be udpated */
        set_feature_status(&update_features, DHCP_RELAY, ENABLE);

        /* Set new configuration value */
        set_feature_status(&(config_value.config), DHCP_RELAY, DISABLE);
    }

    return dhcp_relay_config(&update_features, &config_value);
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

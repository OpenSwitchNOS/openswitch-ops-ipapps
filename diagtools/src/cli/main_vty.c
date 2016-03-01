/* IP Applications CLI commands
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
 * File: main_vty.c
 *
 * Purpose: To add IP Applications specific CLI commands.
 */

#include <stdint.h>
#include "vtysh/command.h"
#include "ping.h"
#include "traceroute.h"

void cli_pre_init(void)
{
}

void cli_post_init(void)
{
    /* ping CLI commands */
    install_element (ENABLE_NODE, &cli_ping_cmd);
    install_element (ENABLE_NODE, &cli_ping6_cmd);

    /* Traceroute CLI commands */
    install_element (ENABLE_NODE, &cli_traceroute_cmd);
    install_element (ENABLE_NODE, &cli_traceroute6_cmd);
    install_element (ENABLE_NODE, &cli_traceroute_ipoption_cmd);
}

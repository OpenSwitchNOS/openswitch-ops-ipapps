/* UDP Forwarder common header file
 *
 * Copyright (C) 2016 Hewlett Packard Enterprise Development LP
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * File: udpfwd_common.h
 *
 * Purpose:  To add common declarations for UDP Forwarder
 */

#ifndef UDPFWD_COMMON_H
#define UDPFWD_COMMON_H

/* IP Address string maximum length(Ex : 255.255.255.255). */
#define IPADDRESS_STRING_MAX_LENGTH   15

/* Maximum entries allowed on an interface. */
#define MAX_HELPER_ADDRESSES_PER_INTERFACE   8

#endif /* udpfwd_common.h */
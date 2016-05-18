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
 * File: nwdiag.c
 *
 * Purpose : To enable non root users to execute command in a specific namespace.
 */

#define _GNU_SOURCE
#include <fcntl.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "nl-utils.h"

#define PING              "ping"
#define PING6             "ping6"
#define TRACEROUTE        "traceroute"
#define TRACEROUTE6       "traceroute6"
#define BUFFSIZE          1024
#define NSPATHSIZE        280
#define NAMESPACELENGTH   256

int main(int argc, char *argv[])
{
    int len = 0;
    FILE *fp = NULL;
    /* Assuming max namespace name size as 256 */
    char buffer[BUFFSIZE];
    char *target = buffer;

    /* Arguments to be passed in below order.
     * argv[1] = namespace name.
     * argv[2] = command to execute.
     * argv[3] = parameters required by command.
     */
    if (argc < 3) {
        printf("Invalid number of arguments specified\n");
        exit(0);
    }

    if (strncmp("mgmt", (char*)argv[1], strlen("mgmt")) == 0)
    {
        if (nl_setns_oobm() == -1)
        {
            printf("Internal error, failed to enter mgmt OOBM namespace\n");
            exit(0);
        }
    }
    else
    {
        if (strlen((char*)argv[1]) > NAMESPACELENGTH)
        {
            printf("Internal error, invalid vrf\n");
            exit(0);
        }

        if (nl_setns_with_name((char*)argv[1]) == -1)
        {
            printf("Internal error, vrf not found\n");
            exit(0);
        }
    }

    /* Change to current user */
    setuid(getuid());

    if ((strncmp(PING, argv[2], strlen(argv[2])) == 0))
    {
        len += snprintf(target+len, BUFFSIZE, "%s ", PING);
    }
    else if ((strncmp(PING6, argv[2], strlen(argv[2])) == 0))
    {
        len += snprintf(target+len, BUFFSIZE, "%s ", PING6);
    }
    else if ((strncmp(TRACEROUTE, argv[2], strlen(argv[2])) == 0))
    {
        len += snprintf(target+len, BUFFSIZE, "%s ", TRACEROUTE);
    }
    else if ((strncmp(TRACEROUTE6, argv[2], strlen(argv[2])) == 0))
    {
        len += snprintf(target+len, BUFFSIZE, "%s ", TRACEROUTE6);
    }
    else
    {
        printf("Internal error: unsupported operation\n");
        exit (0);
    }

    if (strlen((char*)argv[3]) < (BUFFSIZE-len))
    {
        len += snprintf(target+len, BUFFSIZE-len, "%s ", argv[3]);
    }
    else
    {
        printf("Internal error: buffer overflow\n");
        exit (0);
    }
    fp = popen(buffer, "w");
    if (fp)
    {
        while (fgets(buffer, BUFFSIZE, fp) != NULL)
            printf("%s", buffer);
    }
    else
        printf("Internal error");

    pclose(fp);

}

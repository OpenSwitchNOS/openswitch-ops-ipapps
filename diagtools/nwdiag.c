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

int main(int argc, char *argv[])
{
    int fd = -1, len = 0;
    FILE *fp = NULL;
    /* Assuming max namespace name size as 256 */
    char ns_path[280];
    char buffer[1024];
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

    if (strncmp("mgmt", (char*)argv[1], strlen("mgmt")) != 0)
    {
        sprintf(ns_path, "/var/run/netns/");
        sprintf(ns_path+strlen(ns_path), "%s", argv[1]);
        fd = open(ns_path, O_RDONLY);  /* Get descriptor for namespace */
        if (fd == -1)
        {
            printf("ping: Internal error, vrf not found\n");
            exit(0);
        }

        if (setns(fd, CLONE_NEWNET) == -1) /* Join that namespace */
        {
            printf("Internal error");
            close(fd);
            exit(0);
        }
    }

    close(fd);
    /* Change to current user */
    setuid(getuid());

    if ((strncmp("ping", argv[2], strlen("ping")) == 0) ||
        (strncmp("ping6", argv[2], strlen("ping6")) == 0) ||
        (strncmp("traceroute", argv[2], strlen("traceroute")) == 0) ||
        (strncmp("traceroute6", argv[2], strlen("traceroute6")) == 0))
    {
        len += sprintf(target+len, "%s ", argv[2]);
        len += sprintf(target+len, "%s ", argv[3]);
        fp = popen(buffer, "w");
        if (fp)
        {
            while (fgets(buffer, 1024, fp) != NULL)
                printf("%s", buffer);
        }
        else
            printf("Internal error");

        pclose(fp);
    }
    else
        printf("Unsupported operation\n");
}

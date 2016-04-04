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
 * File: nscmdexec.c
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

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

int main(int argc, char *argv[])
{
    int fd = -1, len = 0;
    FILE *fp = NULL;
    char output[1024], buffer[1024];
    char *target = buffer;

    if (argc < 3) {
        printf("%s /proc/PID/ns/FILE cmd args...\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fd = open(argv[1], O_RDONLY);  /* Get descriptor for namespace */
    if (fd == -1)
        errExit("open");

    if (setns(fd, CLONE_NEWNET) == -1) /* Join that namespace */
        errExit("setns");

    /* Change to admin user */
    setuid(getuid());

    if ((strcmp("ping", argv[2]) == 0) ||
        (strcmp("ping6", argv[2]) == 0) ||
        (strcmp("traceroute", argv[2]) == 0) ||
        (strcmp("traceroute6", argv[2]) == 0))
    {
        len += sprintf(target+len, "%s ", argv[2]);
        len += sprintf(target+len, "%s ", argv[3]);
        fp = popen(buffer, "w");
        if (fp)
        {
            while (fgets(output, 1024, fp) != NULL)
                printf("%s", output);
        }
        pclose(fp);
    }
    else
        printf("supported only for ipapps\n");
}

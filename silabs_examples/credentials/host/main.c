/***************************************************************************/ /**
* @file main.c
* @brief main() function.
*******************************************************************************
* # License
* <b>Copyright 2022 Silicon Laboratories Inc.
*www.silabs.com</b>
*******************************************************************************
*
* The licensor of this software is Silicon
*Laboratories Inc. Your use of this software is
*governed by the terms of Silicon Labs Master
*Software License Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement.
*This software is distributed to you in Source Code
*format and is governed by the sections of the MSLA
*applicable to Source Code.
*
******************************************************************************/

#include "host_creds.h"
#include <getopt.h>
#include <stdio.h>
#include <string.h>

#define OPTION_LENGTH_MAX 128
static const char options[] = "p:f:N:P:V:";

int main(int argc, char * argv[])
{
    int c;
    int option_count                    = 0;
    char port[OPTION_LENGTH_MAX]         = { 0 };
    char filename[OPTION_LENGTH_MAX] = { 0 };
    char common_name[OPTION_LENGTH_MAX] = { 0 };
    uint32_t vendor_id = 0;
    uint32_t product_id = 0;
    size_t length                      = 0;

    if (!argv)
    {
        printf("Error: argv[] is NULL\n");
        return -1;
    }

     while ((c = getopt(argc, argv, options)) != -1 && optarg)
    {
        option_count++;

        switch (c)
        {
        case 'p':
            if (!strncpy(port, optarg, sizeof(port)))
            {
                printf("Invalid port '%s'\n", optarg ? optarg : "NULL");
            }
            break;

        case 'f':
            if (!strncpy(filename, optarg, sizeof(filename)))
            {
                printf("Invalid CSR '%s'\n", optarg ? optarg : "NULL");
            }
            break;

        case 'N':
            if (!strncpy(common_name, optarg, sizeof(common_name)))
            {
                printf("Invalid Common Name '%s'\n", optarg ? optarg : "NULL");
            }
            break;

        case 'V':
            if (sscanf(optarg, "%u", &vendor_id) != 1)
            {
                printf("Invalid vendor_id %s.\n", optarg ? optarg : "NULL");
            }
            break;

        case 'P':
            if (sscanf(optarg, "%u", &product_id) != 1)
            {
                printf("Invalid product_id %s.\n", optarg ? optarg : "NULL");
            }
            break;

        default:
            break;
        }
    }

    length = strlen(port);
    if (length < 1 || length >= OPTION_LENGTH_MAX - 1)
    {
        printf("Invalid port '%s'\n", port);
        return -1;
    }
    
    length = strlen(filename);
    if (length < 1 || length >= OPTION_LENGTH_MAX - 1)
    {
        printf("Invalid filename '%s'\n", filename);
        return -1;
    }

    length = strlen(common_name);
    if (length < 1 || length >= OPTION_LENGTH_MAX - 1)
    {
        printf("Invalid common_name '%s'\n", common_name);
        return -1;
    }

    if (! vendor_id)
    {
        printf("Invalid vendor_id %04x\n", vendor_id);
        return -1;
    }

    if (! product_id)
    {
        printf("Invalid product_id %04x\n", product_id);
        return -1;
    }

    return host_creds_csr(port, common_name, vendor_id, product_id, filename);
}

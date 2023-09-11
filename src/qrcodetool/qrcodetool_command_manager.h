/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef QRCODETOOL_CMD_MANAGER_H
#define QRCODETOOL_CMD_MANAGER_H

#include "setup_payload_commands.h"

typedef int (*command_func)(int argc, char * const * argv);

typedef struct qrcodetool_command_t
{
    const char * c_name;  /* name of the command. */
    command_func c_func;  /* function to execute the command. */
    const char * c_usage; /* usage string for command. */
    const char * c_help;  /* help string for (or description of) command. */
} qrcodetool_command_t;

qrcodetool_command_t commands[] = { { "generate-qr-code", setup_payload_operation_generate_qr_code,
                                      "    -f File path of payload.\n", "Generate qr code from payload in text file." },

                                    { "generate-manual-code", setup_payload_operation_generate_manual_code,
                                      "[-f file-path]\n"
                                      "    -f File path of payload.\n",
                                      "Generate manual code from payload in text file." },
                                    // Last one
                                    {} };

#endif

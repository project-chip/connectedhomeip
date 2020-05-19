/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
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

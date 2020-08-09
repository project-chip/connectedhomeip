/**
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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************//**
 * @file
 * @brief CLI for the Identify plugin.
 *******************************************************************************
   ******************************************************************************/

// *******************************************************************
// * identify-cli.c
// *
// *
// * Copyright 2012 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"

void emAfPluginIdentifyCliPrint(void);

#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginIdentifyCommands[] = {
  emberCommandEntryAction("print", emAfPluginIdentifyCliPrint, "", "Print the identify state of each endpoint"),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

// plugin identify print
void emAfPluginIdentifyCliPrint(void)
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_IDENTIFY_CLUSTER)
  uint8_t i;
  for (i = 0; i < emberAfEndpointCount(); ++i) {
    uint8_t endpoint = emberAfEndpointFromIndex(i);
    emberAfIdentifyClusterPrintln("Endpoint 0x%x is identifying: %p",
                                  endpoint,
                                  (emberAfIsDeviceIdentifying(endpoint)
                                   ? "true"
                                   : "false"));
  }
#endif //defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_IDENTIFY_CLUSTER)
}

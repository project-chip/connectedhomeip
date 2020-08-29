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
/****************************************************************************
 * @file
 * @brief CLI for the Scenes plugin.
 *******************************************************************************
 ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "scenes.h"

void emAfPluginScenesServerClear(void);

#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginScenesCommands[] = {
    emberCommandEntryAction("print", emAfPluginScenesServerPrintInfo, "", "Print the scenes table."),
    emberCommandEntryAction("clear", emAfPluginScenesServerClear, "", "Clear the scenes table on every endpoint."),
    emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

void emAfPluginScenesServerClear(void)
{
    emberAfCorePrintln("Clearing all scenes.");
    emberAfScenesClusterClearSceneTableCallback(EMBER_BROADCAST_ENDPOINT);
}

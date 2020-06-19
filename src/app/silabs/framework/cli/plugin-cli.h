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
 * @brief Entry point for CLI commands contributed by plugins.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_PLUGIN_CLI_H
#define SILABS_PLUGIN_CLI_H

#include ATTRIBUTE_STORAGE_CONFIGURATION

#ifdef EMBER_AF_GENERATED_PLUGIN_COMMAND_DECLARATIONS
  #define EMBER_AF_PLUGIN_COMMANDS { "plugin", NULL, (const char *)emberAfPluginCommands },
extern EmberCommandEntry emberAfPluginCommands[];
EMBER_AF_GENERATED_PLUGIN_COMMAND_DECLARATIONS
#else
  #define EMBER_AF_PLUGIN_COMMANDS
#endif

#endif // SILABS_PLUGIN_CLI_H

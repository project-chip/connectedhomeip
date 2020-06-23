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
 * @brief Macros for custom CLI.
 *******************************************************************************
   ******************************************************************************/

#ifndef CUSTOM_CLI_H
 #define CUSTOM_CLI_H
 #ifdef EMBER_AF_ENABLE_CUSTOM_COMMANDS
extern EmberCommandEntry emberAfCustomCommands[];
   #ifndef CUSTOM_SUBMENU_NAME
     #define CUSTOM_SUBMENU_NAME "custom"
   #endif
   #ifndef CUSTOM_SUBMENU_DESCRIPTION
     #define CUSTOM_SUBMENU_DESCRIPTION "Custom commands defined by the developer"
   #endif
   #define CUSTOM_COMMANDS   emberCommandEntrySubMenu(CUSTOM_SUBMENU_NAME, emberAfCustomCommands, CUSTOM_SUBMENU_DESCRIPTION),
 #else
   #ifndef CUSTOM_COMMANDS
     #define CUSTOM_COMMANDS
   #endif
 #endif
#endif

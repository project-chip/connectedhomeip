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
 * @brief CLI commands for forming/joining using the hardware buttons.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"

void emberAfPluginButtonJoiningButton0Command(void);
void emberAfPluginButtonJoiningButton1Command(void);

extern void emberAfPluginButtonJoiningPressButton(uint8_t button);

#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginButtonJoiningCommands[] = {
  emberCommandEntryAction("button0",
                          emberAfPluginButtonJoiningButton0Command,
                          "",
                          "Press button 0"),
  emberCommandEntryAction("button1",
                          emberAfPluginButtonJoiningButton1Command,
                          "",
                          "Press button 1"),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

void emberAfPluginButtonJoiningButton0Command(void)
{
  emberAfPluginButtonJoiningPressButton(BUTTON0);
}

void emberAfPluginButtonJoiningButton1Command(void)
{
  emberAfPluginButtonJoiningPressButton(BUTTON1);
}

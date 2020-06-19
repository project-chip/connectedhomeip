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
 * @brief This file provides the CLI for interacting with the Exosite website.
 *******************************************************************************
   ******************************************************************************/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "app/framework/include/af.h"
#include "app/framework/plugin/exosite-demo/exosite-demo.h"

//=============================================================================
// Globals

//=============================================================================

void emberAfPluginExositeDemoPrintStatusCommand(void)
{
  bool lightOn;
  EmberStatus status = emberAfPluginExositeDemoGetLightStatus(&lightOn);
  if (status) {
    emberAfCorePrintln("Error: Failed to retrieve light status.");
    return;
  }

  emberAfCorePrintln("Light is: %p", (lightOn ? "on" : "off"));
}

void emberAfPluginExositeDemoSetStatusCommand(void)
{
  // Looking for 'on' or 'off' text.
  bool turnOn = (emberCommandName()[1] == 'n');

  if (emberAfPluginExositeDemoSetLightStatus(turnOn)) {
    emberAfCorePrintln("Error: Could not set light state.");
  }
}

#endif // DOXYGEN_SHOULD_SKIP_THIS

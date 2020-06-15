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
 * @brief CLI for the Update TC Link Key plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "update-tc-link-key.h"
#include "app/framework/plugin/network-steering/network-steering.h"
#include "app/framework/plugin/network-steering/network-steering-internal.h"

// -----------------------------------------------------------------------------
// Helper macros and functions

void emberAfPluginSetTCLinkKeyUpdateTimerCommand(void)
{
  uint32_t timeInMilliseconds = (uint32_t)emberUnsignedCommandArgument(0);
  emberAfPluginSetTCLinkKeyUpdateTimerMilliSeconds(timeInMilliseconds);
}

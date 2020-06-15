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
 * @brief CLI for the EM4 plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "em4.h"

void emberAfPluginEm4StatusCommand(void)
{
  uint32_t tmp;
  if (!emberAfPluginEm4EnterCallback(&tmp)) {
    emberAfCorePrintln("Inactive");
    emberAfCorePrintln("In order to activate EM4 mode you must implement emberAfPluginEm4EnterCallback() to return True");
  } else {
    emberAfCorePrintln("Active");
  }
}

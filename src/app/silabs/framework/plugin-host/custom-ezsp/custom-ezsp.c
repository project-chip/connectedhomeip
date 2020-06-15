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
 * @brief A sample of custom EZSP protocol.
 *******************************************************************************
   ******************************************************************************/

#include "custom-ezsp.h"

#include "app/framework/include/af.h"
#include "app/xncp/xncp-sample-custom-ezsp-protocol.h"

void ezspCustomFrameHandler(uint8_t payloadLength, uint8_t* payload)
{
  uint8_t commandId;
  assert(payloadLength > 0);

  commandId = payload[0];

  switch (commandId) {
    case EMBER_CUSTOM_EZSP_CALLBACK_REPORT:
      emberAfCorePrintln("Got report, count=0x%x",
                         HIGH_LOW_TO_INT(payload[2],
                                         payload[1]));
      break;
  }
}

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
 * @brief Routines for the Poll Control Client plugin, which implement the
 *        client side of the Poll Control cluster. The Poll Control cluster
 *        provides a means to communicate with an end device with a sleep
 *        schedule.
 *******************************************************************************
   ******************************************************************************/

#include "../../include/af.h"
#include "poll-control-client.h"

static bool fastPolling = false;
static bool respondToCheckIn = true;
static uint16_t fastPollingTimeout = EMBER_AF_PLUGIN_POLL_CONTROL_CLIENT_DEFAULT_FAST_POLL_TIMEOUT;

void emAfSetFastPollingMode(bool mode)
{
  fastPolling = mode;
}

void emAfSetFastPollingTimeout(uint16_t timeout)
{
  fastPollingTimeout = timeout;
}

void emAfSetResponseMode(bool mode)
{
  respondToCheckIn = mode;
}

bool emberAfPollControlClusterCheckInCallback(void)
{
  emberAfPollControlClusterPrintln("RX: CheckIn");
  if (respondToCheckIn) {
    emberAfFillCommandPollControlClusterCheckInResponse(fastPolling,
                                                        fastPollingTimeout);
    emberAfSendResponse();
  }
  return true;
}

void emAfPollControlClientPrint(void)
{
  emberAfPollControlClusterPrintln("Poll Control Client:\n%p %p\n%p 0x%2x",
                                   "fast polling: ",
                                   fastPolling ? "on" : "off",
                                   "fast polling timeout: ",
                                   fastPollingTimeout);
}

/***************************************************************************//**
 *
 *    <COPYRIGHT>
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
 *
 ******************************************************************************
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_HAL
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_ZCL_CORE

#include "poll-control-client.h"

//TODO - Client checkIn response content fixed for now...
static bool fastPolling = true;
static bool respondToCheckIn = true;
static uint16_t fastPollingTimeout = 12;

void emAfSetFastPollingMode(bool mode)
{
  fastPolling = mode;
}

void emAfSetFastPollingTimeout(uint16_t timeout)
{
  fastPollingTimeout = timeout;
}

void emAfSetResponseMode(boolean mode)
{
  respondToCheckIn = mode;
}

void emberZclClusterPollControlClientCommandCheckInRequestHandler(const EmberZclCommandContext_t *context, const EmberZclClusterPollControlClientCommandCheckInRequest_t *request)
{
  if (respondToCheckIn) {
    EmberZclClusterPollControlClientCommandCheckInResponse_t response;
    response.startFastPolling = fastPolling;
    response.fastPollTimeout = fastPollingTimeout;

    emberZclSendClusterPollControlClientCommandCheckInResponse(context,
                                                               &response);
  }
}

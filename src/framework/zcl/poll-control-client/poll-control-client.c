/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
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

/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#include CHIP_AF_API_HAL
#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#endif
#include CHIP_AF_API_ZCL_CORE

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

void chipZclClusterPollControlClientCommandCheckInRequestHandler(const ChipZclCommandContext_t *context, const ChipZclClusterPollControlClientCommandCheckInRequest_t *request)
{
  if (respondToCheckIn) {
    ChipZclClusterPollControlClientCommandCheckInResponse_t response;
    response.startFastPolling = fastPolling;
    response.fastPollTimeout = fastPollingTimeout;

    chipZclSendClusterPollControlClientCommandCheckInResponse(context,
                                                               &response);
  }
}

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
#include "thread-callbacks.h"

static uint16_t getIdentifyTimeS(ChipZclEndpointId_t endpointId);
static void setIdentifyTimeS(ChipZclEndpointId_t endpointId,
                             uint16_t identifyTimeS,
                             bool external);
static bool allIdentifyTimesAre0(void);

ChipEventControl chZclIdentifyServerEventControl;
static bool notify = true;

void chZclIdentifyServerPostAttributeChangeHandler(ChipZclEndpointId_t endpointId,
                                                   const ChipZclClusterSpec_t *clusterSpec,
                                                   ChipZclAttributeId_t attributeId,
                                                   const void *buffer,
                                                   size_t bufferLength)
{
  if (chipZclAreClusterSpecsEqual(&chipZclClusterIdentifyServerSpec,
                                   clusterSpec)
      && attributeId == CHIP_ZCL_CLUSTER_IDENTIFY_SERVER_ATTRIBUTE_IDENTIFY_TIME) {
    uint16_t identifyTimeS = getIdentifyTimeS(endpointId);
    if (identifyTimeS == 0) {
      if (chipEventControlGetActive(chZclIdentifyServerEventControl)) {
        if (allIdentifyTimesAre0()) {
          chipEventControlSetInactive(chZclIdentifyServerEventControl);
        }
        chipZclIdentifyServerStopIdentifyingCallback(endpointId);
      }
    } else {
      if (notify) {
        chipZclIdentifyServerStartIdentifyingCallback(endpointId, identifyTimeS);
      }
      chipEventControlSetDelayMS(chZclIdentifyServerEventControl,
                                  MILLISECOND_TICKS_PER_SECOND);
    }
  }
}

void chZclIdentifyServerEventHandler(void)
{
  for (ChipZclEndpointIndex_t index = 0;
       index < CHIP_ZCL_CLUSTER_IDENTIFY_SERVER_COUNT;
       index++) {
    ChipZclEndpointId_t endpointId
      = chipZclEndpointIndexToId(index, &chipZclClusterIdentifyServerSpec);
    uint16_t identifyTimeS = getIdentifyTimeS(endpointId);
    if (identifyTimeS > 0) {
      setIdentifyTimeS(endpointId,
                       identifyTimeS - 1,
                       false); // internal change
    }
  }
}

void chipZclClusterIdentifyServerCommandIdentifyRequestHandler(const ChipZclCommandContext_t *context,
                                                                const ChipZclClusterIdentifyServerCommandIdentifyRequest_t *request)
{
  chipAfCorePrintln("RX: Identify");
  setIdentifyTimeS(context->endpointId,
                   request->identifyTime,
                   true); // external change
  chipZclSendDefaultResponse(context, CHIP_ZCL_STATUS_SUCCESS);
}

void chipZclClusterIdentifyServerCommandIdentifyQueryRequestHandler(const ChipZclCommandContext_t *context,
                                                                     const ChipZclClusterIdentifyServerCommandIdentifyQueryRequest_t *request)
{
  ChipZclClusterIdentifyServerCommandIdentifyQueryResponse_t response;
  chipAfCorePrintln("RX: IdentifyQuery");
  response.timeout = getIdentifyTimeS(context->endpointId);
  chipZclSendClusterIdentifyServerCommandIdentifyQueryResponse(context,
                                                                &response);
}

static uint16_t getIdentifyTimeS(ChipZclEndpointId_t endpointId)
{
  uint16_t identifyTimeS;
  if (chipZclReadAttribute(endpointId,
                            &chipZclClusterIdentifyServerSpec,
                            CHIP_ZCL_CLUSTER_IDENTIFY_SERVER_ATTRIBUTE_IDENTIFY_TIME,
                            &identifyTimeS,
                            sizeof(identifyTimeS))
      == CHIP_ZCL_STATUS_SUCCESS) {
    return identifyTimeS;
  } else {
    return 0;
  }
}

static void setIdentifyTimeS(ChipZclEndpointId_t endpointId,
                             uint16_t identifyTimeS,
                             bool external)
{
  notify = external;
  chipZclWriteAttribute(endpointId,
                         &chipZclClusterIdentifyServerSpec,
                         CHIP_ZCL_CLUSTER_IDENTIFY_SERVER_ATTRIBUTE_IDENTIFY_TIME,
                         &identifyTimeS,
                         sizeof(identifyTimeS));
  notify = true;
}

static bool allIdentifyTimesAre0(void)
{
  for (ChipZclEndpointIndex_t index = 0;
       index < CHIP_ZCL_CLUSTER_IDENTIFY_SERVER_COUNT;
       index++) {
    ChipZclEndpointId_t endpointId
      = chipZclEndpointIndexToId(index, &chipZclClusterIdentifyServerSpec);
    if (getIdentifyTimeS(endpointId) > 0) {
      return false;
    }
  }
  return true;
}

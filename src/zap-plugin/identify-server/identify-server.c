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
#include "thread-callbacks.h"

static uint16_t getIdentifyTimeS(EmberZclEndpointId_t endpointId);
static void setIdentifyTimeS(EmberZclEndpointId_t endpointId,
                             uint16_t identifyTimeS,
                             bool external);
static bool allIdentifyTimesAre0(void);

EmberEventControl emZclIdentifyServerEventControl;
static bool notify = true;

void emZclIdentifyServerPostAttributeChangeHandler(EmberZclEndpointId_t endpointId,
                                                   const EmberZclClusterSpec_t *clusterSpec,
                                                   EmberZclAttributeId_t attributeId,
                                                   const void *buffer,
                                                   size_t bufferLength)
{
  if (emberZclAreClusterSpecsEqual(&emberZclClusterIdentifyServerSpec,
                                   clusterSpec)
      && attributeId == EMBER_ZCL_CLUSTER_IDENTIFY_SERVER_ATTRIBUTE_IDENTIFY_TIME) {
    uint16_t identifyTimeS = getIdentifyTimeS(endpointId);
    if (identifyTimeS == 0) {
      if (emberEventControlGetActive(emZclIdentifyServerEventControl)) {
        if (allIdentifyTimesAre0()) {
          emberEventControlSetInactive(emZclIdentifyServerEventControl);
        }
        emberZclIdentifyServerStopIdentifyingCallback(endpointId);
      }
    } else {
      if (notify) {
        emberZclIdentifyServerStartIdentifyingCallback(endpointId, identifyTimeS);
      }
      emberEventControlSetDelayMS(emZclIdentifyServerEventControl,
                                  MILLISECOND_TICKS_PER_SECOND);
    }
  }
}

void emZclIdentifyServerEventHandler(void)
{
  for (EmberZclEndpointIndex_t index = 0;
       index < EMBER_ZCL_CLUSTER_IDENTIFY_SERVER_COUNT;
       index++) {
    EmberZclEndpointId_t endpointId
      = emberZclEndpointIndexToId(index, &emberZclClusterIdentifyServerSpec);
    uint16_t identifyTimeS = getIdentifyTimeS(endpointId);
    if (identifyTimeS > 0) {
      setIdentifyTimeS(endpointId,
                       identifyTimeS - 1,
                       false); // internal change
    }
  }
}

void emberZclClusterIdentifyServerCommandIdentifyRequestHandler(const EmberZclCommandContext_t *context,
                                                                const EmberZclClusterIdentifyServerCommandIdentifyRequest_t *request)
{
  emberAfCorePrintln("RX: Identify");
  setIdentifyTimeS(context->endpointId,
                   request->identifyTime,
                   true); // external change
  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
}

void emberZclClusterIdentifyServerCommandIdentifyQueryRequestHandler(const EmberZclCommandContext_t *context,
                                                                     const EmberZclClusterIdentifyServerCommandIdentifyQueryRequest_t *request)
{
  EmberZclClusterIdentifyServerCommandIdentifyQueryResponse_t response;
  emberAfCorePrintln("RX: IdentifyQuery");
  response.timeout = getIdentifyTimeS(context->endpointId);
  emberZclSendClusterIdentifyServerCommandIdentifyQueryResponse(context,
                                                                &response);
}

static uint16_t getIdentifyTimeS(EmberZclEndpointId_t endpointId)
{
  uint16_t identifyTimeS;
  if (emberZclReadAttribute(endpointId,
                            &emberZclClusterIdentifyServerSpec,
                            EMBER_ZCL_CLUSTER_IDENTIFY_SERVER_ATTRIBUTE_IDENTIFY_TIME,
                            &identifyTimeS,
                            sizeof(identifyTimeS))
      == EMBER_ZCL_STATUS_SUCCESS) {
    return identifyTimeS;
  } else {
    return 0;
  }
}

static void setIdentifyTimeS(EmberZclEndpointId_t endpointId,
                             uint16_t identifyTimeS,
                             bool external)
{
  notify = external;
  emberZclWriteAttribute(endpointId,
                         &emberZclClusterIdentifyServerSpec,
                         EMBER_ZCL_CLUSTER_IDENTIFY_SERVER_ATTRIBUTE_IDENTIFY_TIME,
                         &identifyTimeS,
                         sizeof(identifyTimeS));
  notify = true;
}

static bool allIdentifyTimesAre0(void)
{
  for (EmberZclEndpointIndex_t index = 0;
       index < EMBER_ZCL_CLUSTER_IDENTIFY_SERVER_COUNT;
       index++) {
    EmberZclEndpointId_t endpointId
      = emberZclEndpointIndexToId(index, &emberZclClusterIdentifyServerSpec);
    if (getIdentifyTimeS(endpointId) > 0) {
      return false;
    }
  }
  return true;
}

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
 * @brief Definitions for the Metering Server plugin, which implements the
 *        server side of the Simple Metering cluster.
 *******************************************************************************
   ******************************************************************************/

#ifdef EMBER_SCRIPTED_TEST
#include "metering-server-test.h"
#endif

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "metering-server.h"
#include "metering-interface.h"

#ifdef EMBER_AF_PLUGIN_REPORTING
#include "app/framework/plugin/reporting/reporting.h"
#endif

#define METERING_REPORTABLE_CHANGES \
  (EMBER_AF_PLUGIN_METERING_SERVER_DEFAULT_REPORTABLE_ENERGY_CHANGE_WS)

#define METERING_MAX_SAMPLES_PER_SESSION \
  (EMBER_AF_PLUGIN_METERING_SERVER_MAX_SAMPLES_PER_SESSION + 1)

#define METERING_MAX_FAST_POLLING_PERIOD_IN_MINUTES                  15
#define METERING_MIN_DEFAULT_REPORT_INTERVAL_S                        1
#define METERING_MAX_DEFAULT_REPORT_INTERVAL_S                      300

#define METERING_SUPPLY_STATUS_OFF                                    0
#define METERING_SUPPLY_STATUS_OFF_ARMED                              1
#define METERING_SUPPLY_STATUS_ON                                     2

#define METERING_SUPPLY_CONTROL_ACK_REQUIRED                          1
#define METERING_SUPPLY_CONTROL_ACK_BIT                               1

#define METERING_NOTIFICATION_SCHEME_SINGLE                           1
#define METERING_NOTIFICATION_SCHEME_MULTIPLE                         2
#define METERING_NOTIFICATION_SCHEME_SINGLE_MSG_LEN                   4
#define METERING_NOTIFICATION_SCHEME_MULTIPLE_MSG_LEN                20
// Get Sampled Data Response does not send the maximum number of samples.
// If we want 5 samples(which we interpret as diff's between 2 consecutive
// sampling data points) we need 6 data points.
#define METERING_SAMPLE_DATA_SIZE 6

void emberAfPluginMeteringServerProcessNotificationFlagsCallback(
  uint16_t attributeId,
  uint32_t attributeValue);

static uint32_t minIssuerEventId = 0x00000000;

typedef struct {
  uint8_t endpoint;
  uint32_t issuerEventId;
  uint32_t startTime;
  uint16_t sampleId;
  uint8_t  sampleType;
  uint16_t sampleRequestInterval;
  uint16_t maxNumberOfSamples;
  uint8_t  validSamples;
  uint8_t  samples[METERING_SAMPLE_DATA_SIZE][METERING_MAX_SAMPLES_PER_SESSION];
} EmberAfSimpleMeteringClusterSamplingData;

typedef struct {
  uint8_t srcEndpoint;
  uint8_t destEndpoint;
  uint16_t nodeId;
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t implementationDateTime;
  uint8_t proposedSupplyStatus;
} EmberAfSimpleMeteringClusterSupplyEvent;

static uint32_t fastPollEndTimeUtcTable[EMBER_AF_SIMPLE_METERING_CLUSTER_SERVER_ENDPOINT_COUNT];
static EmberAfSimpleMeteringClusterSamplingData samplingData[EMBER_AF_PLUGIN_METERING_SERVER_MAX_SAMPLING_SESSIONS];
static EmberAfSimpleMeteringClusterSupplyEvent changeSupply;

EmberEventControl emberAfPluginMeteringServerSamplingEventControl;
EmberEventControl emberAfPluginMeteringServerSupplyEventControl;

static bool fastPolling = false;

void emberAfPluginMeteringServerEnableFastPolling(bool enableFastPolling)
{
  fastPolling = enableFastPolling;
}

static void samplingDataInit(void)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_METERING_SERVER_MAX_SAMPLING_SESSIONS; i++) {
    samplingData[i].sampleId = MAX_INT16U_VALUE;
    samplingData[i].validSamples = 0x00;
  }
}

static uint8_t findSamplingSession(uint16_t sampleId)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_METERING_SERVER_MAX_SAMPLING_SESSIONS; i++) {
    if (samplingData[i].sampleId == sampleId) {
      return i;
    }
  }
  return MAX_INT8U_VALUE;
}

static uint8_t findSamplingSessionByEventId(uint32_t issuerEventId)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_METERING_SERVER_MAX_SAMPLING_SESSIONS; i++) {
    if (samplingData[i].issuerEventId == issuerEventId) {
      return i;
    }
  }
  return MAX_INT8U_VALUE;
}

static void checkForReportingConfig(void)
{
#ifdef EMBER_AF_PLUGIN_REPORTING
  uint8_t i, endpoint;
  EmberAfPluginReportingEntry defaultReportingEntry = {
    EMBER_ZCL_REPORTING_DIRECTION_REPORTED,
    0, // this will be populated below
    ZCL_SIMPLE_METERING_CLUSTER_ID,
    ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID,
    CLUSTER_MASK_SERVER,
    EMBER_AF_NULL_MANUFACTURER_CODE,
    .data.reported = {
      METERING_MIN_DEFAULT_REPORT_INTERVAL_S, // minimum interval (second)
      METERING_MAX_DEFAULT_REPORT_INTERVAL_S,// maximum interval (second)
      METERING_REPORTABLE_CHANGES // report when this amount changes
    }
  };

  for (i = 0; i < emberAfEndpointCount(); i++) {
    endpoint = emberAfEndpointFromIndex(i);
    if (emberAfContainsServer(endpoint,
                              ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID)) {
      defaultReportingEntry.endpoint = endpoint;
      emAfPluginReportingConditionallyAddReportingEntry(
        &defaultReportingEntry);
    }
  }
#endif
}

void emberAfSimpleMeteringClusterServerInitCallback(uint8_t endpoint)
{
  checkForReportingConfig();
  emberAfPluginMeteringServerInterfaceInit(endpoint);
  MEMSET(fastPollEndTimeUtcTable, 0, EMBER_AF_SIMPLE_METERING_CLUSTER_SERVER_ENDPOINT_COUNT
         * sizeof(fastPollEndTimeUtcTable[0]));
  samplingDataInit();
  emberAfScheduleServerTick(endpoint,
                            ZCL_SIMPLE_METERING_CLUSTER_ID,
                            MILLISECOND_TICKS_PER_SECOND);
}

void emberAfSimpleMeteringClusterServerTickCallback(uint8_t endpoint)
{
  emberAfPluginMeteringServerInterfaceTick(endpoint);
  emberAfScheduleServerTick(endpoint,
                            ZCL_SIMPLE_METERING_CLUSTER_ID,
                            MILLISECOND_TICKS_PER_SECOND);
}

bool emberAfSimpleMeteringClusterGetProfileCallback(uint8_t intervalChannel,
                                                    uint32_t endTime,
                                                    uint8_t numberOfPeriods)
{
  return emberAfPluginMeteringServerInterfaceGetProfiles(intervalChannel,
                                                         endTime,
                                                         numberOfPeriods);
}

bool emberAfSimpleMeteringClusterRequestFastPollModeCallback(
  uint8_t fastPollUpdatePeriod,
  uint8_t durationInMinutes)
{
  uint8_t endpoint;
  uint8_t endpointIndex;
  uint8_t appliedUpdateRate;
  EmberStatus status;
  uint8_t fastPollingUpdateAttribute;
  uint32_t currentTime;

  if (!fastPolling) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND);
    return true;
  }
  endpoint = emberAfCurrentEndpoint();
  endpointIndex = emberAfFindClusterServerEndpointIndex(endpoint,
                                                        ZCL_SIMPLE_METERING_CLUSTER_ID);

  appliedUpdateRate = fastPollUpdatePeriod;

  if (endpointIndex == MAX_INT8U_VALUE) {
    emberAfSimpleMeteringClusterPrintln("Invalid endpoint %x",
                                        emberAfCurrentEndpoint());
    return false;
  }

  status = emberAfReadServerAttribute(endpoint,
                                      ZCL_SIMPLE_METERING_CLUSTER_ID,
                                      ZCL_FAST_POLL_UPDATE_PERIOD_ATTRIBUTE_ID,
                                      (uint8_t *)&fastPollingUpdateAttribute,
                                      sizeof(fastPollingUpdateAttribute));

  if (status == EMBER_SUCCESS) {
    if (fastPollUpdatePeriod < fastPollingUpdateAttribute) {
      appliedUpdateRate = fastPollingUpdateAttribute;
      emberAfSimpleMeteringClusterPrintln(
        "Applying fast Poll rate %x endpointIndex %u",
        appliedUpdateRate,
        endpointIndex);
    }
  } else {
    emberAfSimpleMeteringClusterPrintln(
      "Reading fast Poll Attribute failed. endpointIndex %u  status %x",
      endpointIndex,
      status);
    emberAfFillCommandSimpleMeteringClusterRequestFastPollModeResponse(
      0,                                        // appliedUpdatePeriod
      0);                                       // fastPollModeEndtime
    emberAfSendResponse();
    return true;
  }

  currentTime = emberAfGetCurrentTime();
  if (currentTime > fastPollEndTimeUtcTable[endpointIndex]) {
    durationInMinutes = ((durationInMinutes
                          > METERING_MAX_FAST_POLLING_PERIOD_IN_MINUTES)
                         ? METERING_MAX_FAST_POLLING_PERIOD_IN_MINUTES
                         : durationInMinutes);
    fastPollEndTimeUtcTable[endpointIndex] = currentTime
                                             + (durationInMinutes * 60);
    emberAfSimpleMeteringClusterPrintln(
      "Starting fast polling for %u minutes\
         End Time 0x%4x,current Time 0x%4x",
      durationInMinutes,
      fastPollEndTimeUtcTable[endpointIndex],
      currentTime);
  } else {
    emberAfSimpleMeteringClusterPrintln(
      "Fast polling mode currently active. endpointIndex %u \
         fastPollEndTimeUtcTable[%u] 0x%4x \
         current Time 0x%4x ",
      endpointIndex,
      endpointIndex,
      fastPollEndTimeUtcTable[endpointIndex],
      currentTime);
  }
  emberAfFillCommandSimpleMeteringClusterRequestFastPollModeResponse(
    appliedUpdateRate,
    fastPollEndTimeUtcTable[endpointIndex]);
  emberAfSendResponse();
  return true;
}

void emberAfSimpleMeteringClusterServerDefaultResponseCallback(
  uint8_t endpoint,
  uint8_t commandId,
  EmberAfStatus status)
{
  if (commandId == ZCL_REMOVE_MIRROR_COMMAND_ID
      && status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfSimpleMeteringClusterPrintln("Mirror remove FAILED status 0x%x",
                                        status);
  }
}

bool emberAfSimpleMeteringClusterGetSampledDataCallback(
  uint16_t sampleId,
  uint32_t earliestSampleTime,
  uint8_t sampleType,
  uint16_t numberOfSamples)
{
  uint8_t i;
  emberAfSimpleMeteringClusterPrintln(
    "sampleId %u earliestSampleTime %u sampleType %u numberOfSamples %u",
    sampleId,
    earliestSampleTime,
    sampleType,
    numberOfSamples);
  for (i = 0; i < EMBER_AF_PLUGIN_METERING_SERVER_MAX_SAMPLING_SESSIONS; i++) {
    uint8_t j;
    uint32_t diff;
    if (samplingData[i].sampleId == sampleId
        && samplingData[i].sampleId < MAX_INT16U_VALUE
        && samplingData[i].startTime <= earliestSampleTime
        && samplingData[i].sampleType == sampleType) {
      // If we have only one sample data point or lesser, we don't have enough
      // information for even one sample.
      if (samplingData[i].validSamples <= 1) {
        goto kickout;
      }

      if (numberOfSamples > samplingData[i].validSamples - 1) {
        numberOfSamples = samplingData[i].validSamples - 1;
      }

      emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                 | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT),
                                ZCL_SIMPLE_METERING_CLUSTER_ID,
                                ZCL_GET_SAMPLED_DATA_RESPONSE_COMMAND_ID,
                                "vwuvv",
                                samplingData[i].sampleId,
                                samplingData[i].startTime,
                                samplingData[i].sampleType,
                                samplingData[i].sampleRequestInterval,
                                numberOfSamples);

      emberAfSimpleMeteringClusterPrintln("numberOfSamples 0x%2x",
                                          numberOfSamples);

      for (j = 0; j < numberOfSamples; j++) {
        uint32_t b = INT8U_TO_INT32U(samplingData[i].samples[j + 1][3],
                                     samplingData[i].samples[j + 1][2],
                                     samplingData[i].samples[j + 1][1],
                                     samplingData[i].samples[j + 1][0]);
        uint32_t a = INT8U_TO_INT32U(samplingData[i].samples[j][3],
                                     samplingData[i].samples[j][2],
                                     samplingData[i].samples[j][1],
                                     samplingData[i].samples[j][0]);

        diff = b - a;
        emberAfPutInt24uInResp(diff);
        emberAfSimpleMeteringClusterPrintln("index %u numberOfSamples\
                                             %u diff %u",
                                            j,
                                            numberOfSamples,
                                            diff);
      }
      emberAfSendResponse();
      return true;
    }
  }

  kickout:
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
  return true;
}

void emberAfPluginMeteringServerStackStatusCallback(EmberStatus status)
{
  uint8_t i;
  uint8_t endpoint;
  // On network connect, chances are very good that someone (coordinator,
  // gateway, etc) will poll for attribute values. so refresh the attributes.
  if (status == EMBER_NETWORK_UP) {
    // Cycle through all endpoints, check to see if the endpoint has a simple
    // metering server, and if so update the attributes of that endpoint
    checkForReportingConfig();
    for (i = 0; i < emberAfEndpointCount(); i++) {
      endpoint = emberAfEndpointFromIndex(i);
      if (emberAfContainsServer(endpoint, ZCL_SIMPLE_METERING_CLUSTER_ID)) {
        emberAfPluginMeteringServerAttributeInit(endpoint);
      }
    }
  }
}

uint16_t emberAfPluginMeteringServerStartSampling(uint16_t requestedSampleId,
                                                  uint32_t issuerEventId,
                                                  uint32_t startSamplingTime,
                                                  uint8_t sampleType,
                                                  uint16_t sampleRequestInterval,
                                                  uint16_t maxNumberOfSamples,
                                                  uint8_t endpoint)
{
  uint32_t delay;
  uint32_t currentTime;
  static uint16_t nextSampleId = 0x0001;
  uint16_t sampleId;
  uint8_t index;

  emberAfSimpleMeteringClusterPrintln(
    "StartSampling: \
       requestedSampleId %u \
       issuerEventId %u \
       startSamplingTime %u \
       sampleType %u \
       sampleRequestInterval %u \
       maxNumberOfSamples %u \
       endpoint %u",
    requestedSampleId,
    issuerEventId,
    startSamplingTime,
    sampleType,
    sampleRequestInterval,
    maxNumberOfSamples,
    endpoint);

  // Find an unused sampling session table entry
  index = findSamplingSession(MAX_INT16U_VALUE);
  if (index == MAX_INT8U_VALUE) {
    emberAfSimpleMeteringClusterPrintln(
      "ERR: No available entries in sampling session table");
    return MAX_INT16U_VALUE;
  }

  // if the caller requested a specific sampleId then check to make sure it's
  // not already in use.
  if (requestedSampleId != MAX_INT16U_VALUE) {
    if (findSamplingSession(requestedSampleId) != MAX_INT8U_VALUE) {
      emberAfSimpleMeteringClusterPrintln(
        "ERR: requested sampleId already in use");
      return MAX_INT16U_VALUE;
    } else {
      sampleId = requestedSampleId;
    }
  } else {
    sampleId = nextSampleId;
    while (findSamplingSession(sampleId) != MAX_INT8U_VALUE) {
      sampleId = (sampleId == (MAX_INT16U_VALUE - 1)) ? 1 : (sampleId + 1);
    }
    nextSampleId = (sampleId == (MAX_INT16U_VALUE - 1)) ? 1 : (sampleId + 1);
  }

  // This will always be true thanks to the check above, satisfies CSTAT checking
  if (index < EMBER_AF_PLUGIN_METERING_SERVER_MAX_SAMPLING_SESSIONS) {
    samplingData[index].sampleId = sampleId;
    samplingData[index].issuerEventId = issuerEventId;
    samplingData[index].startTime = startSamplingTime;
    samplingData[index].sampleType = sampleType;
    samplingData[index].sampleRequestInterval = sampleRequestInterval;

    // This is a hardcoded limit in EmberAfSimpleMeteringClusterSamplingData.
    if (maxNumberOfSamples > METERING_MAX_SAMPLES_PER_SESSION) {
      maxNumberOfSamples = METERING_MAX_SAMPLES_PER_SESSION;
    }
    samplingData[index].maxNumberOfSamples = maxNumberOfSamples;
    samplingData[index].endpoint = endpoint;

    currentTime = emberAfGetCurrentTime();
    if (startSamplingTime < currentTime) {
      startSamplingTime = currentTime;
    }
    delay = startSamplingTime - currentTime;
    emberEventControlSetDelayMS(emberAfPluginMeteringServerSamplingEventControl,
                                (delay * MILLISECOND_TICKS_PER_SECOND));
    return sampleId;
  } else {
    return MAX_INT16U_VALUE;
  }
}

bool emberAfSimpleMeteringClusterStartSamplingCallback(
  uint32_t issuerEventId,
  uint32_t startSamplingTime,
  uint8_t sampleType,
  uint16_t sampleRequestInterval,
  uint16_t maxNumberOfSamples)
{
  static uint8_t firstIssuerId = 0;
  uint16_t sampleId;
  uint8_t eventIndex;
  // Issuer Eventid should not be ignored. We accept *anything* for the first
  // time. After that, we only accept values greater than the max id received
  // so far(which should be the last valid issuerId received).
  if (firstIssuerId == 0) {
    minIssuerEventId = issuerEventId;
    firstIssuerId = 1;
  } else if (issuerEventId <= minIssuerEventId
             && startSamplingTime != MAX_INT32U_VALUE) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    emberAfSimpleMeteringClusterPrintln(
      "Rejecting StartSamplingCallback issuerEventId %u minIssuerEventId %u",
      issuerEventId,
      minIssuerEventId);
    return true;
  } else {
    minIssuerEventId = issuerEventId;
  }

  // StartSampling Event ID field is ignored by the metering server
  // Special time reserved to cancel a startSampling Request.
  if (startSamplingTime == MAX_INT32U_VALUE) {
    eventIndex = findSamplingSessionByEventId(issuerEventId);
    // Event not found.
    if (eventIndex == MAX_INT8U_VALUE) {
      emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
      return true;
    } else {
      // eventIndex cannot be out of bounds as checked above
      samplingData[eventIndex].sampleId = MAX_INT16U_VALUE;
      samplingData[eventIndex].validSamples = 0x00;
      emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
      return true;
    }
  }

  sampleId = emberAfPluginMeteringServerStartSampling(
    MAX_INT16U_VALUE,
    issuerEventId,
    startSamplingTime,
    sampleType,
    sampleRequestInterval,
    maxNumberOfSamples,
    emberAfCurrentEndpoint());
  emberAfFillCommandSimpleMeteringClusterStartSamplingResponse(sampleId);
  emberAfSendResponse();
  return true;
}

void emberAfPluginMeteringServerSamplingEventHandler(void)
{
  uint8_t i;
  EmberAfAttributeType dataType;
  EmberStatus status;
  emberEventControlSetInactive(emberAfPluginMeteringServerSamplingEventControl);

  // Now let's adjust the summation
  for (i = 0; i < EMBER_AF_PLUGIN_METERING_SERVER_MAX_SAMPLING_SESSIONS; i++) {
    if (samplingData[i].startTime <= emberAfGetCurrentTime()
        && samplingData[i].validSamples <= samplingData[i].maxNumberOfSamples
        && samplingData[i].sampleId < MAX_INT16U_VALUE) {
      status = emberAfReadAttribute(
        samplingData[i].endpoint,
        ZCL_SIMPLE_METERING_CLUSTER_ID,
        ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID,
        CLUSTER_MASK_SERVER,
        samplingData[i].samples[samplingData[i].validSamples],
        METERING_SAMPLE_DATA_SIZE,
        &dataType);
      if (status == EMBER_SUCCESS) {
        samplingData[i].validSamples++;
      }
      emberEventControlSetDelayMS(
        emberAfPluginMeteringServerSamplingEventControl,
        samplingData[i].sampleRequestInterval * MILLISECOND_TICKS_PER_SECOND);
    }
  }
}

void emberAfPluginMeteringServerSupplyEventHandler(void)
{
  uint32_t changeTime;
  uint32_t currentTime;
  emberEventControlSetInactive(emberAfPluginMeteringServerSupplyEventControl);

  emberAfReadServerAttribute(
    changeSupply.srcEndpoint,
    ZCL_SIMPLE_METERING_CLUSTER_ID,
    ZCL_PROPOSED_CHANGE_SUPPLY_IMPLEMENTATION_TIME_ATTRIBUTE_ID,
    (uint8_t *)&changeTime,
    sizeof(changeTime));
  if (changeTime == MAX_INT32U_VALUE) {
    // Nothing pending
    return;
  }

  currentTime = emberAfGetCurrentTime();
  if (changeTime > currentTime) {
    uint32_t delay = changeTime - currentTime;
    emberEventControlSetDelayMS(emberAfPluginMeteringServerSupplyEventControl,
                                delay * MILLISECOND_TICKS_PER_SECOND);
  } else {
    // Per SE specification: "The ProposedChangeImplementationTime attribute
    // indicates the time at which a proposed change to the supply is to be
    // implemented. If there is no change of supply pending, this attribute
    // will be set to 0xFFFFFFFF.
    uint32_t implementationDateTime = MAX_INT32U_VALUE;
    emberAfWriteAttribute(
      changeSupply.srcEndpoint,
      ZCL_SIMPLE_METERING_CLUSTER_ID,
      ZCL_PROPOSED_CHANGE_SUPPLY_IMPLEMENTATION_TIME_ATTRIBUTE_ID,
      CLUSTER_MASK_SERVER,
      (uint8_t *)&implementationDateTime,
      ZCL_UTC_TIME_ATTRIBUTE_TYPE);

    emberAfFillCommandSimpleMeteringClusterSupplyStatusResponse(
      changeSupply.providerId,
      changeSupply.issuerEventId,
      changeSupply.implementationDateTime,
      changeSupply.proposedSupplyStatus);
    emberAfSetCommandEndpoints(
      changeSupply.srcEndpoint,
      changeSupply.destEndpoint);
    emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, changeSupply.nodeId);
  }
}

bool emberAfSimpleMeteringClusterResetLoadLimitCounterCallback(
  uint32_t providerId,
  uint32_t issuerEventId)
{
  uint8_t counter = 0;
  emberAfSimpleMeteringClusterPrintln(
    "Reset Load Counter providerId %u issuerEventId %u",
    providerId,
    issuerEventId);
  emberAfWriteAttribute(emberAfCurrentEndpoint(),
                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                        ZCL_LOAD_LIMIT_COUNTER_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        &counter,
                        ZCL_INT8U_ATTRIBUTE_TYPE);
  return true;
}

bool emberAfSimpleMeteringClusterChangeSupplyCallback(
  uint32_t providerId,
  uint32_t issuerEventId,
  uint32_t requestDateTime,
  uint32_t implementationDateTime,
  uint8_t proposedSupplyStatus,
  uint8_t supplyControlBits)
{
  uint32_t delay;
  EmberAfClusterCommand *cmd;

  emberAfSimpleMeteringClusterPrintln(
    "Change Supply Callback providerId %u \
       issuerEventId %u \
       implementationDateTime %u \
       supplyStatus %u",
    providerId,
    issuerEventId,
    implementationDateTime,
    supplyControlBits);
  if (implementationDateTime < emberAfGetCurrentTime()
      && implementationDateTime != 0) {
    emberAfSimpleMeteringClusterPrintln("implementationDateTime %u",
                                        implementationDateTime);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_VALUE);
    return true;
  }
  if (implementationDateTime == MAX_INT32U_VALUE) {
    emberAfSimpleMeteringClusterPrintln("Canceling change supply");
    // Per SE specification: "The ProposedChangeImplementationTime attribute
    // indicates the time at which a proposed change to the supply is to be
    // implemented. If there is no change of supply pending, this attribute
    // will be set to 0xFFFFFFFF.
    emberAfWriteAttribute(
      emberAfCurrentEndpoint(),
      ZCL_SIMPLE_METERING_CLUSTER_ID,
      ZCL_PROPOSED_CHANGE_SUPPLY_IMPLEMENTATION_TIME_ATTRIBUTE_ID,
      CLUSTER_MASK_SERVER,
      (uint8_t *)&implementationDateTime,
      ZCL_UTC_TIME_ATTRIBUTE_TYPE);
    emberEventControlSetInactive(
      emberAfPluginMeteringServerSupplyEventControl);
  }
  if (implementationDateTime == 0x00000000) {
    if (((supplyControlBits & METERING_SUPPLY_CONTROL_ACK_BIT)
         == METERING_SUPPLY_CONTROL_ACK_REQUIRED)
        || (proposedSupplyStatus == METERING_SUPPLY_STATUS_ON)) {
      emberAfFillCommandSimpleMeteringClusterSupplyStatusResponse(
        providerId,
        issuerEventId,
        implementationDateTime,
        proposedSupplyStatus);
      emberAfSendResponse();
    } else {
      emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    }
  } else {
    emberAfWriteAttribute(
      emberAfCurrentEndpoint(),
      ZCL_SIMPLE_METERING_CLUSTER_ID,
      ZCL_PROPOSED_CHANGE_SUPPLY_IMPLEMENTATION_TIME_ATTRIBUTE_ID,
      CLUSTER_MASK_SERVER,
      (uint8_t *)&implementationDateTime,
      ZCL_UTC_TIME_ATTRIBUTE_TYPE);

    emberAfWriteAttribute(emberAfCurrentEndpoint(),
                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                          ZCL_PROPOSED_CHANGE_SUPPLY_STATUS_ATTRIBUTE_ID,
                          CLUSTER_MASK_SERVER,
                          &proposedSupplyStatus,
                          ZCL_ENUM8_ATTRIBUTE_TYPE);
    delay = implementationDateTime - emberAfGetCurrentTime();
    cmd = emberAfCurrentCommand();
    changeSupply.srcEndpoint = cmd->apsFrame->destinationEndpoint;
    changeSupply.destEndpoint = cmd->apsFrame->sourceEndpoint;
    changeSupply.nodeId = cmd->source;
    changeSupply.providerId = providerId;
    changeSupply.issuerEventId = issuerEventId;
    changeSupply.implementationDateTime = implementationDateTime;
    changeSupply.proposedSupplyStatus = proposedSupplyStatus;
    if ((supplyControlBits & METERING_SUPPLY_CONTROL_ACK_BIT)
        == METERING_SUPPLY_CONTROL_ACK_REQUIRED) {
      emberEventControlSetDelayMS(emberAfPluginMeteringServerSupplyEventControl,
                                  delay * MILLISECOND_TICKS_PER_SECOND);
    }
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  }
  return true;
}

bool emberAfSimpleMeteringClusterLocalChangeSupplyCallback(
  uint8_t proposedSupplyStatus)
{
  if (proposedSupplyStatus < METERING_SUPPLY_STATUS_OFF_ARMED
      || proposedSupplyStatus > METERING_SUPPLY_STATUS_ON) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_AUTHORIZED);
  } else {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    emberAfSimpleMeteringClusterPrintln("Setting localSupply Status %u",
                                        proposedSupplyStatus);
  }
  return true;
}

bool emberAfSimpleMeteringClusterSetSupplyStatusCallback(
  uint32_t issuerEventId,
  uint8_t supplyTamperState,
  uint8_t supplyDepletionState,
  uint8_t supplyUncontrolledFlowState,
  uint8_t loadLimitSupplyState)
{
  emberAfWriteAttribute(emberAfCurrentEndpoint(),
                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                        ZCL_SUPPLY_TAMPER_STATE_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        &supplyTamperState,
                        ZCL_ENUM8_ATTRIBUTE_TYPE);

  emberAfWriteAttribute(emberAfCurrentEndpoint(),
                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                        ZCL_SUPPLY_DEPLETION_STATE_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        &supplyDepletionState,
                        ZCL_ENUM8_ATTRIBUTE_TYPE);

  emberAfWriteAttribute(emberAfCurrentEndpoint(),
                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                        ZCL_SUPPLY_UNCONTROLLED_FLOW_STATE_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        &supplyUncontrolledFlowState,
                        ZCL_ENUM8_ATTRIBUTE_TYPE);

  emberAfWriteAttribute(emberAfCurrentEndpoint(),
                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                        ZCL_LOAD_LIMIT_SUPPLY_STATE_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        &loadLimitSupplyState,
                        ZCL_ENUM8_ATTRIBUTE_TYPE);
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

bool emberAfSimpleMeteringClusterSetUncontrolledFlowThresholdCallback(
  uint32_t providerId,
  uint32_t issuerEventId,
  uint16_t uncontrolledFlowThreshold,
  uint8_t unitOfMeasure,
  uint16_t multiplier,
  uint16_t divisor,
  uint8_t stabilisationPeriod,
  uint16_t measurementPeriod)
{
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

bool emberAfSimpleMeteringClusterMirrorReportAttributeResponseCallback(
  uint8_t notificationScheme,
  uint8_t* notificationFlags)
{
  uint32_t functionalFlags;
  uint32_t notificationFlags2;
  uint32_t notificationFlags3;
  uint32_t notificationFlags4;
  uint32_t notificationFlags5;
  if (notificationScheme == METERING_NOTIFICATION_SCHEME_SINGLE) {
    uint32_t functionalFlags = emberAfGetInt32u(
      notificationFlags,
      0,
      METERING_NOTIFICATION_SCHEME_SINGLE_MSG_LEN);
    emberAfPluginMeteringServerProcessNotificationFlagsCallback(
      ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID,
      functionalFlags);
  } else if (notificationScheme == METERING_NOTIFICATION_SCHEME_MULTIPLE) {
    functionalFlags = emberAfGetInt32u(
      notificationFlags,
      0,
      METERING_NOTIFICATION_SCHEME_MULTIPLE_MSG_LEN);
    notificationFlags2 = emberAfGetInt32u(
      notificationFlags,
      sizeof(uint32_t),
      METERING_NOTIFICATION_SCHEME_MULTIPLE_MSG_LEN);
    notificationFlags3 = emberAfGetInt32u(
      notificationFlags,
      2 * sizeof(uint32_t),
      METERING_NOTIFICATION_SCHEME_MULTIPLE_MSG_LEN);
    notificationFlags4 = emberAfGetInt32u(
      notificationFlags,
      3 * sizeof(uint32_t),
      METERING_NOTIFICATION_SCHEME_MULTIPLE_MSG_LEN);
    notificationFlags5 = emberAfGetInt32u(
      notificationFlags,
      4 * sizeof(uint32_t),
      METERING_NOTIFICATION_SCHEME_MULTIPLE_MSG_LEN);
    emberAfSimpleMeteringClusterPrintln(
      "functionalFlags 0x%4x notificationFlags2-5 0x%4x 0x%4x 0x%4x 0x%4x",
      functionalFlags, notificationFlags2, notificationFlags3,
      notificationFlags4, notificationFlags5);
    emberAfPluginMeteringServerProcessNotificationFlagsCallback(
      ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID,
      functionalFlags);
    emberAfPluginMeteringServerProcessNotificationFlagsCallback(
      ZCL_NOTIFICATION_FLAGS_2_ATTRIBUTE_ID,
      notificationFlags2);
    emberAfPluginMeteringServerProcessNotificationFlagsCallback(
      ZCL_NOTIFICATION_FLAGS_3_ATTRIBUTE_ID,
      notificationFlags3);
    emberAfPluginMeteringServerProcessNotificationFlagsCallback(
      ZCL_NOTIFICATION_FLAGS_4_ATTRIBUTE_ID,
      notificationFlags4);
    emberAfPluginMeteringServerProcessNotificationFlagsCallback(
      ZCL_NOTIFICATION_FLAGS_5_ATTRIBUTE_ID,
      notificationFlags5);
  } else {
    return false;
  }
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

void emberAfPluginSimpleMeteringClusterReadAttributesResponseCallback(
  EmberAfClusterId clusterId,
  uint8_t *buffer,
  uint16_t bufLen)
{
  uint16_t bufIndex = 0;
  uint16_t dataSize;
  EmberAfStatus status;
  uint8_t dataType;
  uint32_t bitMap;

  if (clusterId != ZCL_SIMPLE_METERING_CLUSTER_ID
      || (emberAfCurrentCommand()->direction
          != ZCL_FRAME_CONTROL_CLIENT_TO_SERVER)) {
    return;
  }

  // Each record in the response has a two-byte attribute id and a one-byte
  // status.  If the status is SUCCESS, there will also be a one-byte type and
  // variable-length data.
  while (bufIndex + sizeof(EmberAfAttributeId) + sizeof(EmberAfStatus) <= bufLen) {
    EmberAfAttributeId attributeId =
      (EmberAfAttributeId)emberAfGetInt16u(buffer, bufIndex, bufLen);
    bufIndex += sizeof(uint16_t);
    status = (EmberAfStatus)emberAfGetInt8u(buffer, bufIndex, bufLen);
    bufIndex += sizeof(uint8_t);
    if (status == EMBER_ZCL_STATUS_SUCCESS) {
      dataType = emberAfGetInt8u(buffer, bufIndex, bufLen);
      bufIndex++;

      dataSize = emberAfAttributeValueSize(dataType, buffer + bufIndex);

      if (dataSize <= bufLen - bufIndex) {
        // The Notification Attribute Set is in the range of 0x0000 - 0x00FF
        // and are all 32 bit BitMap types. Each application may decide to
        // handle the notification flags differently so we'll callback to the
        // application for each notification flags attribute.
        if (attributeId < ZCL_CURRENT_TIER1_SUMMATION_DELIVERED_ATTRIBUTE_ID) {
          bitMap = emberAfGetInt32u(buffer, bufIndex, bufLen);
          emberAfSimpleMeteringClusterPrintln("Attribute value 0x%4x", bitMap);
          emberAfPluginMeteringServerProcessNotificationFlagsCallback(
            attributeId,
            bitMap);
        }
        bufIndex += dataSize;
      } else {
        // dataSize exceeds buffer length, terminate loop
        emberAfSimpleMeteringClusterPrintln("ERR: attr:%2x size %d exceeds buffer size", attributeId, dataSize);
        break;
      }
    }
  }
}

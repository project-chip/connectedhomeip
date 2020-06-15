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
 * @brief Routines for the DRLC plugin.
 *******************************************************************************
   ******************************************************************************/

#include "../../include/af.h"
#include "../../util/common.h"
#include "demand-response-load-control.h"

#include "app/framework/plugin/esi-management/esi-management.h"

//------------------------------------------------------------------------------

/**
 * Checks to see if the passed UEG OR deviceClass match those stored in
 * the cluster attributes.
 *
 * Returns true if either the device class or the UEG match. This
 * is the assumed interpretation of the spec..
 **/
static bool afMatchUegOrDeviceClass(uint8_t endpoint,
                                    uint8_t utilityEnrollmentGroup,
                                    uint16_t deviceClass)
{
  uint8_t       ueg      = 0;
  uint16_t      dc;
  uint8_t       dcArray[2];
  uint8_t       dataType = 0;
  uint8_t       status;

  status =
    emberAfReadAttribute(endpoint,
                         ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID,
                         ZCL_UTILITY_ENROLLMENT_GROUP_ATTRIBUTE_ID,
                         CLUSTER_MASK_CLIENT,
                         (uint8_t*)&ueg,
                         1,
                         &dataType);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return false;
  }

  dataType = 0;
  status =
    emberAfReadAttribute(endpoint,
                         ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID,
                         ZCL_DEVICE_CLASS_VALUE_ATTRIBUTE_ID,
                         CLUSTER_MASK_CLIENT,
                         (uint8_t*)dcArray,
                         2,
                         &dataType);
  dc = (uint16_t)dcArray[0] | ((uint16_t)dcArray[1] << 8);
  emberAfDemandResponseLoadControlClusterPrint("Device Class: 0x%2x, status: 0x%x\r\n",
                                               dc,
                                               status);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return false;
  }
  // Our thinking is that
  // device class should always match otherwise there would be no way to
  // narrow the group and there would be no way to address specific devices
  // that are not in a specific group. - WEH
  else if (!(dc & deviceClass)) {
    return false;
  }
  if (ueg == 0 || utilityEnrollmentGroup == 0 || (utilityEnrollmentGroup == ueg)) {
    return true;
  }
  return false;
}

static void addSignature(void)
{
  // The requirement of signing DRLC messages was removed from SE 1.1, however
  // we still must include the signature type and a dummy signature in the
  // message for backwards compatibility.
  appResponseData[appResponseLength] = EMBER_ZCL_SIGNATURE_TYPE_RESERVED;
  appResponseLength++;
  MEMSET(&(appResponseData[appResponseLength]), 0xFF, EMBER_SIGNATURE_SIZE);
  appResponseLength += EMBER_SIGNATURE_SIZE;
}

// -------------------------------------------------------
// DEMAND RESPONSE LOAD CONTROL CLIENT
// -------------------------------------------------------
static void fillReportEventData(uint8_t commandStatus,
                                uint8_t sequenceNumber,
                                EmberAfLoadControlEvent* e,
                                uint32_t statusTime)
{
  //emberAfDemandResponseLoadControlClusterFlush();
  emberAfDemandResponseLoadControlClusterPrintln("= TX Report Event Status =");
  //emberAfDemandResponseLoadControlClusterFlush();
  emberAfDemandResponseLoadControlClusterPrintln("  eid: %4x",
                                                 e->eventId);
  //emberAfDemandResponseLoadControlClusterFlush();
  emberAfDemandResponseLoadControlClusterPrintln("   es: %x",
                                                 commandStatus);
  emberAfDemandResponseLoadControlClusterPrintln("   st: T%4x",
                                                 statusTime);
  //emberAfDemandResponseLoadControlClusterFlush();
  emberAfDemandResponseLoadControlClusterPrintln("  cla: %x",
                                                 e->criticalityLevel);
  emberAfDemandResponseLoadControlClusterPrintln("   ec: %x", e->eventControl);
  emberAfDemandResponseLoadControlClusterPrintln("ctspa: %2x (%d)",
                                                 e->coolingTempSetPoint,
                                                 e->coolingTempSetPoint);
  //emberAfDemandResponseLoadControlClusterFlush();
  emberAfDemandResponseLoadControlClusterPrintln("htspa: %2x (%d)",
                                                 e->heatingTempSetPoint,
                                                 e->heatingTempSetPoint);
  emberAfDemandResponseLoadControlClusterPrintln("  avg: %x", e->avgLoadPercentage);
  //emberAfDemandResponseLoadControlClusterFlush();
  emberAfDemandResponseLoadControlClusterPrintln("   dc: %x", e->dutyCycle);
  emberAfDemandResponseLoadControlClusterFlush();

  appResponseLength = 0;
  emberAfPutInt8uInResp(ZCL_CLUSTER_SPECIFIC_COMMAND
                        | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER);
  emberAfPutInt8uInResp(sequenceNumber);
  emberAfPutInt8uInResp(ZCL_REPORT_EVENT_STATUS_COMMAND_ID);
  emberAfPutInt32uInResp(e->eventId);
  emberAfPutInt8uInResp(commandStatus); //cmd rx
  emberAfPutInt32uInResp(statusTime);
  emberAfPutInt8uInResp(e->criticalityLevel);
  emberAfPutInt16uInResp(e->coolingTempSetPoint);
  emberAfPutInt16uInResp(e->heatingTempSetPoint);
  emberAfPutInt8uInResp(e->avgLoadPercentage);
  emberAfPutInt8uInResp(e->dutyCycle);
  emberAfPutInt8uInResp(e->eventControl);
  addSignature();
}

/**
 * A mechanism for retrieving the right amount of randomization
 * if startRand is true, the table is requesting the amount of
 * start randomization, otherwise it is requesting the amount of
 * duration randomization. The value returned represents seconds of
 * randomization.
 */
static uint16_t afGetRandomizationTime(uint8_t endpoint,
                                       bool startRand)
{
  uint8_t maxRandMin, dataType;
  EmberStatus status =
    emberAfReadAttribute(endpoint,
                         ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID,
                         (startRand
                          ? ZCL_START_RANDOMIZATION_MINUTES_ATTRIBUTE_ID
                          : ZCL_DURATION_RANDOMIZATION_MINUTES_ATTRIBUTE_ID),
                         CLUSTER_MASK_CLIENT,
                         (uint8_t*)&maxRandMin,
                         1,
                         &dataType);
  if ( (status != EMBER_ZCL_STATUS_SUCCESS) || (maxRandMin == 0) ) {
    return 0; //not performed
  } else {
    uint16_t randNum, maxRandSec, randSec;
    randNum = emberGetPseudoRandomNumber();
    maxRandSec = maxRandMin * 60;
    randSec = randNum % maxRandSec;
    return randSec; //return value rep in seconds.
  }
}

void emberAfDemandResponseLoadControlClusterClientInitCallback(uint8_t endpoint)
{
  uint16_t deviceClass = EMBER_AF_PLUGIN_DRLC_DEVICE_CLASS;
  // Initialize device class
  emberAfWriteAttribute(endpoint,
                        ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID,
                        ZCL_DEVICE_CLASS_VALUE_ATTRIBUTE_ID,
                        CLUSTER_MASK_CLIENT,
                        (uint8_t*)(&deviceClass),
                        ZCL_INT16U_ATTRIBUTE_TYPE);

  emAfLoadControlEventTableInit(endpoint);

  // Start tick
  emberAfScheduleClientTick(endpoint,
                            ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID,
                            MILLISECOND_TICKS_PER_SECOND);
}

void emberAfDemandResponseLoadControlClusterClientTickCallback(uint8_t endpoint)
{
  emAfLoadControlEventTableTick(endpoint);
  emberAfScheduleClientTick(endpoint,
                            ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID,
                            MILLISECOND_TICKS_PER_SECOND);
}

bool emberAfDemandResponseLoadControlClusterLoadControlEventCallback(uint32_t eventId,
                                                                     uint16_t deviceClass,
                                                                     uint8_t utilityEnrollmentGroup,
                                                                     uint32_t startTime,
                                                                     uint16_t duration,
                                                                     uint8_t criticalityLevel,
                                                                     uint8_t coolingTempOffset,
                                                                     uint8_t heatingTempOffset,
                                                                     int16_t coolingTempSetPoint,
                                                                     int16_t heatingTempSetPoint,
                                                                     int8_t averageLoadAdjustPercent,
                                                                     uint8_t dutyCycle,
                                                                     uint8_t eventControl)
{
  EmberAfLoadControlEvent e;
  EmberAfClusterCommand *cc = emberAfCurrentCommand();

  emberAfDemandResponseLoadControlClusterPrintln("= RX Load Control Event =");
  emberAfDemandResponseLoadControlClusterPrintln("  eid: %4x", eventId);
  emberAfDemandResponseLoadControlClusterPrintln(" dc/g: %2x / %x",
                                                 deviceClass,
                                                 utilityEnrollmentGroup);
  emberAfDemandResponseLoadControlClusterPrintln("   st: T%4x", startTime);
  emberAfDemandResponseLoadControlClusterFlush();
  emberAfDemandResponseLoadControlClusterPrintln("  dur: %2x", duration);
  emberAfDemandResponseLoadControlClusterPrintln("  cla: %x", criticalityLevel);
  emberAfDemandResponseLoadControlClusterPrintln("co/ho: %x / %x",
                                                 coolingTempOffset,
                                                 heatingTempOffset);
  emberAfDemandResponseLoadControlClusterFlush();
  emberAfDemandResponseLoadControlClusterPrintln("ctspa: %2x (%d)",
                                                 coolingTempSetPoint,
                                                 coolingTempSetPoint);
  emberAfDemandResponseLoadControlClusterPrintln("htspa: %2x (%d)",
                                                 heatingTempSetPoint,
                                                 heatingTempSetPoint);
  emberAfDemandResponseLoadControlClusterFlush();
  emberAfDemandResponseLoadControlClusterPrintln("  avg: %x",
                                                 averageLoadAdjustPercent);
  emberAfDemandResponseLoadControlClusterPrintln("   dc: %x", dutyCycle);
  emberAfDemandResponseLoadControlClusterPrintln("   ec: %x", eventControl);
  emberAfDemandResponseLoadControlClusterFlush();

  if (!afMatchUegOrDeviceClass(emberAfCurrentEndpoint(),
                               utilityEnrollmentGroup,
                               deviceClass)) {
    emberAfDemandResponseLoadControlClusterPrintln(
      "Neither UEG nor Device class matched.  Ignoring.");

    // CCB 1380: Device must send back a default response of success, or do
    // nothing. We choose to do nothing.
    //emberAfSendDefaultResponse(emberAfCurrentCommand(),
    //                           EMBER_ZCL_STATUS_NOT_FOUND);
    return true;
  }

  // Reserved criticality level
  if (criticalityLevel == EMBER_ZCL_AMI_CRITICALITY_LEVEL_RESERVED
      || criticalityLevel > EMBER_ZCL_AMI_CRITICALITY_LEVEL_UTILITY_DEFINED6) {
    emberAfDemandResponseLoadControlClusterPrintln(
      "Reserved criticality level.  Ignoring.");
    return true;
  }

  e.destinationEndpoint = cc->apsFrame->destinationEndpoint;

  e.eventId = eventId;
  e.deviceClass = deviceClass;
  e.utilityEnrollmentGroup = utilityEnrollmentGroup;
  if (startTime == 0) {
    e.startTime = emberAfGetCurrentTime();
  } else {
    e.startTime = startTime;
  }
  e.duration = duration;
  e.criticalityLevel = criticalityLevel;
  e.coolingTempOffset = coolingTempOffset;
  e.heatingTempOffset = heatingTempOffset;
  e.coolingTempSetPoint = coolingTempSetPoint;
  e.heatingTempSetPoint = heatingTempSetPoint;
  e.avgLoadPercentage = averageLoadAdjustPercent;
  e.dutyCycle = dutyCycle;
  e.eventControl = eventControl;
  e.optionControl = EVENT_OPT_IN_DEFAULT;
  e.startRand = 0;
  e.durationRand = 0;
  e.esiBitmask = 0;

  if (eventControl & RANDOMIZE_START_TIME_FLAG) {
    e.startRand = afGetRandomizationTime(emberAfCurrentEndpoint(), true);
  }
  if (eventControl & RANDOMIZE_DURATION_TIME_FLAG) {
    e.durationRand = afGetRandomizationTime(emberAfCurrentEndpoint(), false);
  }

  emberAfDemandResponseLoadControlClusterPrintln(
    "schedule -- start: %4x, start-random: %2x duration-random: %2x",
    e.startTime,
    e.startRand,
    e.durationRand);

  emAfScheduleLoadControlEvent(emberAfCurrentEndpoint(), &e);
  return true;
}

bool emberAfDemandResponseLoadControlClusterCancelLoadControlEventCallback(
  uint32_t eventId,
  uint16_t deviceClass,
  uint8_t utilityEnrollmentGroup,
  uint8_t cancelControl,
  uint32_t effectiveTime)
{
  // FILTER ON DEVICE CLASS AND UTILITY ENROLLMENT GROUP
  if (!afMatchUegOrDeviceClass(emberAfCurrentEndpoint(),
                               utilityEnrollmentGroup,
                               deviceClass)) {
    // CCB 1380: Device must send back a default response of success, or do
    // nothing. We choose to do nothing.
    //emberAfSendDefaultResponse(emberAfCurrentCommand(), EMBER_ZCL_STATUS_NOT_FOUND);
    return true;
  }
  emAfCancelLoadControlEvent(emberAfCurrentEndpoint(),
                             eventId,
                             cancelControl,
                             effectiveTime);
  return true;
}

bool emberAfDemandResponseLoadControlClusterCancelAllLoadControlEventsCallback(
  uint8_t cancelControl)
{
  if (!emAfCancelAllLoadControlEvents(emberAfCurrentEndpoint(), cancelControl)) {
    // We didn't find any events to cancel but we were still successful in trying
    // So we return a default response of success.
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  }
  return true;
}

void emberAfEventAction(EmberAfLoadControlEvent *event,
                        uint8_t eventStatus,
                        uint8_t sequenceNumber,
                        uint8_t esiIndex)
{
  bool sendReport;
  uint8_t i;

  // Here we defer to custom callback which can do whatever it wants with
  // the even.
  sendReport = emberAfPluginDrlcEventActionCallback(event,
                                                    (EmberAfAmiEventStatus)eventStatus,
                                                    sequenceNumber);

  if (!sendReport) {
    return;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE; i++) {
    EmberAfPluginEsiManagementEsiEntry* esiEntry;
    uint8_t curIndex = (esiIndex < 0xFF) ? esiIndex : i;

    // Bit associated to this ESI not set, skip to the next one. If we are
    // sending to a specific ESI (i.e., esiIndex < 0xFF) the ESI table should
    // always contain an active entry at the passed index.
    if (esiIndex == 0xFF && (event->esiBitmask & BIT(curIndex)) == 0) {
      continue;
    }

    assert(curIndex < EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE);

    esiEntry = emberAfPluginEsiManagementEsiLookUpByIndex(curIndex);

    if (esiEntry != NULL) {
      fillReportEventData(eventStatus,
                          sequenceNumber,
                          event,
                          emberAfGetCurrentTime());

      assert(emberLookupNodeIdByEui64(esiEntry->eui64) == esiEntry->nodeId);
      emberAfResponseDestination = esiEntry->nodeId;
      emberAfResponseApsFrame.sourceEndpoint = event->destinationEndpoint;
      emberAfResponseApsFrame.destinationEndpoint = esiEntry->endpoint;

      emberAfResponseApsFrame.clusterId =
        ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID;
      emberAfSendResponse();

      // If we are sending the response to a specific ESI, we brake the for cycle
      // after we are done.
      if (esiIndex < 0xFF) {
        break;
      }
    } else {
      assert(false);
    }
  }
}

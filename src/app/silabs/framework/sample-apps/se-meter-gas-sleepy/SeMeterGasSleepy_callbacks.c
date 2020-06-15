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
 * @brief
 *******************************************************************************
   ******************************************************************************/

// *******************************************************************
// * SeMeterGas_callbacks.c
// *
// * This file contains all application specific code for the
// * gas meter sample application. This application is intended
// * to be used along with the meter mirror application
// * SeMeterMirror
// *
// * Copyright 2007 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/util/common.h"
#include "app/framework/util/client-api.h"
#include "client-command-macro.h"
#include "hal/hal.h"

#define GAS_METER_ENDPOINT 0x01
#define INVALID_MIRROR_ENDPOINT 0xFF

// Global used to track mirror endpoint
uint8_t mirrorEndpoint = INVALID_MIRROR_ENDPOINT;
uint16_t mirrorAddress = 0;

//#ifdef EMBER_TEST
//bool registrationCompleted = true;
//#else
bool registrationCompleted = false;
//#endif

// Number of seconds before trying to find a mirror
// Once connected to a network, the sleepy gas meter will
// try to find a mirror every x seconds.
#define INITIAL_STATE_EVENT_INTERVAL_SECONDS 10

// Number of seconds between mirror updates. Once a mirror
// is found, the meter will update the mirror every x seconds
#define MIRROR_UPDATE_INTERVAL_SECONDS 60

// Number of seconds between segments of the mirror
// update. During an update of the mirror, the sleepy gas meter
// will send out update segments every x seconds.
#define MIRROR_UPDATE_IN_PROGRESS_INTERVAL_SECONDS 1

#define GAS_METER_ATTRIBUTE_RECORD_BUFFER_SIZE 50
#define GAS_METER_SUPPORTED_ATTRIBUTE_COUNT 10
#define GAS_METER_SUPPORTED_ATTRIBUTES {                \
    ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID,       \
    ZCL_CURRENT_SUMMATION_RECEIVED_ATTRIBUTE_ID,        \
    ZCL_CURRENT_MAX_DEMAND_DELIVERED_ATTRIBUTE_ID,      \
    ZCL_CURRENT_MAX_DEMAND_RECEIVED_ATTRIBUTE_ID,       \
    ZCL_DFT_SUMMATION_ATTRIBUTE_ID,                     \
    ZCL_DAILY_FREEZE_TIME_ATTRIBUTE_ID,                 \
    ZCL_POWER_FACTOR_ATTRIBUTE_ID,                      \
    ZCL_READING_SNAP_SHOT_TIME_ATTRIBUTE_ID,            \
    ZCL_CURRENT_MAX_DEMAND_DELIVERED_TIME_ATTRIBUTE_ID, \
    ZCL_CURRENT_MAX_DEMAND_RECEIVED_TIME_ATTRIBUTE_ID }

uint8_t lastAttributeWritten = GAS_METER_SUPPORTED_ATTRIBUTE_COUNT;

// This enum is used by the state machine to keep track of what
// the sleepy gas meter is doing.
enum {
  INITIAL_STATE,
  SERVICE_DISCOVERY_STARTED,
  MIRROR_FOUND,
  MIRROR_UPDATE_IN_PROGRESS
};
static uint8_t state;

// Sleepy meter events, this event is included in the application event
// configuration run by the application framework. The application framework
// will allow the device to sleep until this event or others need to fire.
// This event replaces the use of the emberAfMainTickCallback which should
// not be used by sleepy devices as it does not properly allow for power
// management.
EmberEventControl sleepyMeterEventControl;
void sleepyMeterEvent(void);

// A function used to set the current state of the sleepy meter
// and schedule the timing of the next sleepy meter event. Each call to the
// sleepy meter event should result in updating of the state and thus the
// scheduling of the next event.
static void setSleepyMeterState(uint8_t s)
{
  state = s;
  switch (state) {
    case INITIAL_STATE:
      emberEventControlSetDelayQS(sleepyMeterEventControl, (INITIAL_STATE_EVENT_INTERVAL_SECONDS << 2));
      break;
    case MIRROR_FOUND:
      emberEventControlSetDelayQS(sleepyMeterEventControl, (MIRROR_UPDATE_INTERVAL_SECONDS << 2));
      break;
    case SERVICE_DISCOVERY_STARTED:
    case MIRROR_UPDATE_IN_PROGRESS:
    default:
      emberEventControlSetDelayQS(sleepyMeterEventControl, (MIRROR_UPDATE_IN_PROGRESS_INTERVAL_SECONDS << 2));
      break;
  }
}

// Service discovery callback. Passed into
// emberAfFindDevicesByProfileAndCluster below. This callback is not part of
// the application framework callbacks, it is used by the service discovery
// libraries API.
static void serviceDiscoveryCallback(const EmberAfServiceDiscoveryResult* result)
{
  if (state == SERVICE_DISCOVERY_STARTED
      && emberAfHaveDiscoveryResponseStatus(result->status)) {
    uint8_t i;
    uint8_t physAttId[] = {
      LOW_BYTE(ZCL_PHYSICAL_ENVIRONMENT_ATTRIBUTE_ID),
      HIGH_BYTE(ZCL_PHYSICAL_ENVIRONMENT_ATTRIBUTE_ID)
    };
    const EmberAfEndpointList* epList
      = (const EmberAfEndpointList*)result->responseData;

    emberAfCorePrintln("match found: 0x%2x", result->matchAddress);

    // Send a physical attribute read to each responding endpoint
    for (i = 0; i < epList->count; i++) {
      emberAfFillCommandGlobalClientToServerReadAttributes(ZCL_BASIC_CLUSTER_ID,
                                                           physAttId,
                                                           sizeof(physAttId));
      emberAfSetCommandEndpoints(GAS_METER_ENDPOINT, epList->list[i]);
      emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, result->matchAddress);
    }
    for (i = 0; i < epList->count; i++) {
      emberAfCorePrintln("  ept: %x", epList->list[i]);
    }
  } else if (result->status == EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE) {
    emberAfCorePrintln("service discovery complete.");
  }
}

// *******************************************************************
// * emberAfReadAttributeResponseCallback
// *
// * we read the physical environment during mirror discovery, if we find
// * a device on which the physical environment is set, we send a mirror
// * request.
// *******************************************************************
bool emberAfReadAttributesResponseCallback(EmberAfClusterId clusterId,
                                           uint8_t * buffer,
                                           uint16_t bufLen)
{
  if (state == SERVICE_DISCOVERY_STARTED
      && clusterId == ZCL_BASIC_CLUSTER_ID) {
    if (bufLen < 5) {
      return false;
    }
    if ((emberAfGetInt16u(buffer, 0, bufLen)
         == ZCL_PHYSICAL_ENVIRONMENT_ATTRIBUTE_ID)
        && (emberAfGetInt8u(buffer, 2, bufLen) == 0)
        && (emberAfGetInt8u(buffer, 4, bufLen) & 0x01)) {
      EmberAfClusterCommand *currentCommand = emberAfCurrentCommand();
      emberAfFillCommandSimpleMeteringClusterRequestMirror();
      emberAfSetCommandEndpoints(GAS_METER_ENDPOINT,
                                 currentCommand->apsFrame->sourceEndpoint);
      emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, currentCommand->source);
    }
  }
  return false;
}

// *******************************************************************
// * RegistrationCallback
// *
// * This callback is called by the application framework when
// * Smart Energy registration is completed. No actions should be
// * taken on the Smart Energy network until this is completed.
// *
// *
// *******************************************************************
void emberAfRegistrationCallback(bool success)
{
  setSleepyMeterState(INITIAL_STATE);
  registrationCompleted = success;
  emberAfCorePrintln("Smart Energy registration %p.",
                     (success ? "completed" : "FAILED"));
}

// *****************************************************************
// * emberAfMainInitCallback
// *
// * This is the main application initialization callback. It is called prior to
// * application initialization and gives the application a chance to take care
// * of any of its own initialization procedures.
// *
// ******************************************************************
void emberAfMainInitCallback(void)
{
  setSleepyMeterState(INITIAL_STATE);
}

// *******************************************************************
// * mainTickCallback
// *
// * Add state mechanism to seek and join a mirror
// *
// *
// *******************************************************************
void sleepyMeterEvent(void)
{
  EmberStatus status;

  // don't do anything unless we're on the network
  // and se registration is complete
  if (emberAfNetworkState() != EMBER_JOINED_NETWORK || !registrationCompleted) {
    setSleepyMeterState(INITIAL_STATE);
    return;
  }

  switch (state) {
    // We'll look for all devices that support the basic cluster. Once a device
    // responds we'll check their physical environment attribute to see if they
    // support mirroring.
    case INITIAL_STATE: {
      status = emberAfFindDevicesByProfileAndCluster(EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS,
                                                     SE_PROFILE_ID,
                                                     ZCL_BASIC_CLUSTER_ID,
                                                     EMBER_AF_SERVER_CLUSTER_DISCOVERY,
                                                     serviceDiscoveryCallback);
      if (status == EMBER_SUCCESS) {
        setSleepyMeterState(SERVICE_DISCOVERY_STARTED);
      } else {
        setSleepyMeterState(INITIAL_STATE);
      }
      break;
    }
    // Once a mirror has been found we go into a loop of updating it every
    // time this event is called.
    case MIRROR_FOUND:
    case MIRROR_UPDATE_IN_PROGRESS: {
      if (lastAttributeWritten < GAS_METER_SUPPORTED_ATTRIBUTE_COUNT) {
        uint8_t data[GAS_METER_ATTRIBUTE_RECORD_BUFFER_SIZE];
        uint8_t bufIndex = 0;
        uint16_t attributes[] = GAS_METER_SUPPORTED_ATTRIBUTES;

        while (lastAttributeWritten < GAS_METER_SUPPORTED_ATTRIBUTE_COUNT) {
          EmberAfStatus status = emberAfAppendAttributeReportFields(GAS_METER_ENDPOINT,
                                                                    ZCL_SIMPLE_METERING_CLUSTER_ID,
                                                                    attributes[lastAttributeWritten],
                                                                    CLUSTER_MASK_SERVER,
                                                                    data,
                                                                    GAS_METER_ATTRIBUTE_RECORD_BUFFER_SIZE,
                                                                    &bufIndex);
          if (status != EMBER_ZCL_STATUS_SUCCESS) {
            break;
          }
          lastAttributeWritten++;
        }
        emberAfFillCommandGlobalServerToClientReportAttributes(ZCL_SIMPLE_METERING_CLUSTER_ID,
                                                               data,
                                                               bufIndex);
        emberAfSetCommandEndpoints(GAS_METER_ENDPOINT, mirrorEndpoint);
        emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, mirrorAddress);
      }

      if (lastAttributeWritten < GAS_METER_SUPPORTED_ATTRIBUTE_COUNT) {
        setSleepyMeterState(MIRROR_UPDATE_IN_PROGRESS);
      } else {
        // Reset the last att written, this will start writing
        // of attributes to the mirror until they are all written.
        lastAttributeWritten = 0;
        setSleepyMeterState(MIRROR_FOUND);
      }
      break;
    }
    // If we are in service discovery, we are looking for a meter mirror
    // and are essentially in a holding pattern until one is found.
    case SERVICE_DISCOVERY_STARTED:
    default:
      setSleepyMeterState(state); //maintain current state
  }
}

// *******************************************************************
// * clusterInitCallback
// *
// * Make sure that the simple metering device type on endpoint 1
// * is set to gas meter. We could do this in the main init too but
// * it is a little bit cleaner here, provides more code separation.
// *
// *
// *******************************************************************
void emberAfClusterInitCallback(uint8_t endpoint, EmberAfClusterId clusterId)
{
  if (endpoint == GAS_METER_ENDPOINT
      && clusterId == ZCL_SIMPLE_METERING_CLUSTER_ID) {
    uint8_t deviceType = EMBER_ZCL_METERING_DEVICE_TYPE_GAS_METERING;
    emberAfWriteAttribute(endpoint,
                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                          ZCL_METERING_DEVICE_TYPE_ATTRIBUTE_ID,
                          CLUSTER_MASK_SERVER,
                          (uint8_t *)&deviceType,
                          ZCL_INT8U_ATTRIBUTE_TYPE);
  }
}

// *******************************************************************
// * simpleMeteringClusterMirrorRemovedCallback
// *
// * This callback simply prints out the endpoint from which
// * the mirror was removed, and sets our state back to looking for
// * a new mirror
// *
// *******************************************************************
bool emberAfSimpleMeteringClusterMirrorRemovedCallback(uint16_t endpointId)
{
  if (endpointId == 0xffff) {
    emberAfCorePrintln("Mirror remove FAILED");
  } else {
    emberAfCorePrintln("Mirror REMOVED from %x", endpointId);
    setSleepyMeterState(INITIAL_STATE);
  }
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

// *******************************************************************
// * simpleMeteringClusterRequestMirrorResponseCallback
// *
// * This callback simply prints out the endpoint to which the
// * mirror was added.
// *
// * A more complex implementation of this sample
// * application would start writing attributes by sending
// * global attribute writes to the mirrored endpoint.
// * I have not implemented a mechanism for
// * this and instead write the attributes using the cli commands
// * such as:
// *
// * zcl global write 0x0702 0x0306 0x18 {04}
// * send 0 1 3
// *
// *******************************************************************
bool emberAfSimpleMeteringClusterRequestMirrorResponseCallback(uint16_t endpointId)
{
  if (endpointId == 0xffff) {
    emberAfCorePrintln("Mirror add FAILED");
  } else {
    if (state != MIRROR_FOUND) {
      mirrorEndpoint = endpointId;
      mirrorAddress = emberAfCurrentCommand()->source;
      emberAfCorePrintln("Mirror ADDED on 0x%2x, 0x%x", mirrorAddress, endpointId);
      setSleepyMeterState(MIRROR_FOUND);
    } else {
      emberAfCorePrintln("Mirror add for 0x%2x, 0x%x ignored, already mirrored on 0x%2x 0x%x.",
                         emberAfCurrentCommand()->source, endpointId,
                         mirrorAddress, mirrorEndpoint);
    }
  }
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

/** @brief Ok To Sleep
 *
 * This function is called by the Idle/Sleep plugin before sleeping.  The
 * application should return true if the device may sleep or false otherwise.
 *
 * @param durationMs The maximum duration in milliseconds that the device will
 * sleep.  Ver.: always
 */
bool emberAfPluginIdleSleepOkToSleepCallback(uint32_t durationMs)
{
  return true;
}

/** @brief Wake Up
 *
 * This function is called by the Idle/Sleep plugin after sleeping.
 *
 * @param durationMs The duration in milliseconds that the device slept.
 * Ver.: always
 */
void emberAfPluginIdleSleepWakeUpCallback(uint32_t durationMs)
{
}

/** @brief Ok To Idle
 *
 * This function is called by the Idle/Sleep plugin before idling.  The
 * application should return true if the device may idle or false otherwise.
 *
 */
bool emberAfPluginIdleSleepOkToIdleCallback(void)
{
  return true;
}

/** @brief Active
 *
 * This function is called by the Idle/Sleep plugin after idling.
 *
 */
void emberAfPluginIdleSleepActiveCallback(void)
{
}

/** @brief Finished
 *
 * This callback is fired when the network-find plugin is finished with the
 * forming or joining process.  The result of the operation will be returned
 * in the status parameter.
 *
 * @param status   Ver.: always
 */
void emberAfPluginNetworkFindFinishedCallback(EmberStatus status)
{
}

/** @brief Join
 *
 * This callback is called by the plugin when a joinable network has been
 * found.  If the application returns true, the plugin will attempt to join
 * the network.  Otherwise, the plugin will ignore the network and continue
 * searching.  Applications can use this callback to implement a network
 * blacklist.
 *
 * @param networkFound   Ver.: always
 * @param lqi   Ver.: always
 * @param rssi   Ver.: always
 */
bool emberAfPluginNetworkFindJoinCallback(EmberZigbeeNetwork *networkFound,
                                          uint8_t lqi,
                                          int8_t rssi)
{
  return true;
}

/** @brief Poll Completed
 *
 * This function is called by the application framework after a poll is
 * completed.
 *
 * @param status Return status of a completed poll operation  Ver.: always
 */
void emberAfPluginEndDeviceSupportPollCompletedCallback(EmberStatus status)
{
}

/** @brief Process Notification Flags
 *
 * This function is called by the metering server plugin when any of the
 * Notification Attribute Set attributes are read or reported by the metering
 * client.
 *
 * @param attributeId   Ver.: always
 * @param attributeValue   Ver.: always
 */
void emberAfPluginSimpleMeteringServerProcessNotificationFlagsCallback(uint16_t attributeId,
                                                                       uint32_t attributeValue)
{
}

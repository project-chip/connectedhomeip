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
 * @brief Code to handle being a mirror for a sleepy meter.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/util.h"
#include "meter-mirror.h"

//----------------------------------------------------------------------------
// Globals

#define HAVE_MIRROR_CAPACITY 0x01
#ifndef EMBER_AF_GBCS_COMPATIBLE
#define NO_MIRROR_CAPACITY   0x00       // never used under GBCS
#endif

// Although technically endpoints are 8-bit, the returned value to the
// Mirror request and Mirror Remove commands are 16-bit.  Therefore
// we will use a 16-bit value to indicate an invalid endpoint.
#define INVALID_MIRROR_ENDPOINT 0xFFFF

#define INVALID_INDEX 0xFF

EmberEUI64 nullEui64 = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

#define MIRROR_NOT_FOUND_ZCL_RETURN_CODE 0xFFFF

// Each record in the report has a two-byte attribute id, a one-byte type, and
// variable-length data.
#define ATTRIBUTE_OVERHEAD 3

static bool sendMirrorReportAttributeResponse(uint8_t endpoint, uint8_t index);
static uint8_t findMirrorIndex(EmberEUI64 requestingDeviceIeeeAddress);

// This is NOT stored across reboots.  Future versions of the plugin will need
// support to persistently store this.
typedef struct {
  uint8_t eui64[EUI64_SIZE];
  uint32_t issuerEventId;
  uint32_t reportingInterval;
  uint8_t mirrorNotificationReporting;
  uint8_t notificationScheme;
} MirrorEntry;

static MirrorEntry mirrorList[EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS];

//-----------------------------------------------------------------------------
// Functions

uint8_t emAfPluginMeterMirrorGetMirrorsAllocated(void)
{
  uint8_t mirrors = 0;
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS; i++) {
    if (0 != MEMCOMPARE(mirrorList[i].eui64, nullEui64, EUI64_SIZE)) {
      mirrors++;
    }
  }
  return mirrors;
}

static void updatePhysicalEnvironment(void)
{
  uint8_t physEnv =
#ifdef EMBER_AF_GBCS_COMPATIBLE
    HAVE_MIRROR_CAPACITY;
#else
    (emAfPluginMeterMirrorGetMirrorsAllocated() < EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS
     ? HAVE_MIRROR_CAPACITY
     : NO_MIRROR_CAPACITY);
#endif

  EmberAfStatus status
    = emberAfWriteServerAttribute(EMBER_AF_PLUGIN_METER_MIRROR_METER_CLIENT_ENDPOINT,
                                  ZCL_BASIC_CLUSTER_ID,
                                  ZCL_PHYSICAL_ENVIRONMENT_ATTRIBUTE_ID,
                                  &physEnv,
                                  ZCL_ENUM8_ATTRIBUTE_TYPE);
  if (status) {
    emberAfSimpleMeteringClusterPrintln("Error:  Could not update mirror capacity status %u.",
                                        status);
  }
}

void emClearMirrorByEndpoint(uint8_t endpoint)
{
}

static uint8_t getIndexFromEndpoint(uint8_t endpoint)
{
  if (endpoint < EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START
      ||  (endpoint >= (EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START
                        + EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS))) {
    return INVALID_INDEX;
  }
  return (endpoint - EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START);
}

static void clearMirrorByIndex(uint8_t index, bool print)
{
  MEMSET(mirrorList[index].eui64, 0xFF, EUI64_SIZE);

  if (print) {
    emberAfSimpleMeteringClusterPrintln("Removed meter mirror at endpoint %d",
                                        index + EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START);
  }
//  emberAfEndpointEnableDisable(endpoint,false);
  updatePhysicalEnvironment();
}

static void clearAllMirrors(bool print)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS; i++) {
    clearMirrorByIndex(i, print);
  }
}

bool emberAfPluginMeterMirrorGetEui64ByEndpoint(uint8_t endpoint,
                                                EmberEUI64 returnEui64)
{
  uint8_t index = getIndexFromEndpoint(endpoint);
  if (index == INVALID_INDEX) {
    return false;
  }
  MEMMOVE(returnEui64, mirrorList[index].eui64, EUI64_SIZE);
  return true;
}

bool emberAfPluginMeterMirrorGetEndpointByEui64(EmberEUI64 eui64,
                                                uint8_t *returnEndpoint)
{
  uint8_t index = findMirrorIndex(eui64);
  if (index == INVALID_INDEX) {
    return false;
  }
  *returnEndpoint = EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START + index;
  return true;
}

void emberAfPluginMeterMirrorInitCallback(void)
{
  // disable all mirror endpoints
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS; i++) {
    emberAfEndpointEnableDisable(EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START + i,
                                 false);
  }

  // clear internal list of meter mirror EUIs/update phy env
  clearAllMirrors(false);
}

bool emberAfPluginMeterMirrorIsMirrorUsed(uint8_t endpoint)
{
  uint8_t index = getIndexFromEndpoint(endpoint);
  if (INVALID_INDEX == index) {
    emberAfSimpleMeteringClusterPrintln("Error:  Endpoint %d is not a valid mirror endpoint.",
                                        endpoint);
    return false;
  }
  return (0 != MEMCOMPARE(mirrorList[index].eui64, nullEui64, EUI64_SIZE));
}

void emberAfPluginMeterMirrorStackStatusCallback(EmberStatus status)
{
  if (status == EMBER_NETWORK_DOWN
      && !emberStackIsPerformingRejoin()) {
    emberAfSimpleMeteringClusterPrintln("Re-initializing mirrors due to stack down.");
    clearAllMirrors(true);  // print?
  }
}

static uint8_t findMirrorIndex(EmberEUI64 requestingDeviceIeeeAddress)
{
  uint8_t i;

  for (i = 0; i < EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS; i++) {
    if (0 == MEMCOMPARE(requestingDeviceIeeeAddress, mirrorList[i].eui64, EUI64_SIZE)) {
      return i;
    }
  }
  return INVALID_INDEX;
}

uint16_t emberAfPluginSimpleMeteringClientRequestMirrorCallback(EmberEUI64 requestingDeviceIeeeAddress)
{
  return emberAfPluginMeterMirrorRequestMirror(requestingDeviceIeeeAddress);
}

uint16_t emberAfPluginMeterMirrorRequestMirror(EmberEUI64 requestingDeviceIeeeAddress)
{
  uint8_t meteringDeviceTypeAttribute =  EMBER_ZCL_METERING_DEVICE_TYPE_UNDEFINED_MIRROR_METER;
  uint8_t index = findMirrorIndex(requestingDeviceIeeeAddress);
  uint8_t endpoint = EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START;
  EmberStatus status;

  if (emAfPluginMeterMirrorGetMirrorsAllocated() >= EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS
      && index == INVALID_INDEX) {
    return INVALID_MIRROR_ENDPOINT;
  }

  if (index != INVALID_INDEX) {
    endpoint += index;
    emberAfSimpleMeteringClusterPrintln("Mirror already allocated on endpoint %d",
                                        endpoint);
    return endpoint;
  }

  index = findMirrorIndex(nullEui64);
  if (index == INVALID_INDEX) {
    emberAfSimpleMeteringClusterPrintln("No free mirror endpoints for new mirror.\n");
    return INVALID_MIRROR_ENDPOINT;
  }

  endpoint += index;
  MEMCOPY(mirrorList[index].eui64, requestingDeviceIeeeAddress, EUI64_SIZE);
  mirrorList[index].issuerEventId = 0;
  mirrorList[index].reportingInterval = 0;
  mirrorList[index].mirrorNotificationReporting = false;
  mirrorList[index].notificationScheme = EMBER_ZCL_NOTIFICATION_SCHEME_NO_NOTIFICATION_SCHEME_DEFINED;
  emberAfEndpointEnableDisable(endpoint, true);
  emberAfSimpleMeteringClusterPrint("Registered new meter mirror: ");
  emberAfPrintBigEndianEui64(requestingDeviceIeeeAddress);
  emberAfSimpleMeteringClusterPrintln(".  Endpoint: %d", endpoint);
  updatePhysicalEnvironment();

  status
    = emberAfWriteServerAttribute(endpoint,
                                  ZCL_SIMPLE_METERING_CLUSTER_ID,
                                  ZCL_METERING_DEVICE_TYPE_ATTRIBUTE_ID,
                                  (uint8_t*) &meteringDeviceTypeAttribute,
                                  ZCL_BITMAP8_ATTRIBUTE_TYPE);
  if (status) {
    emberAfSimpleMeteringClusterPrintln("Failed to write Metering device type attribute on ep%d, status :  0x%x",
                                        endpoint,
                                        status);
  }

  emberAfPluginMeterMirrorMirrorAddedCallback(requestingDeviceIeeeAddress, endpoint);
  return endpoint;
}

bool emberAfSimpleMeteringClusterConfigureMirrorCallback(uint32_t issuerEventId,
                                                         uint32_t reportingInterval,
                                                         uint8_t mirrorNotificationReporting,
                                                         uint8_t notificationScheme)
{
  EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
  uint8_t endpoint = emberAfCurrentEndpoint();
  EmberEUI64 sendersEui;
  uint8_t index;

  emberAfSimpleMeteringClusterPrintln("ConfigureMirror on endpoint 0x%x", endpoint);

  if (EMBER_SUCCESS != emberLookupEui64ByNodeId(emberAfCurrentCommand()->source, sendersEui)) {
    emberAfSimpleMeteringClusterPrintln("Error: Meter Mirror plugin cannot determine EUI64 for node ID 0x%2X",
                                        emberAfCurrentCommand()->source);
    status = EMBER_ZCL_STATUS_FAILURE;
    goto kickout;
  }

  index = findMirrorIndex(sendersEui);
  if (index == INVALID_INDEX) {
    emberAfSimpleMeteringClusterPrint("Error: Meter mirror plugin received unknown report from ");
    emberAfPrintBigEndianEui64(sendersEui);
    emberAfSimpleMeteringClusterPrintln("");
    status = EMBER_ZCL_STATUS_NOT_AUTHORIZED;
    goto kickout;
  }

  if (mirrorList[index].issuerEventId == 0
      || issuerEventId > mirrorList[index].issuerEventId) {
    if (notificationScheme > 0x02) {
      status = EMBER_ZCL_STATUS_INVALID_FIELD;
      goto kickout;
    }

    mirrorList[index].issuerEventId = issuerEventId;
    mirrorList[index].reportingInterval = reportingInterval;
    mirrorList[index].mirrorNotificationReporting = mirrorNotificationReporting;
    mirrorList[index].notificationScheme = notificationScheme;
  }

  kickout:
  emberAfSendImmediateDefaultResponse(status);
  return true;
}

uint16_t emberAfPluginSimpleMeteringClientRemoveMirrorCallback(EmberEUI64 requestingDeviceIeeeAddress)
{
  return emberAfPluginMeterMirrorRemoveMirror(requestingDeviceIeeeAddress);
}

uint16_t emberAfPluginMeterMirrorRemoveMirror(EmberEUI64 requestingDeviceIeeeAddress)
{
  uint8_t index;
  uint8_t endpoint;

  if (0 == MEMCOMPARE(nullEui64, requestingDeviceIeeeAddress, EUI64_SIZE)) {
    emberAfSimpleMeteringClusterPrintln("Rejecting mirror removal using NULL EUI64");
    return INVALID_MIRROR_ENDPOINT;
  }

  index = findMirrorIndex(requestingDeviceIeeeAddress);

  if (index == INVALID_INDEX) {
    emberAfSimpleMeteringClusterPrint("Unknown mirror for remove: ");
    emberAfPrintBigEndianEui64(requestingDeviceIeeeAddress);
    emberAfSimpleMeteringClusterPrintln(" ");
    return MIRROR_NOT_FOUND_ZCL_RETURN_CODE;
  }

  clearMirrorByIndex(index,
                     true);  // print?

  endpoint = index + EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START;
  emberAfPluginMeterMirrorMirrorRemovedCallback(requestingDeviceIeeeAddress, endpoint);
  return endpoint;
}

bool emberAfReportAttributesCallback(EmberAfClusterId clusterId,
                                     uint8_t *buffer,
                                     uint16_t bufLen)
{
  EmberEUI64 sendersEui;
  uint16_t bufIndex = 0;
  uint8_t endpoint;
  uint8_t index;
  bool attributeReportingComplete = false;

  if (EMBER_SUCCESS
      != emberLookupEui64ByNodeId(emberAfCurrentCommand()->source, sendersEui)) {
    emberAfSimpleMeteringClusterPrintln("Error: Meter Mirror plugin cannot determine EUI64 for node ID 0x%2X",
                                        emberAfCurrentCommand()->source);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    return true;
  }

  if (emberAfCurrentCommand()->direction
      == ZCL_DIRECTION_CLIENT_TO_SERVER) {
    emberAfSimpleMeteringClusterPrintln("Error:  Meter Mirror Plugin does not accept client to server attributes.\n",
                                        emberAfCurrentCommand()->direction);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    return true;
  }

  index = findMirrorIndex(sendersEui);
  if (index == INVALID_INDEX) {
    emberAfSimpleMeteringClusterPrint("Error: Meter mirror plugin received unknown report from ");
    emberAfPrintBigEndianEui64(sendersEui);
    emberAfSimpleMeteringClusterPrintln("");
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_AUTHORIZED);
    return true;
  }

  if (emberAfCurrentCommand()->mfgSpecific) {
    // Here is where we could handle a MFG specific Report attributes and interpret
    // it.  This code does not do that, just politely returns an error.
    emberAfSimpleMeteringClusterPrintln("Error: Unknown MFG Code for mirror: 0x%2X",
                                        emberAfCurrentCommand()->mfgCode);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_UNSUP_MANUF_GENERAL_COMMAND);
    return true;
  }

  endpoint = (index + EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START);
  while (bufIndex + ATTRIBUTE_OVERHEAD < bufLen) {
    EmberAfStatus status;
    EmberAfAttributeId attributeId;
    EmberAfAttributeType dataType;
    uint16_t dataSize;

    attributeId = (EmberAfAttributeId)emberAfGetInt16u(buffer,
                                                       bufIndex,
                                                       bufLen);
    bufIndex += 2;
    dataType = (EmberAfAttributeType)emberAfGetInt8u(buffer, bufIndex, bufLen);
    bufIndex++;

    dataSize = emberAfAttributeValueSize(dataType, buffer + bufIndex);

#if (BIGENDIAN_CPU)
    if (dataSize <= bufLen - bufIndex && dataSize <= ATTRIBUTE_LARGEST) {
      uint8_t data[ATTRIBUTE_LARGEST];
      if (isThisDataTypeSentLittleEndianOTA(dataType)) {
        emberReverseMemCopy(data, buffer + bufIndex, dataSize);
      } else {
        MEMMOVE(data, buffer + bufIndex, dataSize);
      }
#else
    if (dataSize <= bufLen - bufIndex) {
      uint8_t *data = buffer + bufIndex;
#endif

      if (attributeId == ZCL_SIMPLE_METERING_CLUSTER_REPORTING_STATUS_SERVER_ATTRIBUTE_ID) {
        // From the SE 1.2a Specification within the ConfigureMirror command
        // description..
        // 1.  On powering up, the BOMD will send one or more Report Attribute
        //     commands to the Metering client on the mirror endpoint. The last
        //     attribute to be reported to the mirror shall be an Attribute
        //     Reporting Status attribute, as defined in section A.2.
        // 2.  If MirrorReportAttributeResponse is enabled, the server does not
        //     need to request an APS ACK. If the server requests an APS ACK,
        //     the Metering client on the mirror endpoint shall respond first
        //     with an APS ACK and then send the MirrorReportAttributeResponse.
        //
        // If Mirror Notification Reporting is set to false, the
        // MirrorReportAttributeResponse command shall not be enabled; the
        // Metering server may poll the Notification flags by means of a normal
        // ReadAttribute command, as shown in Figure D 29:
        attributeReportingComplete = (data[0] == EMBER_ZCL_ATTRIBUTE_REPORTING_STATUS_ATTRIBUTE_REPORTING_COMPLETE);
        status = EMBER_ZCL_STATUS_SUCCESS;
      } else {
        if (attributeId == ZCL_METERING_DEVICE_TYPE_ATTRIBUTE_ID
            && data[0] < EMBER_ZCL_METERING_DEVICE_TYPE_MIRRORED_ELECTRIC_METERING) {
          data[0] += 127;
        }

        status = emberAfWriteServerAttribute(endpoint,
                                             clusterId,
                                             attributeId,
                                             data,
                                             dataType);
      }

      emberAfSimpleMeteringClusterPrintln("Mirror attribute 0x%2x: 0x%x", attributeId, status);
      bufIndex += dataSize;
    } else {
      // dataSize exceeds buffer length, terminate loop
      emberAfSimpleMeteringClusterPrintln("ERR: attr:%2x size %d exceeds buffer size", attributeId, dataSize);
      break;
    }
  }

  // If reporting is complete then callback to the application so that if it needs
  // to do any post processing on the reported attributes it can do it now.
  if (attributeReportingComplete) {
    emberAfPluginMeterMirrorReportingCompleteCallback(endpoint);
  }

  // Notification flags
  emberAfSimpleMeteringClusterPrintln("Mirror reporting ep: 0x%x, reporting: 0x%x, scheme: 0x%x",
                                      endpoint,
                                      mirrorList[index].mirrorNotificationReporting,
                                      mirrorList[index].notificationScheme);
  if (mirrorList[index].mirrorNotificationReporting  && attributeReportingComplete) {
    if (mirrorList[index].notificationScheme == EMBER_ZCL_NOTIFICATION_SCHEME_PREDEFINED_NOTIFICATION_SCHEME_A
        || mirrorList[index].notificationScheme == EMBER_ZCL_NOTIFICATION_SCHEME_PREDEFINED_NOTIFICATION_SCHEME_B) {
      return sendMirrorReportAttributeResponse(endpoint, index);
    } else {
      // TODO: for custom notification schemes callback to application
      // return emberAfMeterMirrorSendMirrorReportAttributeResponseCallback(...)
    }
  }

  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

static bool sendMirrorReportAttributeResponse(uint8_t endpoint, uint8_t index)
{
  uint8_t status;
  uint8_t notificationFlagsBuffer[20];
  uint8_t flagsBufferLen = 0;
  uint16_t attrId;
  uint32_t readFlag;

  // Predefined Notification Scheme B requires reporting the following
  // notification flags:
  //   FUNCTIONAL_NOTIFICATION_FLAGS
  status = emberAfReadClientAttribute(endpoint,
                                      ZCL_SIMPLE_METERING_CLUSTER_ID,
                                      ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID,
                                      (uint8_t *)&readFlag,
                                      sizeof(uint32_t));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    MEMSET(notificationFlagsBuffer, 0, sizeof(uint32_t));
  } else {
    emberAfCopyInt32u(notificationFlagsBuffer, flagsBufferLen, readFlag);
  }
  flagsBufferLen += 4;

  // Predefined Notification Scheme A prescribes only reporting the functional
  // notification flags
  if (mirrorList[index].notificationScheme == EMBER_ZCL_NOTIFICATION_SCHEME_PREDEFINED_NOTIFICATION_SCHEME_A) {
    goto kickout;
  }

  // Predefined Notification Scheme B requires reporting the following
  // notification flags:
  //   FUNCTIONAL_NOTIFICATION_FLAGS
  //   NOTIFICATION_FLAGS_2
  //   NOTIFICATION_FLAGS_3
  //   NOTIFICATION_FLAGS_4
  //   NOTIFICATION_FLAGS_5
  for (attrId = ZCL_NOTIFICATION_FLAGS_2_ATTRIBUTE_ID;
       attrId < ZCL_NOTIFICATION_FLAGS_6_ATTRIBUTE_ID;
       attrId++) {
    status = emberAfReadClientAttribute(endpoint,
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        attrId,
                                        (uint8_t *)&readFlag,
                                        sizeof(uint32_t));
    if (status != EMBER_ZCL_STATUS_SUCCESS) {
      MEMSET(&notificationFlagsBuffer[flagsBufferLen], 0, sizeof(uint32_t));
    } else {
      emberAfCopyInt32u(notificationFlagsBuffer, flagsBufferLen, readFlag);
    }
    flagsBufferLen += 4;
  }

  kickout:
  // Populate the command buffer
  emberAfFillCommandSimpleMeteringClusterMirrorReportAttributeResponse(mirrorList[index].notificationScheme,
                                                                       (uint8_t *) notificationFlagsBuffer,
                                                                       flagsBufferLen);
  emberAfSimpleMeteringClusterPrintln("Mirror report attribute response buffer populated");
  emberAfSendResponse();
  return true;
}

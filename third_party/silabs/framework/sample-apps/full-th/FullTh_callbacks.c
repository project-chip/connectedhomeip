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
 * @brief
 *******************************************************************************
   ******************************************************************************/

// Copyright 2007 - 2011 by Ember Corporation. All rights reserved.
//
//

// This callback file is created for your convenience. You may add application code
// to this file. If you regenerate this file over a previous version, the previous
// version will be overwritten and any code you have added will be lost.

#include "app/framework/include/af.h"
#include "app/framework/plugin/reporting/reporting.h"
#include "app/util/serial/command-interpreter2.h"

void resetSystemTokens(void); // This is a HAL function that we will use.

/** @brief Allow Network Write Attribute
 *
 * This function is called by the application framework before it writes an
 * attribute in response to a write attribute request from an external device.
 * The value passed into this callback is the value to which the attribute is
 * to be set by the framework.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 * @param attributeId   Ver.: always
 * @param mask   Ver.: always
 * @param value   Ver.: always
 * @param type   Ver.: always
 */
EmberAfAttributeWritePermission emberAfAllowNetworkWriteAttributeCallback(uint8_t endpoint,
                                                                          EmberAfClusterId clusterId,
                                                                          EmberAfAttributeId attributeId,
                                                                          uint8_t mask,
                                                                          uint16_t manufacturerCode,
                                                                          uint8_t* value,
                                                                          uint8_t type)
{
  return EMBER_ZCL_ATTRIBUTE_WRITE_PERMISSION_ALLOW_WRITE_NORMAL; // Default
}

/** @brief Pre Attribute Change
 *
 * This function is called by the application framework before it changes an
 * attribute value. The value passed into this callback is the value to which
 * the attribute is to be set by the framework.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 * @param attributeId   Ver.: always
 * @param mask   Ver.: always
 * @param type   Ver.: always
 * @param size   Ver.: always
 * @param value   Ver.: always
 */
EmberAfStatus emberAfPreAttributeChangeCallback(uint8_t endpoint,
                                                EmberAfClusterId clusterId,
                                                EmberAfAttributeId attributeId,
                                                uint8_t mask,
                                                uint16_t manufacturerCode,
                                                uint8_t type,
                                                uint8_t size,
                                                uint8_t* value)
{
  return EMBER_ZCL_STATUS_SUCCESS;
}

/** @brief Post Attribute Change
 *
 * This function is called by the application framework after it changes an
 * attribute value. The value passed into this callback is the value to which
 * the attribute was set by the framework.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 * @param attributeId   Ver.: always
 * @param mask   Ver.: always
 * @param type   Ver.: always
 * @param size   Ver.: always
 * @param value   Ver.: always
 */
void emberAfPostAttributeChangeCallback(uint8_t endpoint,
                                        EmberAfClusterId clusterId,
                                        EmberAfAttributeId attributeId,
                                        uint8_t mask,
                                        uint16_t manufacturerCode,
                                        uint8_t type,
                                        uint8_t size,
                                        uint8_t* value)
{
  // This is a special unit test for the manufacturer name,
  // see bug 12029
  if (endpoint == 1
      && clusterId == ZCL_BASIC_CLUSTER_ID
      && attributeId == ZCL_MANUFACTURER_NAME_ATTRIBUTE_ID
      && mask == CLUSTER_MASK_SERVER
      && manufacturerCode == EMBER_AF_NULL_MANUFACTURER_CODE) {
    uint8_t status = AFV2_MACRO_TEST;
    uint8_t targetShort[6]; // This should contain "Hello"
    uint8_t targetLong[33]; // This contains whole attribute
    // Manufacturer name attribute
    emberAfCustom1Print("Manufacturer name changed to:");
    emberAfCustom1PrintString(value);
    emberAfCustom1Println("");
    emberAfCustom1Flush();

    status = emberAfReadServerAttribute(1,
                                        ZCL_BASIC_CLUSTER_ID,
                                        ZCL_MANUFACTURER_NAME_ATTRIBUTE_ID,
                                        targetShort,
                                        sizeof(targetShort));
    emberAfCustom1Print("Read attribute,short buffer,status=%x:", status);
    if ( status == EMBER_ZCL_STATUS_SUCCESS ) {
      emberAfCustom1PrintString(targetShort);
      emberAfCustom1Println("");
    } else {
      emberAfCustom1Println("---ERROR---");
    }

    status = emberAfReadServerAttribute(1,
                                        ZCL_BASIC_CLUSTER_ID,
                                        ZCL_MANUFACTURER_NAME_ATTRIBUTE_ID,
                                        targetLong,
                                        sizeof(targetLong));
    emberAfCustom1Print("Read attribute,long buffer,status=%x:", status);
    if ( status == EMBER_ZCL_STATUS_SUCCESS ) {
      emberAfCustom1PrintString(targetLong);
      emberAfCustom1Println("");
    } else {
      emberAfCustom1Println("---ERROR---");
    }
  }
}

/** @brief Pre Message Received
 *
 * This function is called by the application framework when a raw message is
 * received but has not yet been parsed by the application framework. This
 * function should return a bool value indicating whether the message has
 * been handled by the user application code and should be ignored by the
 * AppFramework. By default, this callback returns false indicating that the
 * message has not been handled and should be handled by the AppFramework.
 * This function receives a pointer to a struct containing the super set of
 * arguments sent to the emberIncomingMessageHandler and the
 * ezspIncomingMessageHandler.
 *
 * @param incomingMessage   Ver.: always
 */
bool emberAfPreMessageReceivedCallback(EmberAfIncomingMessage* incomingMessage)
{
  return false;
}

/** @brief Pre Command Received
 *
 * This function is called by the application framework when a command is
 * received but has yet to be handled by the framework code. This function
 * should return a bool value indicating whether the command has been
 * handled by the user application code and should be ignored by the
 * AppFramework. By default, this callback returns false indicating that the
 * message has not been handled and should be handled by the
 *
 * @param cmd   Ver.: always
 */
bool emberAfPreCommandReceivedCallback(EmberAfClusterCommand* cmd)
{
  return false;
}

/** @brief Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives
 * the application an opportunity to take care of cluster initialization
 * procedures. It is called exactly once for each endpoint where cluster is
 * present.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 */
void emberAfClusterInitCallback(uint8_t endpoint,
                                EmberAfClusterId clusterId)
{
}

/** @brief Main Tick
 *
 * Whenever main application tick is called, this callback will be called at
 * the end of the main tick execution.
 *
 */
void emberAfMainTickCallback(void)
{
}

/** @brief External Attribute Write
 *
 * This callback is called for whenever an attribute is written, if this
 * attribute was tagged to be stored externally. Function should return
 * EMBER_ZCL_STATUS_SUCCESS if write was successful and
 * EMBER_ZCL_STATUS_FAILURE if not. Upon successful exit, the data
 * from the passed buffer should be stored as the current value of the
 * attribute. Important to note is, that if this method ever returns false,
 * then it is possible to break contract of the write-undivided-attributes
 * global command. That command checks for ranges and attribute existence, but
 * if this method returns false, when the attribute is finally written, then
 * the contract of writing all or no attributes will be broken.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 * @param attributeMetadata   Ver.: always
 * @param buffer   Ver.: always
 */
EmberAfStatus emberAfExternalAttributeWriteCallback(uint8_t endpoint,
                                                    EmberAfClusterId clusterId,
                                                    EmberAfAttributeMetadata * attributeMetadata,
                                                    uint16_t manufacturerCode,
                                                    uint8_t * buffer)
{
  return EMBER_ZCL_STATUS_FAILURE;
}

/** @brief External Attribute Read
 *
 * If an attribute is tagged to be stored externally, this function will be
 * called whenever that attribute is read. This function should return
 * EMBER_ZCL_STATUS_SUCCESS if read was successful and
 * EMBER_ZCL_STATUS_FAILURE if not. The passed buffer should be populated
 * with the associated attributes value.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 * @param attributeMetadata   Ver.: always
 * @param buffer   Ver.: always
 */
EmberAfStatus emberAfExternalAttributeReadCallback(uint8_t endpoint,
                                                   EmberAfClusterId clusterId,
                                                   EmberAfAttributeMetadata * attributeMetadata,
                                                   uint16_t manufacturerCode,
                                                   uint8_t * buffer,
                                                   uint16_t maxReadLength)
{
  return EMBER_ZCL_STATUS_FAILURE;
}

/** @brief Trust Center Join
 *
 * This callback is called from within the application framework's
 * implementation of emberTrustCenterJoinHandler or
 * ezspTrustCenterJoinHandler. This callback provides the same arguments
 * passed to the TrustCenterJoinHandler. For more information about the
 * TrustCenterJoinHandler please see documentation included in
 * stack/include/trust-center.h.
 *
 * @param newNodeId   Ver.: always
 * @param newNodeEui64   Ver.: always
 * @param parentOfNewNode   Ver.: always
 * @param status   Ver.: always
 * @param decision   Ver.: always
 */
void emberAfTrustCenterJoinCallback(EmberNodeId newNodeId,
                                    EmberEUI64 newNodeEui64,
                                    EmberNodeId parentOfNewNode,
                                    EmberDeviceUpdate status,
                                    EmberJoinDecision decision)
{
}

/** @brief Request Mirror Response
 *
 *
 * @param endpointId   Ver.: always
 */
bool emberAfSimpleMeteringClusterRequestMirrorResponseCallback(uint16_t endpointId)
{
  return false;
}

/** @brief Mirror Removed
 *
 *
 * @param endpointId   Ver.: always
 */
bool emberAfSimpleMeteringClusterMirrorRemovedCallback(uint16_t endpointId)
{
  return false;
}

/** @brief Identify Query Response
 *
 *
 * @param timeout   Ver.: always
 */
bool emberAfIdentifyClusterIdentifyQueryResponseCallback(uint16_t timeout)
{
  return false;
}

/** @brief Zone Status Change Notification
 *
 *
 * @param zoneStatus   Ver.: always
 * @param extendedStatus   Ver.: always
 * @param zoneId   Ver.: since ha-1.2-11-5474-19
 * @param delay   Ver.: since ha-1.2-11-5474-19
 */
bool emberAfIasZoneClusterZoneStatusChangeNotificationCallback(uint16_t zoneStatus,
                                                               uint8_t extendedStatus,
                                                               uint8_t zoneId,
                                                               uint16_t delay)
{
  return false;
}

/** @brief Lock Door Response
 *
 *
 * @param status   Ver.: always
 */
bool emberAfDoorLockClusterLockDoorResponseCallback(uint8_t status)
{
  return false;
}

/** @brief Unlock Door Response
 *
 *
 * @param status   Ver.: always
 */
bool emberAfDoorLockClusterUnlockDoorResponseCallback(uint8_t status)
{
  return false;
}

/** @brief Window Covering Up Open
 *
 *
 */
bool emberAfWindowCoveringClusterWindowCoveringUpOpenCallback(void)
{
  return false;
}

/** @brief Window Covering Down Close
 *
 *
 */
bool emberAfWindowCoveringClusterWindowCoveringDownCloseCallback(void)
{
  return false;
}

/** @brief Window Covering Stop
 *
 *
 */
bool emberAfWindowCoveringClusterWindowCoveringStopCallback(void)
{
  return false;
}

/** @brief Window Covering Go To Lift Setpoint
 *
 *
 * @param indexOfLiftSetpoint   Ver.: always
 */
bool emberAfWindowCoveringClusterWindowCoveringGoToLiftSetpointCallback(uint8_t indexOfLiftSetpoint)
{
  return false;
}

/** @brief Window Covering Go To Lift Value
 *
 *
 * @param liftValue   Ver.: always
 */
bool emberAfWindowCoveringClusterWindowCoveringGoToLiftValueCallback(uint16_t liftValue)
{
  return false;
}

/** @brief Window Covering Go To Lift Percentage
 *
 *
 * @param percentageLiftValue   Ver.: always
 */
bool emberAfWindowCoveringClusterWindowCoveringGoToLiftPercentageCallback(uint8_t percentageLiftValue)
{
  return false;
}

/** @brief Window Covering Go To Tilt Setpoint
 *
 *
 * @param indexOfTiltSetpoint   Ver.: always
 */
bool emberAfWindowCoveringClusterWindowCoveringGoToTiltSetpointCallback(uint8_t indexOfTiltSetpoint)
{
  return false;
}

/** @brief Window Covering Go To Tilt Value
 *
 *
 * @param tiltValue   Ver.: always
 */
bool emberAfWindowCoveringClusterWindowCoveringGoToTiltValueCallback(uint16_t tiltValue)
{
  return false;
}

/** @brief Window Covering Go To Tilt Percentage
 *
 *
 * @param percentageTiltValue   Ver.: always
 */
bool emberAfWindowCoveringClusterWindowCoveringGoToTiltPercentageCallback(uint8_t percentageTiltValue)
{
  return false;
}

/** @brief Window Covering Go To Program Setpoint
 *
 *
 * @param setpointType   Ver.: always
 * @param setpointIndex   Ver.: always
 * @param setpointValue   Ver.: always
 */
bool emberAfWindowCoveringClusterWindowCoveringGoToProgramSetpointCallback(uint8_t setpointType,
                                                                           uint8_t setpointIndex,
                                                                           uint16_t setpointValue)
{
  return false;
}

/** @brief Match Protocol Address Response
 *
 *
 * @param deviceIeeeAddress   Ver.: always
 * @param protocolAddress   Ver.: always
 */
bool emberAfGenericTunnelClusterMatchProtocolAddressResponseCallback(uint8_t* deviceIeeeAddress,
                                                                     uint8_t* protocolAddress)
{
  return false;
}

/** @brief Client Attribute Changed
 *
 * Color Control cluster, Client Attribute Changed
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 * @param attributeId Attribute that changed  Ver.: always
 */
void emberAfColorControlClusterClientAttributeChangedCallback(uint8_t endpoint,
                                                              EmberAfAttributeId attributeId)
{
  emberAfColorControlClusterPrintln("ColorControlClientAttributeChange:%x,%2x",
                                    endpoint,
                                    attributeId);
}

/** @brief Server Attribute Changed
 *
 * Color Control cluster, Server Attribute Changed
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 * @param attributeId Attribute that changed  Ver.: always
 */
void emberAfColorControlClusterServerAttributeChangedCallback(uint8_t endpoint,
                                                              EmberAfAttributeId attributeId)
{
  emberAfColorControlClusterPrintln("ColorControlAttributeChange:%x,%2x",
                                    endpoint,
                                    attributeId);
  emberAfColorControlClusterFlush();
}

/** @brief Main Init
 *
 * This is the main application initialization callback. It is called prior to
 * application initialization and gives the application a chance to take care
 * of any of its own initialization procedures.
 *
 */
void emberAfMainInitCallback(void)
{
  EmberAfStatus status;
  uint8_t shortString[] = { 5, 'A', 'B', 'C', 'D', 'E' };
  uint8_t longString[] = { 40, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                           '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                           '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                           '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
  emberAfCorePrintln("Main init called.");
  // Now check the custom token if it works.

#ifndef EZSP_HOST
  // No tokens on host
  {
    uint8_t value;
#ifdef EMBER_TEST
//    In order to test that mfg-specific tokens are preserved across
//.   reboots in simulation, the following was commented out.
//    resetSystemTokens();
#endif
    halCommonGetToken((tokType_custom_token *)&value, TOKEN_CUSTOM_TOKEN);
    emberAfCorePrintln("Default custom token value:%x\n", value);

    value = 0x13;
    halCommonSetToken(TOKEN_CUSTOM_TOKEN, &value);
    halCommonGetToken((tokType_custom_token *)&value, TOKEN_CUSTOM_TOKEN);
    emberAfCorePrintln("Changed custom token value:%x\n", value);
  }
#endif // EZSP_HOST

  emberAfCoreFlush();
  // Bug 12081 said that trying to write a string that is too long should fail,
  // but it is more natural (to strncpy users, at least) to copy as much data
  // as will fit in the destination.  This change is documented as bug 14684.
  emberAfCorePrintln("Testing emberWriteAttribute for Strings");

  emberAfCoreFlush();
  status = emberAfWriteServerAttribute(1,
                                       ZCL_BASIC_CLUSTER_ID,
                                       ZCL_MANUFACTURER_NAME_ATTRIBUTE_ID,
                                       shortString,
                                       ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
  emberAfCorePrintln("Short string status: %x", status);

  emberAfCoreFlush();
  status = emberAfWriteServerAttribute(1,
                                       ZCL_BASIC_CLUSTER_ID,
                                       ZCL_MANUFACTURER_NAME_ATTRIBUTE_ID,
                                       longString,
                                       ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
  emberAfCorePrintln("Long string status: %x", status);
  emberAfCoreFlush();
}

/** @brief Tunnel Opened
 *
 * This function is called by the Tunneling client plugin whenever a tunnel is
 * opened.  Clients may open tunnels by sending a Request Tunnel command.
 *
 * @param tunnelIndex The index of the tunnel that has been opened.  Ver.:
 * always
 * @param tunnelStatus The status of the request.  Ver.: always
 * @param maximumIncomingTransferSize The maximum incoming transfer size of
 * the server.  Ver.: always
 */
void emberAfPluginTunnelingClientTunnelOpenedCallback(uint8_t tunnelIndex,
                                                      EmberAfPluginTunnelingClientStatus tunnelStatus,
                                                      uint16_t maximumIncomingTransferSize)
{
  emberAfTunnelingClusterFlush();
  emberAfTunnelingClusterPrintln("ClientTunnelOpened:%x,%x,%2x",
                                 tunnelIndex,
                                 tunnelStatus,
                                 maximumIncomingTransferSize);
}

/** @brief Data Received
 *
 * This function is called by the Tunneling client plugin whenever data is
 * received from a server through a tunnel.
 *
 * @param tunnelIndex The index of the tunnel through which the data was
 * received.  Ver.: always
 * @param data Buffer containing the raw octets of the data.  Ver.: always
 * @param dataLen The length in octets of the data.  Ver.: always
 */
void emberAfPluginTunnelingClientDataReceivedCallback(uint8_t tunnelIndex,
                                                      uint8_t * data,
                                                      uint16_t dataLen)
{
  emberAfTunnelingClusterFlush();
  emberAfTunnelingClusterPrint("ClientDataReceived:%x,[", tunnelIndex);
  emberAfTunnelingClusterPrintBuffer(data, dataLen, false);
  emberAfTunnelingClusterPrintln("]");
}

/** @brief Data Error
 *
 * This function is called by the Tunneling client plugin whenever a data
 * error occurs on a tunnel.  Errors occur if a device attempts to send data
 * on tunnel that is no longer active or if the tunneling does not belong to
 * the device.
 *
 * @param tunnelIndex The index of the tunnel on which this data error
 * occurred.  Ver.: always
 * @param transferDataStatus The error that occurred.  Ver.: always
 */
void emberAfPluginTunnelingClientDataErrorCallback(uint8_t tunnelIndex,
                                                   EmberAfTunnelingTransferDataStatus transferDataStatus)
{
  emberAfTunnelingClusterFlush();
  emberAfTunnelingClusterPrintln("ClientDataError:%x,%x",
                                 tunnelIndex,
                                 transferDataStatus);
}

/** @brief Tunnel Closed
 *
 * This function is called by the Tunneling client plugin whenever a server
 * sends a notification that it preemptively closed an inactive tunnel.
 * Servers are not required to notify clients of tunnel closures, so
 * applications cannot rely on this callback being called for all tunnels.
 *
 * @param tunnelIndex The index of the tunnel that has been closed.  Ver.:
 * always
 */
void emberAfPluginTunnelingClientTunnelClosedCallback(uint8_t tunnelIndex)
{
  emberAfTunnelingClusterFlush();
  emberAfTunnelingClusterPrintln("ClientTunnelClosed:%x", tunnelIndex);
}

/** @brief Is Protocol Supported
 *
 * This function is called by the Tunneling server plugin whenever a Request
 * Tunnel command is received.  The application should return true if the
 * protocol is supported and false otherwise.
 *
 * @param protocolId The identifier of the metering communication protocol for
 * which the tunnel is requested.  Ver.: always
 * @param manufacturerCode The manufacturer code for manufacturer-defined
 * protocols or 0xFFFF in unused.  Ver.: always
 */
bool emberAfPluginTunnelingServerIsProtocolSupportedCallback(uint8_t protocolId,
                                                             uint16_t manufacturerCode)
{
  emberAfTunnelingClusterFlush();
  emberAfTunnelingClusterPrintln("IsProtocolSupported:%x,%2x",
                                 protocolId,
                                 manufacturerCode);
  switch (protocolId) {
    case EMBER_ZCL_TUNNELING_PROTOCOL_ID_DLMS_COSEM:
    case EMBER_ZCL_TUNNELING_PROTOCOL_ID_IEC_61107:
    case EMBER_ZCL_TUNNELING_PROTOCOL_ID_ANSI_C12:
    case EMBER_ZCL_TUNNELING_PROTOCOL_ID_M_BUS:
    case EMBER_ZCL_TUNNELING_PROTOCOL_ID_SML:
    case EMBER_ZCL_TUNNELING_PROTOCOL_ID_CLIMATE_TALK:
      return true;
    default:
      return (protocolId >= 200 && manufacturerCode == EMBER_AF_MANUFACTURER_CODE);
  }
}

/** @brief Tunnel Opened
 *
 * This function is called by the Tunneling server plugin whenever a tunnel is
 * opened.  Clients may open tunnels by sending a Request Tunnel command.
 *
 * @param tunnelId The identifier of the tunnel that has been opened.  Ver.:
 * always
 * @param protocolId The identifier of the metering communication protocol for
 * the tunnel.  Ver.: always
 * @param manufacturerCode The manufacturer code for manufacturer-defined
 * protocols or 0xFFFF in unused.  Ver.: always
 * @param flowControlSupport true is flow control support is requested or
 * false if it is not.  Ver.: always
 * @param maximumIncomingTransferSize The maximum incoming transfer size of
 * the client.  Ver.: always
 */
void emberAfPluginTunnelingServerTunnelOpenedCallback(uint16_t tunnelId,
                                                      uint8_t protocolId,
                                                      uint16_t manufacturerCode,
                                                      bool flowControlSupport,
                                                      uint16_t maximumIncomingTransferSize)
{
  emberAfTunnelingClusterFlush();
  emberAfTunnelingClusterPrintln("ServerTunnelOpened:%2x,%x,%2x,%x,%2x",
                                 tunnelId,
                                 protocolId,
                                 manufacturerCode,
                                 flowControlSupport,
                                 maximumIncomingTransferSize);
}

/** @brief Data Received
 *
 * This function is called by the Tunneling server plugin whenever data is
 * received from a client through a tunnel.
 *
 * @param tunnelId The identifier of the tunnel through which the data was
 * received.  Ver.: always
 * @param data Buffer containing the raw octets of the data.  Ver.: always
 * @param dataLen The length in octets of the data.  Ver.: always
 */
void emberAfPluginTunnelingServerDataReceivedCallback(uint16_t tunnelId,
                                                      uint8_t * data,
                                                      uint16_t dataLen)
{
  emberAfTunnelingClusterFlush();
  emberAfTunnelingClusterPrint("ServerDataReceived:%2x,[", tunnelId);
  emberAfTunnelingClusterPrintBuffer(data, dataLen, false);
  emberAfTunnelingClusterPrintln("]");
}

/** @brief Data Error
 *
 * This function is called by the Tunneling server plugin whenever a data
 * error occurs on a tunnel.  Errors occur if a device attempts to send data
 * on tunnel that is no longer active or if the tunneling does not belong to
 * the device.
 *
 * @param tunnelId The identifier of the tunnel on which this data error
 * occurred.  Ver.: always
 * @param transferDataStatus The error that occurred.  Ver.: always
 */
void emberAfPluginTunnelingServerDataErrorCallback(uint16_t tunnelId,
                                                   EmberAfTunnelingTransferDataStatus transferDataStatus)
{
  emberAfTunnelingClusterFlush();
  emberAfTunnelingClusterPrintln("ServerDataError:%2x,%x",
                                 tunnelId,
                                 transferDataStatus);
}

/** @brief Tunnel Closed
 *
 * This function is called by the Tunneling server plugin whenever a tunnel is
 * closed.  Clients may close tunnels by sending a Close Tunnel command.  The
 * server can preemptively close inactive tunnels after a timeout.
 *
 * @param tunnelId The identifier of the tunnel that has been closed.  Ver.:
 * always
 * @param clientInitiated true if the client initiated the closing of the
 * tunnel or false if the server closed the tunnel due to inactivity.  Ver.:
 * always
 */
void emberAfPluginTunnelingServerTunnelClosedCallback(uint16_t tunnelId,
                                                      bool clientInitiated)
{
  emberAfTunnelingClusterFlush();
  emberAfTunnelingClusterPrintln("ServerTunnelClosed:%2x,%p",
                                 tunnelId,
                                 (clientInitiated ? "CLIENT" : "SERVER"));
}

/** @brief Request Mirror
 *
 * This function is called by the Simple Metering client plugin whenever a
 * Request Mirror command is received.  The application should return the
 * endpoint to which the mirror has been assigned.  If no mirror could be
 * assigned, the application should return 0xFFFF.
 *
 * @param requestingDeviceIeeeAddress   Ver.: always
 */
uint16_t emberAfPluginSimpleMeteringClientRequestMirrorCallback(EmberEUI64 requestingDeviceIeeeAddress)
{
  return 0xFFFF;
}

/** @brief Remove Mirror
 *
 * This function is called by the Simple Metering client plugin whenever a
 * Remove Mirror command is received.  The application should return the
 * endpoint on which the mirror has been removed.  If the mirror could not be
 * removed, the application should return 0xFFFF.
 *
 * @param requestingDeviceIeeeAddress   Ver.: always
 */
uint16_t emberAfPluginSimpleMeteringClientRemoveMirrorCallback(EmberEUI64 requestingDeviceIeeeAddress)
{
  return 0xFFFF;
}

/** @brief Display Message
 *
 * This function is called by the Messaging client plugin whenever the
 * application should display a message.
 *
 * @param message The message that should be displayed.  Ver.: always
 */
void emberAfPluginMessagingClientDisplayMessageCallback(EmberAfPluginMessagingClientMessage * message)
{
}

/** @brief Cancel Message
 *
 * This function is called by the Messaging client plugin whenever the
 * application should stop displaying a message.
 *
 * @param message The message that should no longer be displayed.  Ver.:
 * always
 */
void emberAfPluginMessagingClientCancelMessageCallback(EmberAfPluginMessagingClientMessage * message)
{
}

/** @brief Price Started
 *
 * This function is called by the Price client plugin whenever a price starts.
 *
 * @param price The price that has started.  Ver.: always
 */
void emberAfPluginPriceClientPriceStartedCallback(EmberAfPluginPriceClientPrice * price)
{
}

/** @brief Price Expired
 *
 * This function is called by the Price client plugin whenever a price
 * expires.
 *
 * @param price The price that has expired.  Ver.: always
 */
void emberAfPluginPriceClientPriceExpiredCallback(EmberAfPluginPriceClientPrice * price)
{
}

/** @brief Configured
 *
 * This callback is called by the Reporting plugin whenever a reporting entry
 * is configured, including when entries are deleted or updated.  The
 * application can use this callback for scheduling readings or measurements
 * based on the minimum and maximum reporting interval for the entry.  The
 * application should return EMBER_ZCL_STATUS_SUCCESS if it can support the
 * configuration or an error status otherwise.  Note: attribute reporting is
 * required for many clusters and attributes, so rejecting a reporting
 * configuration may violate ZigBee specifications.
 *
 * @param entry   Ver.: always
 */
EmberAfStatus emberAfPluginReportingConfiguredCallback(const EmberAfPluginReportingEntry * entry)
{
  return EMBER_ZCL_STATUS_SUCCESS;
}

/** @brief Registration
 *
 * This callback is called when the device joins a network and the process of
 * registration is complete. This callback provides a success value of true if
 * the registration process was successful and a value of false if
 * registration failed.
 *
 * @param success true if registration succeeded, false otherwise.  Ver.:
 * always
 */
void emberAfRegistrationCallback(bool success)
{
  emberAfRegistrationPrintln("Registration %p", success ? "complete" : "failed");
}

/** @brief Client Default Response
 *
 * Color Control cluster, Client Default Response
 *
 * @param endpoint Destination endpoint  Ver.: always
 * @param commandId Command id  Ver.: always
 * @param status Status in default response  Ver.: always
 */
void emberAfColorControlClusterClientDefaultResponseCallback(uint8_t endpoint,
                                                             uint8_t commandId,
                                                             EmberAfStatus status)
{
  emberAfCorePrintln("ColorControlClientDefaultResponse:%x,%x,%x", endpoint, commandId, status);
}

/** @brief Server Default Response
 *
 * Color Control cluster, Server Default Response
 *
 * @param endpoint Destination endpoint  Ver.: always
 * @param commandId Command id  Ver.: always
 * @param status Status in default response  Ver.: always
 */
void emberAfColorControlClusterServerDefaultResponseCallback(uint8_t endpoint,
                                                             uint8_t commandId,
                                                             EmberAfStatus status)
{
  emberAfCorePrintln("ColorControlServerDefaultResponse:%x,%x,%x", endpoint, commandId, status);
}

/** @brief Event Action
 *
 * This function is called by the demand response and load control client
 * plugin whenever an event status changes within the DRLC event table.  The
 * list of possible event status values is defined by the ZCL spec and is
 * listed in the Application Framework's generated enums located in enums.h.
 * For example, an event status may be:
 * AMI_EVENT_STATUS_LOAD_CONTROL_EVENT_COMMAND_RX indicating that a properly
 * formatted event was received; AMI_EVENT_STATUS_EVENT_STARTED indicating
 * that an event has started; AMI_EVENT_STATUS_THE_EVENT_HAS_BEEN_CANCELED,
 * indicating that the event was canceled.  This callback is intended to give
 * the device an opportunity to take action on the event in question.  For
 * instance if an event starts, the device should take the appropriate event
 * action on the hardware.  This callback returns a bool, if returned value
 * is true, then a notification will be send over the air automatically to the
 * originator of the event.  If it is false, then nothing will be sent back to
 * the originator of the event.  Please note that in order for your
 * application to be ZigBee compliant, a notification must be sent over the
 * air to the originator of the event, so a value of false should only be
 * returned if your application code takes care of sending this message or
 * there is some other reason a message does not need to be sent by the
 * framework.
 *
 * @param loadControlEvent Actual event  Ver.: always
 * @param eventStatus Status of event  Ver.: always
 * @param sequenceNumber Sequence number  Ver.: always
 */
bool emberAfPluginDrlcEventActionCallback(EmberAfLoadControlEvent * loadControlEvent,
                                          EmberAfAmiEventStatus eventStatus,
                                          uint8_t sequenceNumber)
{
  return true;
}

/** @brief Command One
 *
 *
 * @param argOne   Ver.: always
 */
bool emberAfSampleMfgSpecificClusterCommandOneCallback(uint8_t argOne)
{
  emberAfCustom1Println("Rx Sample Mfg Specific Cluster Custom One Command: %x", argOne);
  emberAfCustom1Flush();
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

/** @brief Command One
 *
 *
 * @param argOne   Ver.: always
 */
bool emberAfSampleMfgSpecificCluster2CommandTwoCallback(uint8_t argOne)
{
  emberAfCustom1Println("Rx Sample Mfg Specific Cluster Custom Two Command: %x,%x", argOne, argOne);
  emberAfCustom1Flush();
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

/** @brief Server Manufacturer Specific Attribute Changed
 *
 * Sample Mfg Specific Cluster cluster, Server Manufacturer Specific Attribute
 * Changed
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 * @param attributeId Attribute that changed  Ver.: always
 * @param manufacturerCode Manufacturer Code of the attribute that changed
 * Ver.: always
 */
void emberAfSampleMfgSpecificClusterServerManufacturerSpecificAttributeChangedCallback(uint8_t endpoint,
                                                                                       EmberAfAttributeId attributeId,
                                                                                       uint16_t manufacturerCode)
{
}

/** @brief Sample Mfg Specific Off With Transition
 *
 *
 */
bool emberAfOnOffClusterSampleMfgSpecificOffWithTransitionCallback(void)
{
  emberAfCustom1Println("Rx Sample Mfg Specific Off With Transition");
  emberAfCustom1Flush();
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

/** @brief Sample Mfg Specific On With Transition
 *
 *
 */
bool emberAfOnOffClusterSampleMfgSpecificOnWithTransitionCallback(void)
{
  emberAfCustom1Println("Rx Sample Mfg Specific On With Transition");
  emberAfCustom1Flush();
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

/** @brief Sample Mfg Specific On With Transition
 *
 *
 */
bool emberAfOnOffClusterSampleMfgSpecificOnWithTransition2Callback(void)
{
  emberAfCustom1Println("Rx Sample Mfg Specific 2 On With Transition");
  emberAfCustom1Flush();
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

/** @brief Sample Mfg Specific Toggle With Transition
 *
 *
 */
bool emberAfOnOffClusterSampleMfgSpecificToggleWithTransitionCallback(void)
{
  emberAfCustom1Println("Rx Sample Mfg Specific Toggle With Transition");
  emberAfCustom1Flush();
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

/** @brief Server Manufacturer Specific Attribute Changed
 *
 * On/off cluster, Server Manufacturer Specific Attribute Changed
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 * @param attributeId Attribute that changed  Ver.: always
 * @param manufacturerCode Manufacturer Code of the attribute that changed
 * Ver.: always
 */
void emberAfOnOffClusterServerManufacturerSpecificAttributeChangedCallback(uint8_t endpoint,
                                                                           EmberAfAttributeId attributeId,
                                                                           uint16_t manufacturerCode)
{
  emberAfCustom1Println("Rx On/Off Cluster Mfg Specific attribute changed callback, endpt:%x, attId:%2x, mfgCode:%2x",
                        endpoint, attributeId, manufacturerCode);
  emberAfCustom1Flush();
}

/** @brief Reset To Factory Defaults
 *
 * This function is called by the Basic server plugin when a request to reset
 * to factory defaults is received.  The plugin will reset attributes managed
 * by the framework to their default values.  The application should perform
 * any other necessary reset-related operations in this callback, including
 * resetting any externally-stored attributes.
 *
 * @param endpoint Specific endpoint on which to operate; EMBER_BROADCAST_ENDPOINT for all endpoints
 *
 */
void emberAfPluginBasicResetToFactoryDefaultsCallback(uint8_t endpoint)
{
}

/** @brief Is Color Supported
 *
 * This function will be called to determine whether a color is supported by a
 * device. The color will be specified by hue and saturation.
 *
 * @param hue   Ver.: always
 * @param saturation   Ver.: always
 */
bool emberAfPluginColorControlIsColorSupportedCallback(uint8_t hue,
                                                       uint8_t saturation)
{
  return true;
}

/** @brief Pre Message Received
 *
 * This function is called by the Interpan plugin when an interpan message is
 * received but has not yet been handled by the plugin or the framework.  The
 * application should return true if the message was handled.
 *
 * @param header   Ver.: always
 * @param msgLen   Ver.: always
 * @param message   Ver.: always
 */
bool emberAfPluginInterpanPreMessageReceivedCallback(const EmberAfInterpanHeader * header,
                                                     uint8_t msgLen,
                                                     uint8_t * message)
{
  return false;
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

/** @brief Activate Door Lock
 *
 * This function is provided by the door lock server plugin. It is
 *
 * @param activate True if the lock should move to the locked position, false
 * if it should move to the unlocked position  Ver.: always
 */
bool emberAfPluginDoorLockServerActivateDoorLockCallback(bool activate)
{
  return true;
}

/** @brief Broadcast Sent
 *
 * This function is called when a new MTORR broadcast has been successfully
 * sent by the concentrator plugin.
 *
 */
void emberAfPluginConcentratorBroadcastSentCallback(void)
{
}

/** @brief Group Names Supported
 *
 * This function is called by the framework when it is necessary to determine
 * whether or not group names are supported.
 *
 * @param endpoint The endpoint. Ver.: always
 */
bool emberAfPluginGroupsServerGroupNamesSupportedCallback(uint8_t endpoint)
{
  return false;
}

/** @brief Get Group Name
 *
 * This function is called by the framework to query for a group name based
 * on the endpoint and the group ID.
 *
 * @param endpoint The endpoint. Ver.: always
 * @param groupId  The group ID. Ver.: always
 * @param groupName Pointer to the group name. Ver.: always
 */
void emberAfPluginGroupsServerGetGroupNameCallback(uint8_t endpoint,
                                                   uint16_t groupId,
                                                   uint8_t *groupName)
{
}

/** @brief Set Group Name
 *
 * This function is called by the framework to set a group name based
 * on the endpoint and the group ID.
 *
 * @param endpoint The endpoint. Ver.: always
 * @param groupId  The group ID. Ver.: always
 * @param groupName Pointer to the group name. Ver.: always
 */
void emberAfPluginGroupsServerSetGroupNameCallback(uint8_t endpoint,
                                                   uint16_t groupId,
                                                   uint8_t *groupName)
{
}

/** @brief Button Event
 *
 * This allows another module to get notification when a button is pressed and
 * released but the button joining plugin did not handle it.  This callback is
 * NOT called in ISR context so there are no restrictions on what code can
 * execute.
 *
 * @param buttonNumber The button number that was pressed.  Ver.: always
 * @param buttonPressDurationMs The length of time button was held down before
 * it was released.  Ver.: always
 */
void emberAfPluginButtonJoiningButtonEventCallback(uint8_t buttonNumber,
                                                   uint32_t buttonPressDurationMs)
{
}
/** @brief Counter Rollover
 *
 * This function is called every time a counter exceeds its threshold.
 *
 * @param type The counter that rolled over  Ver.: always
 */
void emberAfPluginCountersRolloverCallback(EmberCounterType type)
{
  emberAfCorePrintln("Counter %u rolled over", type);
}

bool emberAfIasAceClusterBypassCallback(uint8_t numberOfZones,
                                        uint8_t* zoneIds,
                                        uint8_t* armDisarmCode)
{
  return true;
}

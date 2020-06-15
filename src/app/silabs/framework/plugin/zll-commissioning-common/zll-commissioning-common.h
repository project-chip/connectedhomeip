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
 * @brief Definitions for the ZLL Commissioning Common plugin.
 *******************************************************************************
   ******************************************************************************/
#ifndef __ZLL_COMMISIONING_COMMON_H__
#define __ZLL_COMMISIONING_COMMON_H__
// *******************************************************************
// * zll-commissioning-common.h
// *
// *
// Copyright 2010-2018 Silicon Laboratories, Inc.
// *******************************************************************

/** @brief Generates a random network key and initializes the security state of
 * the device.
 *
 * This function is a convenience wrapper for ::emberZllSetInitialSecurityState,
 * which must be called before starting or joining a network. The plugin
 * initializes the security state for the initiator during touch linking. The
 * target must initialize its own security state prior to forming a network
 * either by using this function or by calling ::emberZllSetInitialSecurityState
 * directly.
 *
 * @return An ::EmberStatus value that indicates the success or failure of the
 * command.
 */
EmberStatus emberAfZllSetInitialSecurityState(void);

/** @brief Indicates whether a touch link procedure is currently in progress.
 *
 * @return True if a touch link is in progress or false otherwise.
 */
bool emberAfZllTouchLinkInProgress(void);

/** @brief Resets the local device to a factory new state.
 *
 * This function causes the device to leave the network and clear its
 * network parameters, resets its attributes to their default values, and clears
 * the group and scene tables.
 */
void emberAfZllResetToFactoryNew(void);

/** @brief Disables touchlink processing.
 *
 * This function will cause the device to refuse network start/join
 * requests if it receives them and will not allow touchlinking.
 * Note that this will have the effect of overriding the
 * emberAfZllNoTouchlinkForNFN function.
 */
EmberStatus emberAfZllDisable(void);

/** @brief Enables touchlink processing.
 *
 * This function will cause the device to accept network start/join
 * requests if it receives them and will not allow touchlinking.
 * Note that this will have the effect of overriding the
 * emberAfZllNoTouchlinkForNFN function.
 */
EmberStatus emberAfZllEnable(void);

// Global data for all ZLL commissioning plugins.
extern EmberZllNetwork emAfZllNetwork;
extern uint16_t emAfZllFlags;
extern uint8_t emAfInitiatorRejoinRetryCount;

#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT
// Sub-device info (mainly for client, but server needs to initialize the count).
extern EmberZllDeviceInfoRecord emAfZllSubDevices[];
extern uint8_t emAfZllSubDeviceCount;

// The identify duration may be updated by CLI command.
extern uint16_t emAfZllIdentifyDurationSec;
#endif

// State bits for client and server.
enum {
  INITIAL                     = 0x0000,
  SCAN_FOR_TOUCH_LINK         = 0x0001,
  SCAN_FOR_DEVICE_INFORMATION = 0x0002,
  SCAN_FOR_IDENTIFY           = 0x0004,
  SCAN_FOR_RESET              = 0x0008,
  TARGET_NETWORK_FOUND        = 0x0010,
  ABORTING_TOUCH_LINK         = 0x0020,
  SCAN_COMPLETE               = 0x0040,
  TOUCH_LINK_TARGET           = 0x0080,
  FORMING_NETWORK             = 0x0100,
  RESETTING_TO_FACTORY_NEW    = 0x0200,
};

#define touchLinkInProgress()      (emAfZllFlags                     \
                                    & (SCAN_FOR_TOUCH_LINK           \
                                       | SCAN_FOR_DEVICE_INFORMATION \
                                       | SCAN_FOR_IDENTIFY           \
                                       | SCAN_FOR_RESET              \
                                       | TOUCH_LINK_TARGET))
#define scanForTouchLink()         (emAfZllFlags & SCAN_FOR_TOUCH_LINK)
#define scanForDeviceInformation() (emAfZllFlags & SCAN_FOR_DEVICE_INFORMATION)
#define scanForIdentify()          (emAfZllFlags & SCAN_FOR_IDENTIFY)
#define scanForReset()             (emAfZllFlags & SCAN_FOR_RESET)
#define targetNetworkFound()       (emAfZllFlags & TARGET_NETWORK_FOUND)
#define abortingTouchLink()        (emAfZllFlags & ABORTING_TOUCH_LINK)
#define scanComplete()             (emAfZllFlags & SCAN_COMPLETE)
#define touchLinkTarget()          (emAfZllFlags & TOUCH_LINK_TARGET)
#define formingNetwork()           (emAfZllFlags & FORMING_NETWORK)
#define resettingToFactoryNew()    (emAfZllFlags & RESETTING_TO_FACTORY_NEW)

// The bits for cluster-specific command (0) and disable default response (4)
// are always set.  The direction bit (3) is only set for server-to-client
// commands (i.e., DeviceInformationResponse).  Some Philips devices still use
// the old frame format and set the frame control to zero.
#define ZLL_FRAME_CONTROL_LEGACY           0x00
#define ZLL_FRAME_CONTROL_CLIENT_TO_SERVER 0x11
#define ZLL_FRAME_CONTROL_SERVER_TO_CLIENT 0x19

#define ZLL_HEADER_FRAME_CONTROL_OFFSET   0 // one byte
#define ZLL_HEADER_SEQUENCE_NUMBER_OFFSET 1 // one byte
#define ZLL_HEADER_COMMAND_ID_OFFSET      2 // one byte
#define ZLL_HEADER_TRANSACTION_ID_OFFSET  3 // four bytes
#define ZLL_HEADER_OVERHEAD               7

// Radio modes used by PHY
enum {
  EMBER_RADIO_POWER_MODE_RX_ON,
  EMBER_RADIO_POWER_MODE_OFF
};

// Uncomment the next line for extra debugs!
#define PLUGIN_DEBUG
#if defined(PLUGIN_DEBUG)
  #define debugPrintln(...) emberAfCorePrintln(__VA_ARGS__)
  #define debugPrint(...)   emberAfCorePrint(__VA_ARGS__)
  #define debugExec(x) do { x; } while (0)
#else
  #define debugPrintln(...)
  #define debugPrint(...)
  #define debugExec(x)
#endif

// For legacy code
#define emberAfPluginZllCommissioningInitialSecurityStateCallback \
  emberAfPluginZllCommissioningCommonInitialSecurityStateCallback
#define emberAfPluginZllCommissioningTouchLinkCompleteCallback \
  emberAfPluginZllCommissioningCommonTouchLinkCompleteCallback
#define emberAfPluginZllCommissioningResetToFactoryNewCallback \
  emberAfPluginZllCommissioningCommonResetToFactoryNewCallback
#endif // __ZLL_COMMISIONING_COMMON_H__

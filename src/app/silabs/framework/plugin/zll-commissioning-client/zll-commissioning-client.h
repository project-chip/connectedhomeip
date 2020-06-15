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
 * @brief Definitions for the ZLL Commissioning Client plugin.
 *******************************************************************************
   ******************************************************************************/

// *******************************************************************
// * zll-commissioning-client.h
// *
// *
// Copyright 2010-2018 Silicon Laboratories, Inc.
// *******************************************************************

/** @brief Initiates the touch link procedure.
 *
 * This function causes the stack to broadcast a series of ScanRequest
 * commands via inter-PAN messaging. The plugin selects the target that
 * sent a ScanResponse command with the strongest RSSI and attempts to link with
 * it. If touch linking completes successfully, the plugin will call
 * ::emberAfPluginZllCommissioningTouchLinkCompleteCallback with information
 * about the network and the target. If touch linking fails, the plugin will
 * call ::emberAfPluginZllCommissioningTouchLinkFailedCallback.
 *
 * @return An ::EmberStatus value that indicates the success or failure of the
 * command.
 */
EmberStatus emberAfZllInitiateTouchLink(void);

/** @brief Initiates a touch link to retrieve information
 * about a target device.
 *
 * As with a traditional touch link, this function causes the stack to
 * broadcast messages to discover a target device. When the target is selected
 * (based on RSSI), the plugin will retrieve information about it by unicasting
 * a series of DeviceInformationRequest commands via inter-PAN messaging. If
 * the process completes successfully, the plugin will call
 * ::emberAfPluginZllCommissioningTouchLinkCompleteCallback with information
 * about the target. If touch linking fails, the plugin will call
 * ::emberAfPluginZllCommissioningTouchLinkFailedCallback.
 *
 * @return An ::EmberStatus value that indicates the success or failure of the
 * command.
 */
EmberStatus emberAfZllDeviceInformationRequest(void);

/** @brief Initiates a touch link to cause a target device to
 * identify itself.
 *
 * As with a traditional touch link, this function causes the stack to
 * broadcast messages to discover a target device. When the target is selected
 * (based on RSSI), the plugin will cause it to identify itself by unicasting
 * an IdentifyRequest command via inter-PAN messaging. If the process
 * completes successfully, the plugin will call
 * ::emberAfPluginZllCommissioningTouchLinkCompleteCallback with information
 * about the target. If touch linking fails, the plugin will call
 * ::emberAfPluginZllCommissioningTouchLinkFailedCallback.
 *
 * @return An ::EmberStatus value that indicates the success or failure of the
 * command.
 */
EmberStatus emberAfZllIdentifyRequest(void);

/** @brief Initiates a touch link for the purpose of resetting a target device.
 *
 * As with a traditional touch link, this function causes the stack to
 * broadcast messages to discover a target device. When the target is selected
 * (based on RSSI), the plugin will reset it by unicasting a
 * ResetToFactoryNewRequest command via inter-PAN messaging. If the process
 * completes successfully, the plugin will call
 * ::emberAfPluginZllCommissioningTouchLinkCompleteCallback with information
 * about the target. If touch linking fails, the plugin will call
 * ::emberAfPluginZllCommissioningTouchLinkFailedCallback.
 *
 * @return An ::EmberStatus value that indicates the success or failure of the
 * command.
 */
EmberStatus emberAfZllResetToFactoryNewRequest(void);

/** @brief Aborts the touch link procedure.
 *
 * This function can be called to cancel the touch link procedure. This can be
 * useful, for example, if the touch link target is incorrect.
 */
void emberAfZllAbortTouchLink(void);

// For legacy code
#define emberAfPluginZllCommissioningTouchLinkFailedCallback \
  emberAfPluginZllCommissioningClientTouchLinkFailedCallback

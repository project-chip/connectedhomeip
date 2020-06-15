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
 * @brief Definitions for the ZLL Commissioning Server plugin.
 *******************************************************************************
   ******************************************************************************/

// *******************************************************************
// * zll-commissioning-server.h
// *
// *
// Copyright 2010-2018 Silicon Laboratories, Inc.
// *******************************************************************

/** @brief No touchlink for non-factory new device.
 *
 * This function will cause an NFN device to refuse network start/join/update
 * requests and thus to forbid commissioning by touchlinking. This can be useful
 * to restrict touchlink stealing.
 */
EmberStatus emberAfZllNoTouchlinkForNFN(void);
/** @brief No reset for non-factory new device.
 *
 * This function will cause an NFN device on a centralized security network to
 * a touchlink reset-to-factory-new request from a remote device.
 */
EmberStatus emberAfZllNoResetForNFN(void);

// For legacy code
#define emberAfPluginZllCommissioningGroupIdentifierCountCallback \
  emberAfPluginZllCommissioningServerGroupIdentifierCountCallback
#define emberAfPluginZllCommissioningGroupIdentifierCallback \
  emberAfPluginZllCommissioningServerGroupIdentifierCallback
#define emberAfPluginZllCommissioningEndpointInformationCountCallback \
  emberAfPluginZllCommissioningServerEndpointInformationCountCallback
#define emberAfPluginZllCommissioningEndpointInformationCallback \
  emberAfPluginZllCommissioningServerEndpointInformationCallback
#define emberAfPluginZllCommissioningIdentifyCallback \
  emberAfPluginZllCommissioningServerIdentifyCallback

/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

// Outbound CASE for the Joint Fabric Administrator shared code.
//
// CHIP_CONFIG_JFA_HAS_COMMISSIONER_ROLE selects the stack that owns outbound
// sessions to peers on the joint fabric:
//
//   1: the local DeviceCommissioner brought up by JFAInitCommissioner()
//      (JFAPlatform.h). Requires the platform to build the commissioner stack.
//   0: the Server's CASESessionManager. No commissioner stack is needed.

#pragma once

#include <app/CASESessionManager.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

#ifndef CHIP_CONFIG_JFA_HAS_COMMISSIONER_ROLE
#define CHIP_CONFIG_JFA_HAS_COMMISSIONER_ROLE 0
#endif

namespace chip {

// Finds or establishes a CASE session to peerNodeId on the joint fabric.
// Returns CHIP_ERROR_INCORRECT_STATE if the joint fabric is not known yet, or,
// with the commissioner role, if the commissioner is not initialized yet.
CHIP_ERROR JFAFindOrEstablishSession(NodeId peerNodeId, Callback::Callback<OnDeviceConnected> * onConnected,
                                     Callback::Callback<OnDeviceConnectionFailure> * onFailure);

} // namespace chip

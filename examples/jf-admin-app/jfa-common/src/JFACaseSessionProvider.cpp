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

#include "JFACaseSessionProvider.h"

#include <app/server/Server.h>
#include <lib/support/CodeUtils.h>

#if CHIP_CONFIG_JFA_HAS_COMMISSIONER_ROLE
#include "JFAPlatform.h"
#endif

namespace chip {

CHIP_ERROR JFAFindOrEstablishSession(NodeId peerNodeId, Callback::Callback<OnDeviceConnected> * onConnected,
                                     Callback::Callback<OnDeviceConnectionFailure> * onFailure)
{
#if CHIP_CONFIG_JFA_HAS_COMMISSIONER_ROLE
    Controller::DeviceCommissioner * commissioner = JFAGetDeviceCommissioner();
    VerifyOrReturnError(commissioner != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return commissioner->GetConnectedDevice(peerNodeId, onConnected, onFailure);
#else
    const FabricIndex fabricIndex = Server::GetInstance().GetJointFabricDatastore().GetAnchorFabricIndex();
    VerifyOrReturnError(fabricIndex != kUndefinedFabricIndex, CHIP_ERROR_INCORRECT_STATE);

    Server::GetInstance().GetCASESessionManager()->FindOrEstablishSession(ScopedNodeId(peerNodeId, fabricIndex), onConnected,
                                                                          onFailure);
    return CHIP_NO_ERROR;
#endif
}

} // namespace chip

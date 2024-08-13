/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "StayActiveSender.h"
#include <controller/InvokeInteraction.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AppConfig.h>
#include <app/CommandPathParams.h>
#include <app/OperationalSessionSetup.h>
#include <memory>

StayActiveSender::StayActiveSender(uint32_t stayActiveDuration, const chip::ScopedNodeId & peerNode,
                     chip::app::InteractionModelEngine * engine) :
    mStayActiveDuration(stayActiveDuration), mPeerNode(peerNode), mpImEngine(engine),
    mOnConnectedCallback(HandleDeviceConnected, this), mOnConnectionFailureCallback(HandleDeviceConnectionFailure, this)
{}

CHIP_ERROR StayActiveSender::SendStayActiveCommand(chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle)
{
    auto onSuccess = [&](const chip::app::ConcreteCommandPath & commandPath, const chip::app::StatusIB & status, const auto & dataResponse) {
        uint32_t promisedActiveDuration = dataResponse.promisedActiveDuration;
        ChipLogProgress(ICD, "StayActive command succeeded with promised duration %u", promisedActiveDuration);
        // TODO: please send multiple stayActive request with 30 seconds up to x min since active duration in server side is upper bound to 30 seconds
        if (mStayActiveDuration > dataResponse.promisedActiveDuration)
        {
            mStayActiveDuration -= dataResponse.promisedActiveDuration;
        }
    };

    auto onFailure = [&](CHIP_ERROR error) {
        ChipLogError(ICD, "StayActive command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    chip::EndpointId endpointId = 0;
    chip::app::Clusters::IcdManagement::Commands::StayActiveRequest::Type request;
    request.stayActiveDuration = mStayActiveDuration;
    return chip::Controller::InvokeCommandRequest(&exchangeMgr, sessionHandle, endpointId, request, onSuccess, onFailure);
}

CHIP_ERROR StayActiveSender::EstablishSessionToPeer()
{
    ChipLogProgress(ICD, "Trying to establish a CASE session to extend the active period for lit icd device");
    auto * caseSessionManager = mpImEngine->GetCASESessionManager();
    VerifyOrReturnError(caseSessionManager != nullptr, CHIP_ERROR_INVALID_CASE_PARAMETER);
    caseSessionManager->FindOrEstablishSession(mPeerNode, &mOnConnectedCallback, &mOnConnectionFailureCallback);
    return CHIP_NO_ERROR;
}

void StayActiveSender::HandleDeviceConnected(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                             const chip::SessionHandle & sessionHandle)
{
    StayActiveSender * const _this = static_cast<StayActiveSender *>(context);
    VerifyOrDie(_this != nullptr);

    CHIP_ERROR err = _this->SendStayActiveCommand(exchangeMgr, sessionHandle);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(ICD, "Failed to send stay active command");
    }
}

void StayActiveSender::HandleDeviceConnectionFailure(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR err)
{
    StayActiveSender * const _this = static_cast<StayActiveSender *>(context);
    VerifyOrDie(_this != nullptr);
    ChipLogError(ICD, "Failed to establish CASE for stay active command with error '%" CHIP_ERROR_FORMAT "'", err.Format());;
}

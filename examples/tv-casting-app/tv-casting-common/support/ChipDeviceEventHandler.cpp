/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "ChipDeviceEventHandler.h"

#include "core/CastingPlayer.h"
#include "core/Types.h"
#include "support/CastingStore.h"
#include "support/EndpointListLoader.h"

#include "app/clusters/bindings/BindingManager.h"

namespace matter {
namespace casting {
namespace support {

using namespace matter::casting::core;

bool ChipDeviceEventHandler::sUdcInProgress = false;

void ChipDeviceEventHandler::Handle(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    ChipLogProgress(AppServer, "ChipDeviceEventHandler::Handle() called");

    // Make sure we have not disconnected from the TargetCastingPlayer when handling incoming messages.
    // Sometimes the tv-app will still send messages after we clean up the TargetCastingPlayer.
    // Exmaple: Call stopConnecting() after immediately after calling continueConnectiong()
    //
    // A proper fix would be to either not let user navigate back when running post commission or update commissioner, commissionee
    // communication protocal to better handle asynchronous disconnects
    if (CastingPlayer::GetTargetCastingPlayer() == nullptr)
    {
        ChipLogError(AppServer, "ChipDeviceEventHandler::Handler() TargetCastingPlayer is null for event: %u", event->Type);
        return;
    }

    bool runPostCommissioning           = false;
    chip::NodeId targetNodeId           = 0;
    chip::FabricIndex targetFabricIndex = 0;

    if (event->Type == chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired &&
        CastingPlayer::GetTargetCastingPlayer()->mConnectionState == CASTING_PLAYER_CONNECTING)
    {
        ChipLogProgress(AppServer, "ChipDeviceEventHandler::Handle() event kFailSafeTimerExpired");
        HandleFailSafeTimerExpired();
    }
    else if (event->Type == chip::DeviceLayer::DeviceEventType::kBindingsChangedViaCluster &&
             CastingPlayer::GetTargetCastingPlayer() != nullptr)
    {
        HandleBindingsChangedViaCluster(event, arg, runPostCommissioning, targetNodeId, targetFabricIndex);
    }
    else if (event->Type == chip::DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        // True when device completes initial commissioning (PASE).
        // Note: Not triggered for subsequent CASE sessions established via CastingPlayer::FindOrEstablishSession()
        HandleCommissioningComplete(event, arg, runPostCommissioning, targetNodeId, targetFabricIndex);
    }

    // Run post commissioing for kBindingsChangedViaCluster and kCommissioningComplete events.
    if (runPostCommissioning)
    {
        sUdcInProgress = false;
        CastingPlayer::GetTargetCastingPlayer()->SetNodeId(targetNodeId);
        CastingPlayer::GetTargetCastingPlayer()->SetFabricIndex(targetFabricIndex);

        ChipLogProgress(AppServer, "ChipDeviceEventHandler::Handle() calling CastingPlayer FindOrEstablishSession()");
        CastingPlayer::GetTargetCastingPlayer()->FindOrEstablishSession(
            nullptr,
            [](void * context, chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle) {
                ChipLogProgress(AppServer, "ChipDeviceEventHandler::Handle() Connection to CastingPlayer successful");
                CastingPlayer::GetTargetCastingPlayer()->mConnectionState = CASTING_PLAYER_CONNECTED;

                // this async call will Load all the endpoints with their respective attributes into the TargetCastingPlayer
                // persist the TargetCastingPlayer information into the CastingStore and call mOnCompleted()
                EndpointListLoader::GetInstance()->Initialize(&exchangeMgr, &sessionHandle);
                ChipLogProgress(AppServer, "ChipDeviceEventHandler::Handle() calling EndpointListLoader::GetInstance()->Load()");
                EndpointListLoader::GetInstance()->Load();
            },
            [](void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error) {
                ChipLogError(AppServer, "ChipDeviceEventHandler::Handle(): Connection to CastingPlayer failed");
                CastingPlayer::GetTargetCastingPlayer()->mConnectionState = CASTING_PLAYER_NOT_CONNECTED;
                CHIP_ERROR err = support::CastingStore::GetInstance()->Delete(*CastingPlayer::GetTargetCastingPlayer());
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer, "CastingStore::Delete() failed. Err: %" CHIP_ERROR_FORMAT, err.Format());
                }

                VerifyOrReturn(CastingPlayer::GetTargetCastingPlayer()->mOnCompleted);
                CastingPlayer::GetTargetCastingPlayer()->mOnCompleted(error, nullptr);
                CastingPlayer::mTargetCastingPlayer.reset();
            });
    }
}

void ChipDeviceEventHandler::HandleFailSafeTimerExpired()
{
    // if UDC was in progress (when the Fail-Safe timer expired), reset TargetCastingPlayer commissioning state and return early
    if (sUdcInProgress)
    {
        ChipLogProgress(AppServer, "ChipDeviceEventHandler::HandleFailSafeTimerExpired() when sUdcInProgress: %d, returning early",
                        sUdcInProgress);
        sUdcInProgress                                            = false;
        CastingPlayer::GetTargetCastingPlayer()->mConnectionState = CASTING_PLAYER_NOT_CONNECTED;
        CastingPlayer::GetTargetCastingPlayer()->mOnCompleted(CHIP_ERROR_TIMEOUT, nullptr);
        CastingPlayer::GetTargetCastingPlayer()->mOnCompleted = nullptr;
        CastingPlayer::GetTargetCastingPlayer()->mTargetCastingPlayer.reset();
        return;
    }

    // if UDC was NOT in progress (when the Fail-Safe timer expired), start UDC
    ChipLogProgress(AppServer, "ChipDeviceEventHandler::HandleFailSafeTimerExpired() when sUdcInProgress: %d, starting UDC",
                    sUdcInProgress);
    chip::DeviceLayer::SystemLayer().StartTimer(
        chip::System::Clock::Milliseconds32(1),
        [](chip::System::Layer * aSystemLayer, void * aAppState) {
            ChipLogProgress(AppServer, "ChipDeviceEventHandler::HandleFailSafeTimerExpired() running OpenBasicCommissioningWindow");
            CHIP_ERROR err = chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow(
                chip::System::Clock::Seconds16(CastingPlayer::GetTargetCastingPlayer()->mCommissioningWindowTimeoutSec));
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer,
                             "ChipDeviceEventHandler::HandleFailSafeTimerExpired() Failed to OpenBasicCommissioningWindow "
                             "%" CHIP_ERROR_FORMAT,
                             err.Format());
                CastingPlayer::GetTargetCastingPlayer()->mOnCompleted(err, nullptr);
                return;
            }

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
            err = CastingPlayer::GetTargetCastingPlayer()->SendUserDirectedCommissioningRequest();
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer,
                             "ChipDeviceEventHandler::HandleFailSafeTimerExpired() Failed to SendUserDirectedCommissioningRequest "
                             "%" CHIP_ERROR_FORMAT,
                             err.Format());
                CastingPlayer::GetTargetCastingPlayer()->mOnCompleted(err, nullptr);
                return;
            }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
        },
        nullptr);
}

void ChipDeviceEventHandler::HandleBindingsChangedViaCluster(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg,
                                                             bool & runPostCommissioning, chip::NodeId & targetNodeId,
                                                             chip::FabricIndex & targetFabricIndex)
{
    ChipLogProgress(AppServer, "ChipDeviceEventHandler::HandleBindingsChangedViaCluster() called");

    if (CastingPlayer::GetTargetCastingPlayer()->IsConnected())
    {
        // re-use existing nodeId and fabricIndex
        ChipLogProgress(AppServer, "ChipDeviceEventHandler::HandleBindingsChangedViaCluster() already connected to video player");
        runPostCommissioning = true;
        targetNodeId         = CastingPlayer::GetTargetCastingPlayer()->GetNodeId();
        targetFabricIndex    = CastingPlayer::GetTargetCastingPlayer()->GetFabricIndex();
    }
    // handle the "re-commissioning" use case i.e. we received a kBindingsChangedViaCluster event when UDC was in progress (i.e. no
    // kCommissioningComplete received)
    else if (sUdcInProgress)
    {
        ChipLogProgress(AppServer,
                        "ChipDeviceEventHandler::HandleBindingsChangedViaCluster() UDC is in progress while handling "
                        "kBindingsChangedViaCluster with "
                        "fabricIndex: %d",
                        event->BindingsChanged.fabricIndex);
        sUdcInProgress = false;

        // find targetNodeId from binding table by matching the binding's fabricIndex with the accessing fabricIndex
        // received in BindingsChanged event
        for (const auto & binding : chip::BindingTable::GetInstance())
        {
            ChipLogProgress(AppServer,
                            "ChipDeviceEventHandler::HandleBindingsChangedViaCluster() Read cached binding type=%d fabrixIndex=%d "
                            "nodeId=0x" ChipLogFormatX64
                            " groupId=%d local endpoint=%d remote endpoint=%d cluster=" ChipLogFormatMEI,
                            binding.type, binding.fabricIndex, ChipLogValueX64(binding.nodeId), binding.groupId, binding.local,
                            binding.remote, ChipLogValueMEI(binding.clusterId.value_or(0)));
            if (binding.type == MATTER_UNICAST_BINDING && event->BindingsChanged.fabricIndex == binding.fabricIndex)
            {
                ChipLogProgress(AppServer,
                                "ChipDeviceEventHandler::HandleBindingsChangedViaCluster() Matched accessingFabricIndex with "
                                "nodeId=0x" ChipLogFormatX64,
                                ChipLogValueX64(binding.nodeId));
                targetNodeId         = binding.nodeId;
                targetFabricIndex    = binding.fabricIndex;
                runPostCommissioning = true;
                break;
            }
        }

        if (targetNodeId == 0 && runPostCommissioning == false)
        {
            ChipLogError(
                AppServer,
                "ChipDeviceEventHandler::HandleBindingsChangedViaCluster() accessingFabricIndex: %d did not match bindings",
                event->BindingsChanged.fabricIndex);
            CastingPlayer::GetTargetCastingPlayer()->mOnCompleted(CHIP_ERROR_INCORRECT_STATE,
                                                                  CastingPlayer::GetTargetCastingPlayer());
            return;
        }
    }
}

void ChipDeviceEventHandler::HandleCommissioningComplete(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg,
                                                         bool & runPostCommissioning, chip::NodeId & targetNodeId,
                                                         chip::FabricIndex & targetFabricIndex)
{
    ChipLogProgress(AppServer, "ChipDeviceEventHandler::HandleCommissioningComplete() called");
    sUdcInProgress       = false;
    targetNodeId         = event->CommissioningComplete.nodeId;
    targetFabricIndex    = event->CommissioningComplete.fabricIndex;
    runPostCommissioning = true;
}

CHIP_ERROR ChipDeviceEventHandler::SetUdcStatus(bool udcInProgress)
{
    ChipLogProgress(AppServer, "ChipDeviceEventHandler::SetUdcStatus() called with udcInProgress: %d", udcInProgress);
    if (sUdcInProgress == udcInProgress)
    {
        ChipLogError(AppServer, "ChipDeviceEventHandler::SetUdcStatus() UDC in progress state is already %d", sUdcInProgress);
        return CHIP_ERROR_INCORRECT_STATE;
    }
    sUdcInProgress = udcInProgress;
    return CHIP_NO_ERROR;
}

bool ChipDeviceEventHandler::isUdcInProgress()
{
    return sUdcInProgress;
}

}; // namespace support
}; // namespace casting
}; // namespace matter

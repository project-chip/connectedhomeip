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

#pragma once

#include "DeviceManager.h"
#include "IcdManager.h"
#include "StayActiveSender.h"

#include <app/icd/client/CheckInHandler.h>
#include <app/icd/client/DefaultCheckInDelegate.h>
#include <app/icd/client/DefaultICDClientStorage.h>
#include <bridge/include/FabricAdminDelegate.h>
#include <map>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <system/SystemClock.h>

namespace admin {

struct ScopedNodeIdHasher
{
    std::size_t operator()(const chip::ScopedNodeId & scopedNodeId) const
    {
        std::size_t h1 = std::hash<uint64_t>{}(scopedNodeId.GetFabricIndex());
        std::size_t h2 = std::hash<uint64_t>{}(scopedNodeId.GetNodeId());
        // Bitshifting h2 reduces collisions when fabricIndex == nodeId.
        return h1 ^ (h2 << 1);
    }
};

class FabricAdmin final : public bridge::FabricAdminDelegate, public PairingDelegate, public IcdManager::Delegate
{
public:
    static FabricAdmin & Instance() { return sInstance; }
    static chip::app::DefaultICDClientStorage & GetDefaultICDClientStorage() { return sICDClientStorage; }

    CHIP_ERROR Init();

    CHIP_ERROR OpenCommissioningWindow(chip::Controller::CommissioningWindowVerifierParams params,
                                       chip::FabricIndex fabricIndex) override;

    CHIP_ERROR
    CommissionRemoteBridge(chip::Controller::CommissioningWindowPasscodeParams params, chip::VendorId vendorId,
                           uint16_t productId) override;

    CHIP_ERROR KeepActive(chip::ScopedNodeId scopedNodeId, uint32_t stayActiveDurationMs, uint32_t timeoutMs) override;

    void OnCheckInCompleted(const chip::app::ICDClientInfo & clientInfo) override;

    void OnCommissioningComplete(chip::NodeId deviceId, CHIP_ERROR err) override;

    void ScheduleSendingKeepActiveOnCheckIn(chip::ScopedNodeId scopedNodeId, uint32_t stayActiveDurationMs, uint32_t timeoutMs);

private:
    struct KeepActiveDataForCheckIn
    {
        uint32_t mStayActiveDurationMs = 0;
        chip::System::Clock::Timestamp mRequestExpiryTimestamp;
    };

    struct KeepActiveWorkData
    {
        KeepActiveWorkData(FabricAdmin * fabricAdmin, chip::ScopedNodeId scopedNodeId, uint32_t stayActiveDurationMs,
                           uint32_t timeoutMs) :
            mFabricAdmin(fabricAdmin),
            mScopedNodeId(scopedNodeId), mStayActiveDurationMs(stayActiveDurationMs), mTimeoutMs(timeoutMs)
        {}

        FabricAdmin * mFabricAdmin;
        chip::ScopedNodeId mScopedNodeId;
        uint32_t mStayActiveDurationMs;
        uint32_t mTimeoutMs;
    };

    static void KeepActiveWork(intptr_t arg)
    {
        KeepActiveWorkData * data = reinterpret_cast<KeepActiveWorkData *>(arg);
        data->mFabricAdmin->ScheduleSendingKeepActiveOnCheckIn(data->mScopedNodeId, data->mStayActiveDurationMs, data->mTimeoutMs);
        chip::Platform::Delete(data);
    }

    // Modifications to mPendingCheckIn should be done on the MatterEventLoop thread
    // otherwise we would need a mutex protecting this data to prevent race as this
    // data is accessible by both RPC thread and Matter eventloop.
    std::unordered_map<chip::ScopedNodeId, KeepActiveDataForCheckIn, ScopedNodeIdHasher> mPendingCheckIn;

    static FabricAdmin sInstance;
    static chip::app::DefaultICDClientStorage sICDClientStorage;
    static chip::app::CheckInHandler sCheckInHandler;

    chip::NodeId mNodeId = chip::kUndefinedNodeId;
};

} // namespace admin

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

#include "FabricAdmin.h"

using namespace ::chip;

namespace admin {

namespace {

constexpr uint32_t kCommissionPrepareTimeMs = 500;

} // namespace

FabricAdmin FabricAdmin::sInstance;

FabricAdmin & FabricAdmin::Instance()
{
    if (!sInstance.mInitialized)
    {
        sInstance.Init();
    }
    return sInstance;
}

CHIP_ERROR FabricAdmin::OpenCommissioningWindow(Controller::CommissioningWindowVerifierParams params, FabricIndex fabricIndex)
{
    ScopedNodeId scopedNodeId(params.GetNodeId(), fabricIndex);
    uint32_t iterations              = params.GetIteration();
    uint16_t discriminator           = params.GetDiscriminator();
    uint16_t commissioningTimeoutSec = static_cast<uint16_t>(params.GetTimeout().count());

    // Log request details for debugging purposes
    ChipLogProgress(NotSpecified,
                    "Received OpenCommissioningWindow request: NodeId " ChipLogFormatX64
                    ", Timeout: %u, Iterations: %u, Discriminator: %u",
                    ChipLogValueX64(scopedNodeId.GetNodeId()), commissioningTimeoutSec, iterations, discriminator);

    // Open the device commissioning window with provided salt and verifier data
    DeviceManager::Instance().OpenDeviceCommissioningWindow(scopedNodeId, iterations, commissioningTimeoutSec, discriminator,
                                                            ByteSpan(params.GetSalt().data(), params.GetSalt().size()),
                                                            ByteSpan(params.GetVerifier().data(), params.GetVerifier().size()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR
FabricAdmin::CommissionRemoteBridge(Controller::CommissioningWindowPasscodeParams params, VendorId vendorId, uint16_t productId)
{
    char saltHex[Crypto::kSpake2p_Max_PBKDF_Salt_Length * 2 + 1];
    Encoding::BytesToHex(params.GetSalt().data(), params.GetSalt().size(), saltHex, sizeof(saltHex),
                         Encoding::HexFlags::kNullTerminate);

    ChipLogProgress(NotSpecified, "Received CommissionNode request");

    SetupPayload setupPayload = SetupPayload();

    setupPayload.setUpPINCode = params.GetSetupPIN();
    setupPayload.version      = 0;
    setupPayload.vendorID     = vendorId;
    setupPayload.productID    = productId;
    setupPayload.rendezvousInformation.SetValue(RendezvousInformationFlag::kOnNetwork);

    SetupDiscriminator discriminator{};
    discriminator.SetLongValue(params.GetDiscriminator());
    setupPayload.discriminator = discriminator;

    QRCodeSetupPayloadGenerator generator(setupPayload);
    std::string code;
    CHIP_ERROR err = generator.payloadBase38RepresentationWithAutoTLVBuffer(code);

    if (err == CHIP_NO_ERROR)
    {
        NodeId nodeId = DeviceManager::Instance().GetNextAvailableNodeId();

        // After responding with RequestCommissioningApproval to the node where the client initiated the
        // RequestCommissioningApproval, you need to wait for it to open a commissioning window on its bridge.
        usleep(kCommissionPrepareTimeMs * 1000);

        DeviceManager::Instance().PairRemoteDevice(nodeId, code.c_str());
    }
    else
    {
        ChipLogError(NotSpecified, "Unable to generate pairing code for setup payload: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricAdmin::KeepActive(ScopedNodeId scopedNodeId, uint32_t stayActiveDurationMs, uint32_t timeoutMs)
{
    ChipLogProgress(NotSpecified, "Received KeepActive request: Id[%d, 0x" ChipLogFormatX64 "], %u", scopedNodeId.GetFabricIndex(),
                    ChipLogValueX64(scopedNodeId.GetNodeId()), stayActiveDurationMs);

    KeepActiveWorkData * data = Platform::New<KeepActiveWorkData>(this, scopedNodeId, stayActiveDurationMs, timeoutMs);
    VerifyOrReturnError(data != nullptr, CHIP_ERROR_NO_MEMORY);

    DeviceLayer::PlatformMgr().ScheduleWork(KeepActiveWork, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

void FabricAdmin::ScheduleSendingKeepActiveOnCheckIn(ScopedNodeId scopedNodeId, uint32_t stayActiveDurationMs, uint32_t timeoutMs)
{
    // Accessing mPendingCheckIn should only be done while holding ChipStackLock
    assertChipStackLockedByCurrentThread();

    auto timeNow                             = System::SystemClock().GetMonotonicTimestamp();
    System::Clock::Timestamp expiryTimestamp = timeNow + System::Clock::Milliseconds64(timeoutMs);
    KeepActiveDataForCheckIn checkInData     = { .mStayActiveDurationMs   = stayActiveDurationMs,
                                                 .mRequestExpiryTimestamp = expiryTimestamp };

    auto it = mPendingCheckIn.find(scopedNodeId);
    if (it != mPendingCheckIn.end())
    {
        checkInData.mStayActiveDurationMs   = std::max(checkInData.mStayActiveDurationMs, it->second.mStayActiveDurationMs);
        checkInData.mRequestExpiryTimestamp = std::max(checkInData.mRequestExpiryTimestamp, it->second.mRequestExpiryTimestamp);
    }

    mPendingCheckIn[scopedNodeId] = checkInData;
}

} // namespace admin

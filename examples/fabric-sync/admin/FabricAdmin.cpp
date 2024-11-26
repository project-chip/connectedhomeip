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
#include <AppMain.h>
#include <CommissionerMain.h>
#include <bridge/include/FabricBridge.h>
#include <controller/CHIPDeviceControllerFactory.h>

using namespace ::chip;

namespace admin {

namespace {

constexpr uint32_t kCommissionPrepareTimeMs = 500;

} // namespace

FabricAdmin FabricAdmin::sInstance;
app::DefaultICDClientStorage FabricAdmin::sICDClientStorage;
app::CheckInHandler FabricAdmin::sCheckInHandler;

CHIP_ERROR FabricAdmin::Init()
{
    IcdManager::Instance().SetDelegate(&sInstance);

    ReturnLogErrorOnFailure(sICDClientStorage.Init(GetPersistentStorageDelegate(), GetSessionKeystore()));

    auto engine = chip::app::InteractionModelEngine::GetInstance();
    VerifyOrReturnError(engine != nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnLogErrorOnFailure(IcdManager::Instance().Init(&sICDClientStorage, engine));
    ReturnLogErrorOnFailure(sCheckInHandler.Init(Controller::DeviceControllerFactory::GetInstance().GetSystemState()->ExchangeMgr(),
                                                 &sICDClientStorage, &IcdManager::Instance(), engine));

    return CHIP_NO_ERROR;
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
        mNodeId = DeviceManager::Instance().GetNextAvailableNodeId();

        // After responding with RequestCommissioningApproval to the node where the client initiated the
        // RequestCommissioningApproval, you need to wait for it to open a commissioning window on its bridge.
        usleep(kCommissionPrepareTimeMs * 1000);

        PairingManager::Instance().SetPairingDelegate(this);
        err = PairingManager::Instance().PairDeviceWithCode(mNodeId, code.c_str());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified,
                         "Failed to commission remote bridge device: Node ID " ChipLogFormatX64 " with error: %" CHIP_ERROR_FORMAT,
                         ChipLogValueX64(mNodeId), err.Format());
        }
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

void FabricAdmin::OnCheckInCompleted(const app::ICDClientInfo & clientInfo)
{
    // Accessing mPendingCheckIn should only be done while holding ChipStackLock
    assertChipStackLockedByCurrentThread();
    ScopedNodeId scopedNodeId = clientInfo.peer_node;
    auto it                   = mPendingCheckIn.find(scopedNodeId);
    VerifyOrReturn(it != mPendingCheckIn.end());

    KeepActiveDataForCheckIn checkInData = it->second;
    // Removed from pending map as check-in from this node has occured and we will handle the pending KeepActive
    // request.
    mPendingCheckIn.erase(scopedNodeId);

    auto timeNow = System::SystemClock().GetMonotonicTimestamp();
    if (timeNow > checkInData.mRequestExpiryTimestamp)
    {
        ChipLogError(NotSpecified,
                     "ICD check-in for device we have been waiting, came after KeepActive expiry. Request dropped for ID: "
                     "[%d:0x " ChipLogFormatX64 "]",
                     scopedNodeId.GetFabricIndex(), ChipLogValueX64(scopedNodeId.GetNodeId()));
        return;
    }

    // TODO https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/10448. Spec does
    // not define what to do if we fail to send the StayActiveRequest. We are assuming that any
    // further attempts to send a StayActiveRequest will result in a similar failure. Because
    // there is no mechanism for us to communicate with the client that sent out the KeepActive
    // command that there was a failure, we simply fail silently. After spec issue is
    // addressed, we can implement what spec defines here.
    auto onDone = [=](uint32_t promisedActiveDuration) {
        bridge::FabricBridge::Instance().ActiveChanged(scopedNodeId, promisedActiveDuration);
    };
    CHIP_ERROR err = StayActiveSender::SendStayActiveCommand(checkInData.mStayActiveDurationMs, clientInfo.peer_node,
                                                             app::InteractionModelEngine::GetInstance(), onDone);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to send StayActive command %s", err.AsString());
    }
}

void FabricAdmin::OnCommissioningComplete(NodeId deviceId, CHIP_ERROR err)
{
    if (mNodeId != deviceId)
    {
        ChipLogError(NotSpecified, "Tried to pair a non-bridge device (0x:" ChipLogFormatX64 ") with result: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(deviceId), err.Format());
        return;
    }

    if (err == CHIP_NO_ERROR)
    {
        DeviceManager::Instance().SetRemoteBridgeNodeId(deviceId);
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to pair bridge device (0x:" ChipLogFormatX64 ") with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(deviceId), err.Format());
    }

    mNodeId = kUndefinedNodeId;
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

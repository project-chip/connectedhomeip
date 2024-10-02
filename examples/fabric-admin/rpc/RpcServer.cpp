/*
 *
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

#include "pw_rpc/server.h"
#include "pw_rpc_system_server/rpc_server.h"
#include "pw_rpc_system_server/socket.h"

#include <map>
#include <thread>

#include "RpcClient.h"
#include <commands/common/IcdManager.h>
#include <commands/common/StayActiveSender.h>
#include <commands/fabric-sync/FabricSyncCommand.h>
#include <commands/interactive/InteractiveCommands.h>
#include <device_manager/DeviceManager.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <system/SystemClock.h>

#if defined(PW_RPC_FABRIC_ADMIN_SERVICE) && PW_RPC_FABRIC_ADMIN_SERVICE
#include "pigweed/rpc_services/FabricAdmin.h"
#endif

using namespace ::chip;

namespace {

#if defined(PW_RPC_FABRIC_ADMIN_SERVICE) && PW_RPC_FABRIC_ADMIN_SERVICE

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

class FabricAdmin final : public rpc::FabricAdmin, public IcdManager::Delegate
{
public:
    void OnCheckInCompleted(const app::ICDClientInfo & clientInfo) override
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
        auto onDone    = [=](uint32_t promisedActiveDuration) { ActiveChanged(scopedNodeId, promisedActiveDuration); };
        CHIP_ERROR err = StayActiveSender::SendStayActiveCommand(checkInData.mStayActiveDurationMs, clientInfo.peer_node,
                                                                 app::InteractionModelEngine::GetInstance(), onDone);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Failed to send StayActive command %s", err.AsString());
        }
    }

    pw::Status OpenCommissioningWindow(const chip_rpc_DeviceCommissioningWindowInfo & request,
                                       chip_rpc_OperationStatus & response) override
    {
        VerifyOrReturnValue(request.has_id, pw::Status::InvalidArgument());
        // TODO(#35875): OpenDeviceCommissioningWindow uses the same controller every time and doesn't currently accept
        // FabricIndex. For now we are dropping fabric index from the scoped node id.
        NodeId nodeId                    = request.id.node_id;
        uint32_t commissioningTimeoutSec = request.commissioning_timeout;
        uint32_t iterations              = request.iterations;
        uint16_t discriminator           = request.discriminator;

        // Log the request details for debugging
        ChipLogProgress(NotSpecified,
                        "Received OpenCommissioningWindow request: NodeId 0x%lx, Timeout: %u, Iterations: %u, Discriminator: %u",
                        static_cast<unsigned long>(nodeId), commissioningTimeoutSec, iterations, discriminator);

        // Open the device commissioning window using raw binary data for salt and verifier
        DeviceMgr().OpenDeviceCommissioningWindow(nodeId, commissioningTimeoutSec, iterations, discriminator,
                                                  ByteSpan(request.salt.bytes, request.salt.size),
                                                  ByteSpan(request.verifier.bytes, request.verifier.size));

        response.success = true;

        return pw::OkStatus();
    }

    pw::Status CommissionNode(const chip_rpc_DeviceCommissioningInfo & request, pw_protobuf_Empty & response) override
    {
        char saltHex[Crypto::kSpake2p_Max_PBKDF_Salt_Length * 2 + 1];
        Encoding::BytesToHex(request.salt.bytes, request.salt.size, saltHex, sizeof(saltHex), Encoding::HexFlags::kNullTerminate);

        ChipLogProgress(NotSpecified, "Received CommissionNode request");

        SetupPayload setupPayload = SetupPayload();

        setupPayload.setUpPINCode = request.setup_pin;
        setupPayload.version      = 0;
        setupPayload.vendorID     = request.vendor_id;
        setupPayload.productID    = request.product_id;
        setupPayload.rendezvousInformation.SetValue(RendezvousInformationFlag::kOnNetwork);

        SetupDiscriminator discriminator{};
        discriminator.SetLongValue(request.discriminator);
        setupPayload.discriminator = discriminator;

        QRCodeSetupPayloadGenerator generator(setupPayload);
        std::string code;
        CHIP_ERROR error = generator.payloadBase38RepresentationWithAutoTLVBuffer(code);

        if (error == CHIP_NO_ERROR)
        {
            NodeId nodeId = DeviceMgr().GetNextAvailableNodeId();

            // After responding with RequestCommissioningApproval to the node where the client initiated the
            // RequestCommissioningApproval, you need to wait for it to open a commissioning window on its bridge.
            usleep(kCommissionPrepareTimeMs * 1000);

            DeviceMgr().PairRemoteDevice(nodeId, code.c_str());
        }
        else
        {
            ChipLogError(NotSpecified, "Unable to generate pairing code for setup payload: %" CHIP_ERROR_FORMAT, error.Format());
        }

        return pw::OkStatus();
    }

    pw::Status KeepActive(const chip_rpc_KeepActiveParameters & request, pw_protobuf_Empty & response) override
    {
        VerifyOrReturnValue(request.has_id, pw::Status::InvalidArgument());
        ScopedNodeId scopedNodeId(request.id.node_id, request.id.fabric_index);
        ChipLogProgress(NotSpecified, "Received KeepActive request: Id[%d, 0x" ChipLogFormatX64 "], %u",
                        scopedNodeId.GetFabricIndex(), ChipLogValueX64(scopedNodeId.GetNodeId()), request.stay_active_duration_ms);

        KeepActiveWorkData * data =
            Platform::New<KeepActiveWorkData>(this, scopedNodeId, request.stay_active_duration_ms, request.timeout_ms);
        VerifyOrReturnValue(data, pw::Status::Internal());
        DeviceLayer::PlatformMgr().ScheduleWork(KeepActiveWork, reinterpret_cast<intptr_t>(data));
        return pw::OkStatus();
    }

    void ScheduleSendingKeepActiveOnCheckIn(ScopedNodeId scopedNodeId, uint32_t stayActiveDurationMs, uint32_t timeoutMs)
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

private:
    struct KeepActiveDataForCheckIn
    {
        uint32_t mStayActiveDurationMs = 0;
        System::Clock::Timestamp mRequestExpiryTimestamp;
    };

    struct KeepActiveWorkData
    {
        KeepActiveWorkData(FabricAdmin * fabricAdmin, ScopedNodeId scopedNodeId, uint32_t stayActiveDurationMs,
                           uint32_t timeoutMs) :
            mFabricAdmin(fabricAdmin),
            mScopedNodeId(scopedNodeId), mStayActiveDurationMs(stayActiveDurationMs), mTimeoutMs(timeoutMs)
        {}

        FabricAdmin * mFabricAdmin;
        ScopedNodeId mScopedNodeId;
        uint32_t mStayActiveDurationMs;
        uint32_t mTimeoutMs;
    };

    static void KeepActiveWork(intptr_t arg)
    {
        KeepActiveWorkData * data = reinterpret_cast<KeepActiveWorkData *>(arg);
        data->mFabricAdmin->ScheduleSendingKeepActiveOnCheckIn(data->mScopedNodeId, data->mStayActiveDurationMs, data->mTimeoutMs);
        Platform::Delete(data);
    }

    // Modifications to mPendingCheckIn should be done on the MatterEventLoop thread
    // otherwise we would need a mutex protecting this data to prevent race as this
    // data is accessible by both RPC thread and Matter eventloop.
    std::unordered_map<ScopedNodeId, KeepActiveDataForCheckIn, ScopedNodeIdHasher> mPendingCheckIn;
};

FabricAdmin fabric_admin_service;
#endif // defined(PW_RPC_FABRIC_ADMIN_SERVICE) && PW_RPC_FABRIC_ADMIN_SERVICE

void RegisterServices(pw::rpc::Server & server)
{
#if defined(PW_RPC_FABRIC_ADMIN_SERVICE) && PW_RPC_FABRIC_ADMIN_SERVICE
    server.RegisterService(fabric_admin_service);
    IcdManager::Instance().SetDelegate(&fabric_admin_service);
#endif
}

} // namespace

void RunRpcService()
{
    pw::rpc::system_server::Init();
    RegisterServices(pw::rpc::system_server::Server());
    pw::rpc::system_server::Start();
}

void InitRpcServer(uint16_t rpcServerPort)
{
    pw::rpc::system_server::set_socket_port(rpcServerPort);
    std::thread rpc_service(RunRpcService);
    rpc_service.detach();
}

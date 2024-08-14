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
#include <system/SystemClock.h>

#if defined(PW_RPC_FABRIC_ADMIN_SERVICE) && PW_RPC_FABRIC_ADMIN_SERVICE
#include "pigweed/rpc_services/FabricAdmin.h"
#endif

using namespace ::chip;

namespace {

#if defined(PW_RPC_FABRIC_ADMIN_SERVICE) && PW_RPC_FABRIC_ADMIN_SERVICE

class FabricAdmin final : public rpc::FabricAdmin, public IcdManager::Delegate
{
public:
    void OnCheckInCompleted(const chip::app::ICDClientInfo & clientInfo) override
    {
        chip::NodeId nodeId = clientInfo.peer_node.GetNodeId();
        auto it             = mPendingKeepActiveTimesMs.find(nodeId);
        VerifyOrReturn(it != mPendingKeepActiveTimesMs.end());
        // TODO(#33221): We also need a mechanism here to drop KeepActive
        // request if they were recieved over 60 mins ago.
        uint32_t stayActiveDurationMs = it->second;

        // TODO(#33221): If there is a failure in sending the message this request just gets dropped.
        // Work to see if there should be update to spec on whether some sort of failure later on
        // Should be indicated in some manner, or identify a better recovery mechanism here.
        mPendingKeepActiveTimesMs.erase(nodeId);

        auto onDone    = [=](uint32_t promisedActiveDuration) { ActiveChanged(nodeId, promisedActiveDuration); };
        CHIP_ERROR err = StayActiveSender::SendStayActiveCommand(stayActiveDurationMs, clientInfo.peer_node,
                                                                 chip::app::InteractionModelEngine::GetInstance(), onDone);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Failed to send StayActive command %s", err.AsString());
        }
    }

    pw::Status OpenCommissioningWindow(const chip_rpc_DeviceCommissioningWindowInfo & request,
                                       chip_rpc_OperationStatus & response) override
    {
        NodeId nodeId                 = request.node_id;
        uint32_t commissioningTimeout = request.commissioning_timeout;
        uint32_t iterations           = request.iterations;
        uint32_t discriminator        = request.discriminator;

        char saltHex[Crypto::kSpake2p_Max_PBKDF_Salt_Length * 2 + 1];
        Encoding::BytesToHex(request.salt.bytes, request.salt.size, saltHex, sizeof(saltHex), Encoding::HexFlags::kNullTerminate);

        char verifierHex[Crypto::kSpake2p_VerifierSerialized_Length * 2 + 1];
        Encoding::BytesToHex(request.verifier.bytes, request.verifier.size, verifierHex, sizeof(verifierHex),
                             Encoding::HexFlags::kNullTerminate);

        ChipLogProgress(NotSpecified, "Received OpenCommissioningWindow request: 0x%lx", nodeId);

        DeviceMgr().OpenDeviceCommissioningWindow(nodeId, commissioningTimeout, iterations, discriminator, saltHex, verifierHex);

        response.success = true;

        return pw::OkStatus();
    }

    pw::Status ReverseCommissionBridge(const chip_rpc_DeviceCommissioningWindowInfo & request,
                                       pw_protobuf_Empty & response) override
    {
        ChipLogProgress(NotSpecified, "Received ReverseCommissionBridge request");
		
		// TODO: push the command to pair the bridge

        return pw::OkStatus();
    }

    pw::Status KeepActive(const chip_rpc_KeepActiveParameters & request, pw_protobuf_Empty & response) override
    {
        ChipLogProgress(NotSpecified, "Received KeepActive request: 0x%lx, %u", request.node_id, request.stay_active_duration_ms);
        // TODO(#33221): We should really be using ScopedNode, but that requires larger fix in communication between
        // fabric-admin and fabric-bridge. For now we make the assumption that there is only one fabric used by
        // fabric-admin.
        KeepActiveWorkData * data = chip::Platform::New<KeepActiveWorkData>(this, request.node_id, request.stay_active_duration_ms);
        VerifyOrReturnValue(data, pw::Status::Internal());
        chip::DeviceLayer::PlatformMgr().ScheduleWork(KeepActiveWork, reinterpret_cast<intptr_t>(data));
        return pw::OkStatus();
    }

    void ScheduleSendingKeepActiveOnCheckIn(chip::NodeId nodeId, uint32_t stayActiveDurationMs)
    {
        mPendingKeepActiveTimesMs[nodeId] = stayActiveDurationMs;
    }

private:
    struct KeepActiveWorkData
    {
        KeepActiveWorkData(FabricAdmin * fabricAdmin, chip::NodeId nodeId, uint32_t stayActiveDurationMs) :
            mFabricAdmin(fabricAdmin), mNodeId(nodeId), mStayActiveDurationMs(stayActiveDurationMs)
        {}

        FabricAdmin * mFabricAdmin;
        chip::NodeId mNodeId;
        uint32_t mStayActiveDurationMs;
    };

    static void KeepActiveWork(intptr_t arg)
    {
        KeepActiveWorkData * data = reinterpret_cast<KeepActiveWorkData *>(arg);
        data->mFabricAdmin->ScheduleSendingKeepActiveOnCheckIn(data->mNodeId, data->mStayActiveDurationMs);
        chip::Platform::Delete(data);
    }

    // Modifications to mPendingKeepActiveTimesMs should be done on the MatterEventLoop thread
    std::map<chip::NodeId, uint32_t> mPendingKeepActiveTimesMs;
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

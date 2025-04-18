#include "pigweed/rpc_services/JointFabric.h"

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;

namespace joint_fabric_service {

::pw::Status JointFabric::TransferOwnership(const ::OwnershipContext & request, ::pw_protobuf_Empty & response)
{
    ChipLogProgress(chipTool, "Ownership Transfer for NodeId: " ChipLogFormatX64, ChipLogValueX64(request.node_id));

    OwnershipTransferContext * data = Platform::New<OwnershipTransferContext>(request.node_id);
    VerifyOrReturnValue(data, pw::Status::Internal());
    DeviceLayer::PlatformMgr().ScheduleWork(FinalizeCommissioningWork, reinterpret_cast<intptr_t>(data));

    return pw::OkStatus();
}

} // namespace joint_fabric_service

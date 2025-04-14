#include "pigweed/rpc_services/JointFabric.h"
#include <lib/support/logging/CHIPLogging.h>
#include <app/server/Server.h>
#include <app/server/CommissioningWindowManager.h>

#include "JFAManager.h"

using namespace chip;

namespace joint_fabric_service {

::pw::Status JointFabric::TransferOwnership( const ::OwnershipContext& request, ::pw_protobuf_Empty& response) {
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint64_t nodeId = request.node_id;

    ChipLogProgress(chipTool, "Ownership Transfer for NodeId: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    err = JFAMgr().FinalizeCommissioning(nodeId);

    if (err == CHIP_NO_ERROR)
    {
        return pw::OkStatus();
    }
    else
    {
        return pw::Status::Aborted();
    }
}

} // namespace joint_fabric_service

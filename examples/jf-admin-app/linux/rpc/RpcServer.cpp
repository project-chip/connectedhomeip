#include "pigweed/rpc_services/JointFabric.h"

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;

namespace joint_fabric_service {

::pw::Status JointFabric::TransferOwnership(const ::OwnershipContext & request, ::pw_protobuf_Empty & response)
{
    ChipLogProgress(JointFabric, "Ownership Transfer for NodeId: 0x" ChipLogFormatX64 ", jcm=%d", ChipLogValueX64(request.node_id),
                    request.jcm);

    if (request.jcm && (Crypto::kP256_PublicKey_Length != request.trustedIcacPublicKeyB.size))
    {
        return pw::Status::OutOfRange();
        ChipLogError(JointFabric, "Invalid ICAC Public Key Size");
    }

    for (size_t i = 0; i < Crypto::kP256_PublicKey_Length; ++i)
    {
        ChipLogProgress(JointFabric, "trustedIcacPublicKeyB[%li] = %02X", i, request.trustedIcacPublicKeyB.bytes[i]);
    }

    OwnershipTransferContext * data = Platform::New<OwnershipTransferContext>(
        request.node_id, request.jcm, ByteSpan(request.trustedIcacPublicKeyB.bytes, request.trustedIcacPublicKeyB.size));
    VerifyOrReturnValue(data, pw::Status::Internal());
    DeviceLayer::PlatformMgr().ScheduleWork(FinalizeCommissioningWork, reinterpret_cast<intptr_t>(data));

    return pw::OkStatus();
}

} // namespace joint_fabric_service

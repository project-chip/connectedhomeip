#include "JFAManager.h"
#include "joint_fabric_service/joint_fabric_service.rpc.pb.h"

#include <crypto/CHIPCryptoPAL.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::Crypto;

namespace joint_fabric_service {

class JointFabric : public pw_rpc::nanopb::JointFabric::Service<JointFabric>
{
public:
    ::pw::Status TransferOwnership(const ::OwnershipContext & request, ::pw_protobuf_Empty & response);

private:
    struct OwnershipTransferContext
    {
        OwnershipTransferContext(uint64_t nodeId, bool jcm) : mNodeId(nodeId), mJCM(jcm){}

        uint64_t mNodeId;
        bool mJCM;
    };

    static void FinalizeCommissioningWork(intptr_t arg)
    {
        OwnershipTransferContext * data = reinterpret_cast<OwnershipTransferContext *>(arg);
        JFAMgr().FinalizeCommissioning(data->mNodeId, data->mJCM);
        Platform::Delete(data);
    }
};

} // namespace joint_fabric_service

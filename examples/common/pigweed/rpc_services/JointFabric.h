#include "JFAManager.h"
#include "JFARpc.h"
#include "joint_fabric_service/joint_fabric_service.rpc.pb.h"

#include <crypto/CHIPCryptoPAL.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::Crypto;

namespace joint_fabric_service {

class JointFabric : public pw_rpc::nanopb::JointFabric::Service<JointFabric>, public JFARpc
{
public:
    /*RPC from jfc-client to jfa-server */
    ::pw::Status TransferOwnership(const ::OwnershipContext & request, ::pw_protobuf_Empty & response);
    void GetStream(const ::pw_protobuf_Empty & request, ServerWriter<::RequestOptions> & writer);
    ::pw::Status ResponseStream(const ::Response & ResponseBytes, ::pw_protobuf_Empty & response);

    /* JFARpc overrides */
    CHIP_ERROR GetICACCSRForJF(MutableByteSpan & icacCSR);
    void CloseStreams();

private:
    struct OwnershipTransferContext
    {
        OwnershipTransferContext(uint64_t nodeId, bool jcm, ByteSpan trustedIcacPublicKeyB,
                                 uint16_t peerAdminJFAdminClusterEndpointId) :
            mNodeId(nodeId),
            mJCM(jcm), mPeerAdminJFAdminClusterEndpointId(peerAdminJFAdminClusterEndpointId)
        {
            memcpy(keyRawBytes, trustedIcacPublicKeyB.data(), kP256_PublicKey_Length);
            mTrustedIcacPublicKeyBSerialized = keyRawBytes;
        }

        uint64_t mNodeId;
        bool mJCM;

        uint8_t keyRawBytes[kP256_PublicKey_Length] = { 0 };
        P256PublicKey mTrustedIcacPublicKeyBSerialized;

        uint16_t mPeerAdminJFAdminClusterEndpointId;
    };

    static void FinalizeCommissioningWork(intptr_t arg)
    {
        OwnershipTransferContext * data = reinterpret_cast<OwnershipTransferContext *>(arg);
        JFAMgr().FinalizeCommissioning(data->mNodeId, data->mJCM, data->mTrustedIcacPublicKeyBSerialized,
                                       data->mPeerAdminJFAdminClusterEndpointId);
        Platform::Delete(data);
    }

    ServerWriter<::RequestOptions> rpcGetStream;
};

} // namespace joint_fabric_service

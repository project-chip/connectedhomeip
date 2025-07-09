#include "JFAManager.h"
#include "JFARpc.h"
#include "joint_fabric_service/joint_fabric_service.rpc.pb.h"

#include <crypto/CHIPCryptoPAL.h>
#include <platform/CHIPDeviceLayer.h>

namespace joint_fabric_service {

class JointFabric : public pw_rpc::nanopb::JointFabric::Service<JointFabric>, public JFARpc
{
public:
    /*RPC from jfc-client to jfa-server */
    ::pw::Status TransferOwnership(const ::OwnershipContext & request, ::pw_protobuf_Empty & response);
    void GetICACCSR(const ::pw_protobuf_Empty & request, ServerWriter<::ICACCSROptions> & writer);
    ::pw::Status ReplyWithICACCSR(const ::ICACCSR & ICACCSRBytes, ::pw_protobuf_Empty & response);

    /* JFARpc overrides */
    CHIP_ERROR GetICACCSRForJF(uint64_t anchorFabricId, MutableByteSpan & icacCSR);
    void CloseStreams();

private:
    struct OwnershipTransferContext
    {
        OwnershipTransferContext(uint64_t nodeId, bool jcm, chip::ByteSpan trustedIcacPublicKeyB) : mNodeId(nodeId), mJCM(jcm)
        {
            VerifyOrDie(trustedIcacPublicKeyB.size() == chip::Crypto::kP256_PublicKey_Length);
            memcpy(keyRawBytes, trustedIcacPublicKeyB.data(), chip::Crypto::kP256_PublicKey_Length);
            mTrustedIcacPublicKeyBSerialized = keyRawBytes;
        }

        uint64_t mNodeId;
        bool mJCM;

        uint8_t keyRawBytes[chip::Crypto::kP256_PublicKey_Length] = { 0 };
        chip::Crypto::P256PublicKey mTrustedIcacPublicKeyBSerialized;
    };

    static void FinalizeCommissioningWork(intptr_t arg)
    {
        OwnershipTransferContext * data = reinterpret_cast<OwnershipTransferContext *>(arg);
        chip::JFAMgr().FinalizeCommissioning(data->mNodeId, data->mJCM, data->mTrustedIcacPublicKeyBSerialized);
        chip::Platform::Delete(data);
    }

    ServerWriter<::ICACCSROptions> rpcStreamGetICACCSR;
};

} // namespace joint_fabric_service

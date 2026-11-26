#include "JFAManagerBase.h"
#include "JFARpc.h"
#include "joint_fabric_service/joint_fabric_service.rpc.pb.h"

#include <crypto/CHIPCryptoPAL.h>
#include <platform/CHIPDeviceLayer.h>

namespace joint_fabric_service {

class JointFabric : public pw_rpc::nanopb::JointFabric::Service<JointFabric>, public chip::JFARpc
{

public:
    /*RPC from jfc-client to jfa-server */
    ::pw::Status TransferOwnership(const ::OwnershipContext & request, ::pw_protobuf_Empty & response);
    void GetStream(const ::pw_protobuf_Empty & request, ServerWriter<::RequestOptions> & writer);
    ::pw::Status ResponseStream(const ::Response & ResponseBytes, ::pw_protobuf_Empty & response);

    /* JFARpc overrides */
    CHIP_ERROR GetICACCSRForJF(chip::MutableByteSpan & icacCSR) override;
    CHIP_ERROR GetCrossSignedICAC(chip::FabricId anchorFabricId, chip::ByteSpan csr, chip::MutableByteSpan & icac) override;
    CHIP_ERROR PushJFMaterials(chip::ByteSpan crossSignedICAC, chip::ByteSpan anchorRCAC, chip::ByteSpan anchorIPK) override;
    // Tell JFC-B to discard any JF materials received in a failed JCM flow.
    CHIP_ERROR ClearJFMaterials() override;
    // Ask JFC-B to issue a Fabric-A NOC from a Fabric-B device CSR (device migration).
    // JFC-B also returns the cross-signed ICAC, Fabric-A RCAC and IPK it holds.
    CHIP_ERROR GetNOCForDevice(chip::NodeId deviceNodeId, chip::ByteSpan csr, chip::MutableByteSpan & noc,
                               chip::MutableByteSpan & icac, chip::MutableByteSpan & rcac, chip::MutableByteSpan & ipk) override;
    void CloseStreams() override;

private:
    struct OwnershipTransferContext
    {
        OwnershipTransferContext(uint64_t nodeId, bool jcm, chip::ByteSpan trustedIcacPublicKeyB,
                                 uint16_t peerAdminJFAdminClusterEndpointId) :
            mNodeId(nodeId),
            mJCM(jcm), mPeerAdminJFAdminClusterEndpointId(peerAdminJFAdminClusterEndpointId)
        {
            memcpy(keyRawBytes, trustedIcacPublicKeyB.data(), chip::Crypto::kP256_PublicKey_Length);
            mTrustedIcacPublicKeyBSerialized = keyRawBytes;
        }

        uint64_t mNodeId;
        bool mJCM;
        uint8_t keyRawBytes[chip::Crypto::kP256_PublicKey_Length] = { 0 };
        chip::Crypto::P256PublicKey mTrustedIcacPublicKeyBSerialized;

        uint16_t mPeerAdminJFAdminClusterEndpointId;
    };

    static void FinalizeCommissioningWork(intptr_t arg)
    {
        OwnershipTransferContext * data = reinterpret_cast<OwnershipTransferContext *>(arg);
        TEMPORARY_RETURN_IGNORED chip::JFAMgr().FinalizeCommissioning(
            data->mNodeId, data->mJCM, data->mTrustedIcacPublicKeyBSerialized, data->mPeerAdminJFAdminClusterEndpointId);
        chip::Platform::Delete(data);
    }

    ServerWriter<::RequestOptions> rpcGetStream;
};

} // namespace joint_fabric_service

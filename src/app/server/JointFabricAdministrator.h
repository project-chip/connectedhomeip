/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>

#include <optional>

namespace chip {
namespace app {

class JointFabricAdministrator
{
public:
    class Delegate
    {
    public:
        Delegate() {}
        virtual ~Delegate() {}

        virtual CHIP_ERROR GetIcacCsr(MutableByteSpan & icacCsr) { return CHIP_NO_ERROR; }
        virtual CHIP_ERROR StoreCrossSignedICAC(chip::ByteSpan icac) { return CHIP_NO_ERROR; }
        // Called after AddICAC succeeds so the device can forward the JF materials
        // (cross-signed ICAC, anchor RCAC, anchor IPK) to its paired commissioner
        // via the local RPC channel.
        virtual CHIP_ERROR PushJFMaterials(chip::ByteSpan icac, chip::ByteSpan rcac, chip::ByteSpan ipk) { return CHIP_NO_ERROR; }
        // Called on fail-safe expiry or commissioning abort so the device can
        // tell its paired commissioner (JFC-B) to discard any JF materials it
        // received earlier in this attempt.
        virtual CHIP_ERROR ClearJFMaterials() { return CHIP_NO_ERROR; }

        // Called after AnnounceJointFabricAdministrator succeeds so the device
        // can record the NodeId of the peer JFA (JFA-A) that initiated JCM, along
        // with the fabric index on which JFA-A invoked the command (the joint
        // fabric). The device must reach JFA-A on that same fabric index, not on
        // its own locally-commissioned fabric index.
        // Cleared when the fail-safe timer expires or commissioning is aborted.
        virtual void SetPeerJFANodeId(chip::NodeId nodeId, chip::FabricIndex fabricIndex)
        {
            (void) nodeId;
            (void) fabricIndex;
        }
        virtual void ClearPeerJFANodeId() {}
    };

    static JointFabricAdministrator & GetInstance()
    {
        static JointFabricAdministrator sInstance;
        return sInstance;
    }

    void SetPeerJFAdminClusterEndpointId(chip::EndpointId peerJFAdminClusterEndpointId)
    {
        mPeerJFAdminClusterEndpointId = peerJFAdminClusterEndpointId;
    }

    void SetVidVerificationForFabric(chip::FabricIndex fabricIndex) { mVidVerificationFabricIndex = fabricIndex; }
    void ClearVidVerificationForFabric() { mVidVerificationFabricIndex.reset(); }

    /**
     * Clear cached VID-verification state when its fabric is removed.
     */
    void OnFabricRemoved(chip::FabricIndex removedFabricIndex)
    {
        if (mVidVerificationFabricIndex.has_value() && (mVidVerificationFabricIndex.value() == removedFabricIndex))
        {
            mVidVerificationFabricIndex.reset();
        }
    }
    bool WasVidVerificationExecutedForFabric(chip::FabricIndex fabricIndex) const
    {
        return mVidVerificationFabricIndex.has_value() && (mVidVerificationFabricIndex.value() == fabricIndex);
    }

    CHIP_ERROR SetDelegate(JointFabricAdministrator::Delegate * delegate)
    {
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        mDelegate = delegate;

        return CHIP_NO_ERROR;
    }

    chip::EndpointId GetPeerJFAdminClusterEndpointId() const { return mPeerJFAdminClusterEndpointId; }

    JointFabricAdministrator::Delegate * GetDelegate() { return mDelegate; }

private:
    chip::EndpointId mPeerJFAdminClusterEndpointId = chip::kInvalidEndpointId;
    std::optional<chip::FabricIndex> mVidVerificationFabricIndex;
    JointFabricAdministrator::Delegate * mDelegate = nullptr;
};

} // namespace app
} // namespace chip

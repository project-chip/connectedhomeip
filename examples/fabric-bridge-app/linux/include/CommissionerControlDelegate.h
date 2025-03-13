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
#pragma once

#include <app/clusters/commissioner-control-server/commissioner-control-server.h>
#include <app/util/af-types.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissionerControl {

inline constexpr EndpointId kAggregatorEndpointId = 1;

class CommissionerControlDelegate : public Delegate
{
public:
    CommissionerControlDelegate() : mCommissionerControlServer(this, kAggregatorEndpointId) {}

    CHIP_ERROR HandleCommissioningApprovalRequest(const CommissioningApprovalRequest & request) override;
    // TODO(#35627) clientNodeId should move towards ScopedNodeId.
    CHIP_ERROR ValidateCommissionNodeCommand(NodeId clientNodeId, uint64_t requestId) override;
    CHIP_ERROR GetCommissioningWindowParams(CommissioningWindowParams & outParams) override;
    CHIP_ERROR HandleCommissionNode(const CommissioningWindowParams & params) override;

    ~CommissionerControlDelegate() = default;

    CommissionerControlServer & GetCommissionerControlServer() { return mCommissionerControlServer; }

private:
    enum class Step : uint8_t
    {
        // Ready to start reverse commissioning.
        kIdle,
        // Wait for the commission node command.
        kWaitCommissionNodeRequest,
        // Need to commission node.
        kStartCommissionNode,
    };

    static const char * GetStateString(Step step)
    {
        switch (step)
        {
        case Step::kIdle:
            return "kIdle";
        case Step::kWaitCommissionNodeRequest:
            return "kWaitCommissionNodeRequest";
        case Step::kStartCommissionNode:
            return "kStartCommissionNode";
        default:
            return "Unknown";
        }
    }

    void ResetDelegateState();

    static constexpr size_t kLabelBufferSize = 64;

    Step mNextStep       = Step::kIdle;
    uint64_t mRequestId  = 0;
    NodeId mClientNodeId = kUndefinedNodeId;
    VendorId mVendorId   = VendorId::Unspecified;
    uint16_t mProductId  = 0;
    char mLabelBuffer[kLabelBufferSize + 1];
    Optional<CharSpan> mLabel;

    // Parameters needed for non-basic commissioning.
    uint8_t mPBKDFSaltBuffer[Crypto::kSpake2p_Max_PBKDF_Salt_Length];
    ByteSpan mPBKDFSalt;
    Crypto::Spake2pVerifierSerialized mPAKEPasscodeVerifierBuffer;
    ByteSpan mPAKEPasscodeVerifier;

    CommissionerControlServer mCommissionerControlServer;
};

} // namespace CommissionerControl
} // namespace Clusters
} // namespace app
} // namespace chip

namespace bridge {

CHIP_ERROR CommissionerControlInit();
CHIP_ERROR CommissionerControlShutdown();

} // namespace bridge

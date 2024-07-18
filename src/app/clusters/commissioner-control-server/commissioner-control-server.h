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

#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissionerControl {

struct CommissioningApprovalRequest
{
    uint64_t requestId;
    VendorId vendorId;
    uint16_t productId;
    NodeId clientNodeId;
    FabricIndex fabricIndex;
    Optional<CharSpan> label;
};

struct CommissioningWindowParams
{
    uint32_t iterations;
    uint16_t commissioningTimeout;
    uint16_t discriminator;
    ByteSpan PAKEPasscodeVerifier;
    ByteSpan salt;
};

struct CommissionNodeInfo
{
    CommissioningWindowParams params;
    Optional<ByteSpan> ipAddress;
    Optional<uint16_t> port;
};

class Delegate
{
public:
    // Command Delegates
    virtual CHIP_ERROR HandleCommissioningApprovalRequest(const CommissioningApprovalRequest & request) = 0;
    virtual CHIP_ERROR ValidateCommissionNodeCommand(NodeId clientNodeId, uint64_t requestId,
                                                     CommissioningWindowParams & outParams)             = 0;
    virtual CHIP_ERROR ReverseCommissionNode(const CommissioningWindowParams & params, const Optional<ByteSpan> & ipAddress,
                                             const Optional<uint16_t> & port)                           = 0;
    virtual ~Delegate()                                                                                 = default;
};

class CommissionerControlServer
{
public:
    static CommissionerControlServer & Instance();

    CHIP_ERROR Init(Delegate & delegate);

    Delegate * GetDelegate() { return mDelegate; }

    Protocols::InteractionModel::Status
    GetSupportedDeviceCategoriesValue(EndpointId endpoint,
                                      BitMask<SupportedDeviceCategoryBitmap> * supportedDeviceCategories) const;

    Protocols::InteractionModel::Status
    SetSupportedDeviceCategoriesValue(EndpointId endpoint, const BitMask<SupportedDeviceCategoryBitmap> supportedDeviceCategories);

    /**
     * @brief
     *   Called after the server return SUCCESS to a correctly formatted RequestCommissioningApproval command.
     */
    CHIP_ERROR GenerateCommissioningRequestResultEvent(const Events::CommissioningRequestResult::Type & result);

private:
    CommissionerControlServer()  = default;
    ~CommissionerControlServer() = default;

    static CommissionerControlServer mInstance;

    Delegate * mDelegate = nullptr;
};

} // namespace CommissionerControl
} // namespace Clusters
} // namespace app
} // namespace chip

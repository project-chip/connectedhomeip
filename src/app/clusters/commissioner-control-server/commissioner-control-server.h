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

// Spec indicates that IP Address is either 4 or 16 bytes.
static constexpr size_t kIpAddressBufferSize = 16;

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

class ProtectedIPAddress
{
public:
    const Optional<ByteSpan> GetIPAddress() { return ipAddress; }

    CHIP_ERROR SetIPAddress(const Optional<ByteSpan> & address)
    {
        if (!address.HasValue())
        {
            ipAddress.ClearValue();
            return CHIP_NO_ERROR;
        }

        const ByteSpan & addressSpan = address.Value();
        size_t addressLength         = addressSpan.size();
        if (addressLength != 4 && addressLength != 16)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        memcpy(ipAddressBuffer, addressSpan.data(), addressLength);
        ipAddress.SetValue(ByteSpan(ipAddressBuffer, addressLength));
        return CHIP_NO_ERROR;
    }

private:
    Optional<ByteSpan> ipAddress;
    uint8_t ipAddressBuffer[kIpAddressBufferSize];
};

struct CommissionNodeInfo
{
    CommissioningWindowParams params;
    ProtectedIPAddress ipAddress;
    Optional<uint16_t> port;
};

class Delegate
{
public:
    /**
     * @brief Handle a commissioning approval request.
     *
     * This command is sent by a client to request approval for a future CommissionNode call.
     * The server SHALL always return SUCCESS to a correctly formatted RequestCommissioningApproval
     * command, and then send a CommissioningRequestResult event once the result is ready.
     *
     * @param request The commissioning approval request to handle.
     * @return CHIP_ERROR indicating the success or failure of the operation.
     */
    virtual CHIP_ERROR HandleCommissioningApprovalRequest(const CommissioningApprovalRequest & request) = 0;

    /**
     * @brief Validate a commission node command.
     *
     * This command is sent by a client to request that the server begins commissioning a previously
     * approved request.
     *
     * The server SHALL return FAILURE if the CommissionNode command is not sent from the same
     * NodeId as the RequestCommissioningApproval or if the provided RequestId to CommissionNode
     * does not match the value provided to RequestCommissioningApproval.
     *
     * The validation SHALL fail if the client Node ID is kUndefinedNodeId, such as getting the NodeID from
     * a group or PASE session.
     *
     * @param clientNodeId The NodeId of the client.
     * @param requestId The request ID to validate.
     * @return CHIP_ERROR indicating the success or failure of the operation.
     */
    virtual CHIP_ERROR ValidateCommissionNodeCommand(NodeId clientNodeId, uint64_t requestId) = 0;

    /**
     * @brief Get the parameters for the commissioning window.
     *
     * This method is called to retrieve the parameters needed for the commissioning window.
     *
     * @param[out] outParams The parameters for the commissioning window.
     * @return CHIP_ERROR indicating the success or failure of the operation.
     */
    virtual CHIP_ERROR GetCommissioningWindowParams(CommissioningWindowParams & outParams) = 0;

    /**
     * @brief Reverse the commission node process.
     *
     * When received within the timeout specified by CommissionNode, the client SHALL open a
     * commissioning window on the node which the client called RequestCommissioningApproval to
     * have commissioned.
     *
     * @param params The parameters for the commissioning window.
     * @param ipAddress Optional IP address for the commissioning window.
     * @param port Optional port for the commissioning window.
     * @return CHIP_ERROR indicating the success or failure of the operation.
     */
    virtual CHIP_ERROR ReverseCommissionNode(const CommissioningWindowParams & params, const Optional<ByteSpan> & ipAddress,
                                             const Optional<uint16_t> & port) = 0;

    virtual ~Delegate() = default;
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

    static CommissionerControlServer sInstance;

    Delegate * mDelegate = nullptr;
};

} // namespace CommissionerControl
} // namespace Clusters
} // namespace app
} // namespace chip

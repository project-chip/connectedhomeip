/*
 *    Copyright (c) 2024-2026 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/temperature-control-server/supported-temperature-levels-manager.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/CommissionerControl/Attributes.h>
#include <clusters/CommissionerControl/Metadata.h>

namespace chip::app::Clusters {

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
     * @brief Handle a commission node request.
     *
     * Commission a node specified by the previously approved request.
     *
     * @param params The parameters for the commissioning window.
     * @return CHIP_ERROR indicating the success or failure of the operation.
     */
    virtual CHIP_ERROR HandleCommissionNode(const CommissioningWindowParams & params) = 0;

    virtual ~Delegate() = default;
};

} // namespace CommissionerControl

class CommissionerControlCluster : public DefaultServerCluster
{
public:
    CommissionerControlCluster(EndpointId endpointId, CommissionerControl::Delegate * delegate = nullptr);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    CHIP_ERROR
    SetSupportedDeviceCategories(const BitMask<CommissionerControl::SupportedDeviceCategoryBitmap> supportedDeviceCategories);
    BitMask<CommissionerControl::SupportedDeviceCategoryBitmap> GetSupportedDeviceCategories() const
    {
        return mSupportedDeviceCategories;
    }

    // Should be called after the server return SUCCESS to a correctly formatted RequestCommissioningApproval command.
    CHIP_ERROR
    GenerateCommissioningRequestResultEvent(const CommissionerControl::Events::CommissioningRequestResult::Type & result);

    CommissionerControl::Delegate * GetDelegate() { return mDelegate; }
    void SetDelegate(CommissionerControl::Delegate * delegate) { mDelegate = delegate; }

protected:
    CommissionerControl::Delegate * mDelegate{};
    BitMask<CommissionerControl::SupportedDeviceCategoryBitmap> mSupportedDeviceCategories;

private:
    std::optional<DataModel::ActionReturnStatus> HandleRequestCommissioningApproval(
        CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
        const CommissionerControl::Commands::RequestCommissioningApproval::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus>
    HandleCommissionNode(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                         const CommissionerControl::Commands::CommissionNode::DecodableType & commandData);
};

} // namespace chip::app::Clusters

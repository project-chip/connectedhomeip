/*
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

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/MessageDef/CommandDataIB.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/OperationTypes.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLVReader.h>
#include <lib/core/TLVWriter.h>
#include <lib/support/CodeUtils.h>
#include <messaging/ExchangeContext.h>

#include <memory>
#include <vector>

namespace chip {
namespace app {
namespace Testing {

// Mock CommandHandler for testing command invocations
class MockCommandHandler : public CommandHandler
{
public:
    struct ResponseRecord
    {
        ConcreteCommandPath path;
        CommandId commandId;
        chip::System::PacketBufferHandle encodedData;
    };

    struct StatusRecord
    {
        ConcreteCommandPath path;
        Protocols::InteractionModel::ClusterStatusCode status;
        const char * context;
    };

    ~MockCommandHandler() override = default;

    CHIP_ERROR FallibleAddStatus(const ConcreteCommandPath & aRequestCommandPath,
                                 const Protocols::InteractionModel::ClusterStatusCode & aStatus,
                                 const char * context = nullptr) override;

    void AddStatus(const ConcreteCommandPath & aRequestCommandPath, const Protocols::InteractionModel::ClusterStatusCode & aStatus,
                   const char * context = nullptr) override;

    FabricIndex GetAccessingFabricIndex() const override { return mFabricIndex; }

    CHIP_ERROR AddResponseData(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                               const DataModel::EncodableToTLV & aEncodable) override;

    void AddResponse(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                     const DataModel::EncodableToTLV & aEncodable) override;

    bool IsTimedInvoke() const override { return false; }
    void FlushAcksRightAwayOnSlowCommand() override {}
    Access::SubjectDescriptor GetSubjectDescriptor() const override { return Access::SubjectDescriptor{}; }
    Messaging::ExchangeContext * GetExchangeContext() const override { return nullptr; }

    // Helper methods to extract response data
    bool HasResponse() const { return !mResponses.empty(); }
    bool HasResponses() const { return !mResponses.empty(); }
    size_t GetResponseCount() const { return mResponses.size(); }

    // Methods for working with single response (first in array)
    CommandId GetResponseCommandId() const { return mResponses.empty() ? 0 : mResponses[0].commandId; }
    const ResponseRecord & GetResponse() const { return mResponses[0]; }

    // Methods for working with all responses
    const std::vector<ResponseRecord> & GetResponses() const { return mResponses; }
    const ResponseRecord & GetResponse(size_t index) const { return mResponses[index]; }
    void ClearResponses() { mResponses.clear(); }

    // Helper methods to access stored statuses
    bool HasStatus() const { return !mStatuses.empty(); }
    const std::vector<StatusRecord> & GetStatuses() const { return mStatuses; }
    const StatusRecord & GetLastStatus() const { return mStatuses.back(); }
    void ClearStatuses() { mStatuses.clear(); }

    // Get a TLV reader positioned at the response data fields (first response)
    CHIP_ERROR GetResponseReader(TLV::TLVReader & reader) const;

    // Get a TLV reader positioned at the response data fields (specific response)
    CHIP_ERROR GetResponseReader(TLV::TLVReader & reader, size_t index) const;

    // Decode response into a specific DecodableType (first response)
    template <typename ResponseType>
    CHIP_ERROR DecodeResponse(ResponseType & response) const
    {
        TLV::TLVReader reader;
        ReturnErrorOnFailure(GetResponseReader(reader));
        return response.Decode(reader);
    }

    // Decode specific response into a specific DecodableType
    template <typename ResponseType>
    CHIP_ERROR DecodeResponse(ResponseType & response, size_t index) const
    {
        TLV::TLVReader reader;
        ReturnErrorOnFailure(GetResponseReader(reader, index));
        return response.Decode(reader);
    }

    // Configuration methods
    void SetFabricIndex(FabricIndex index) { mFabricIndex = index; }

private:
    std::vector<ResponseRecord> mResponses;
    std::vector<StatusRecord> mStatuses;
    FabricIndex mFabricIndex = 1; // Default to 1 to maintain backward compatibility
};

// Helper class for invoking commands in tests.
// Manages TLV encoding/decoding and command handler setup.
class InvokeOperation
{
public:
    InvokeOperation(EndpointId endpoint, ClusterId cluster, CommandId command) :
        mCommandPath(endpoint, cluster, command), mHandler(std::make_unique<MockCommandHandler>())
    {
        mRequest.path = mCommandPath;
    }

    InvokeOperation(const ConcreteCommandPath & path) : mCommandPath(path), mHandler(std::make_unique<MockCommandHandler>())
    {
        mRequest.path = mCommandPath;
    }

    // New constructor: allows injection of a custom MockCommandHandler
    InvokeOperation(const ConcreteCommandPath & path, std::unique_ptr<MockCommandHandler> handler) :
        mCommandPath(path), mHandler(std::move(handler))
    {
        mRequest.path = mCommandPath;
    }

    InvokeOperation(EndpointId endpoint, ClusterId cluster, CommandId command, std::unique_ptr<MockCommandHandler> handler) :
        mCommandPath(endpoint, cluster, command), mHandler(std::move(handler))
    {
        mRequest.path = mCommandPath;
    }
    // Invoke a command using a pre-defined request structure
    template <typename ClusterType, typename RequestType>
    std::optional<DataModel::ActionReturnStatus> Invoke(ClusterType & cluster, const RequestType & request)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        TLV::TLVWriter writer;
        writer.Init(mTlvBuffer);

        // Use the request's built-in Encode method
        SuccessOrExit(err = request.Encode(writer, TLV::AnonymousTag()));
        SuccessOrExit(err = writer.Finalize());

        // Setup reader (declare before any SuccessOrExit calls)
        TLV::TLVReader reader;
        reader.Init(mTlvBuffer, writer.GetLengthWritten());
        SuccessOrExit(err = reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

        // Invoke the command
        return cluster.InvokeCommand(mRequest, reader, mHandler.get());

    exit:
        return std::make_optional(DataModel::ActionReturnStatus(err));
    }

    // Get the command handler to inspect responses
    MockCommandHandler & GetHandler() { return *mHandler; }
    const MockCommandHandler & GetHandler() const { return *mHandler; }

    // Convenience method to decode the response
    template <typename ResponseType>
    CHIP_ERROR DecodeResponse(ResponseType & response)
    {
        return mHandler->DecodeResponse(response);
    }

    // Get the invoke request (path and metadata)
    const DataModel::InvokeRequest & GetRequest() const { return mRequest; }

private:
    // Buffer size for TLV encoding/decoding of command payloads.
    // 256 bytes was chosen as a conservative upper bound for typical command payloads in tests.
    // All command payloads used in tests must fit within this buffer; tests with larger payloads will fail.
    // If protocol or test requirements change, this value may need to be increased.
    static constexpr size_t kTlvBufferSize = 256;

    ConcreteCommandPath mCommandPath;
    DataModel::InvokeRequest mRequest;
    std::unique_ptr<MockCommandHandler> mHandler;
    uint8_t mTlvBuffer[kTlvBufferSize];
};

} // namespace Testing
} // namespace app
} // namespace chip

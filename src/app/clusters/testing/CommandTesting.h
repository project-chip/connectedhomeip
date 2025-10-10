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
#include <messaging/ExchangeContext.h>

#include <memory>

namespace chip {
namespace app {
namespace Testing {

// Mock CommandHandler for testing command invocations
class MockCommandHandler : public CommandHandler
{
public:
    ~MockCommandHandler() override = default;

    CHIP_ERROR FallibleAddStatus(const ConcreteCommandPath & aRequestCommandPath,
                                 const Protocols::InteractionModel::ClusterStatusCode & aStatus,
                                 const char * context = nullptr) override;

    void AddStatus(const ConcreteCommandPath & aRequestCommandPath, const Protocols::InteractionModel::ClusterStatusCode & aStatus,
                   const char * context = nullptr) override;

    FabricIndex GetAccessingFabricIndex() const override { return 1; }

    CHIP_ERROR AddResponseData(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                               const DataModel::EncodableToTLV & aEncodable) override;

    void AddResponse(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                     const DataModel::EncodableToTLV & aEncodable) override;

    bool IsTimedInvoke() const override { return false; }
    void FlushAcksRightAwayOnSlowCommand() override {}
    Access::SubjectDescriptor GetSubjectDescriptor() const override { return Access::SubjectDescriptor{}; }
    Messaging::ExchangeContext * GetExchangeContext() const override { return nullptr; }

    // Helper methods to extract response data
    bool HasResponse() const { return !mEncodedData.IsNull(); }
    CommandId GetResponseCommandId() const { return mResponseCommandId; }

    // Get a TLV reader positioned at the response data fields
    CHIP_ERROR GetResponseReader(TLV::TLVReader & reader);

    // Decode response into a specific DecodableType
    template <typename ResponseType>
    CHIP_ERROR DecodeResponse(ResponseType & response)
    {
        TLV::TLVReader reader;
        ReturnErrorOnFailure(GetResponseReader(reader));
        return response.Decode(reader);
    }

private:
    CommandId mResponseCommandId = 0;
    chip::System::PacketBufferHandle mEncodedData;
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

    // Invoke a command with no fields (empty TLV structure)
    // Use for commands that take no parameters
    template <typename ClusterType>
    std::optional<DataModel::ActionReturnStatus> InvokeEmpty(ClusterType & cluster)
    {
        // Create empty TLV structure
        TLV::TLVWriter writer;
        writer.Init(mTlvBuffer);
        TLV::TLVType outerContainer;

        CHIP_ERROR err = writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainer);
        if (err != CHIP_NO_ERROR)
        {
            return std::make_optional(DataModel::ActionReturnStatus(err));
        }

        err = writer.EndContainer(outerContainer);
        if (err != CHIP_NO_ERROR)
        {
            return std::make_optional(DataModel::ActionReturnStatus(err));
        }

        err = writer.Finalize();
        if (err != CHIP_NO_ERROR)
        {
            return std::make_optional(DataModel::ActionReturnStatus(err));
        }

        // Setup reader
        TLV::TLVReader reader;
        reader.Init(mTlvBuffer, writer.GetLengthWritten());

        err = reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag());
        if (err != CHIP_NO_ERROR)
        {
            return std::make_optional(DataModel::ActionReturnStatus(err));
        }

        // Invoke the command
        return cluster.InvokeCommand(mRequest, reader, mHandler.get());
    }

    // Invoke a command with parameters using an encodable request type
    // Use for commands that take parameters
    template <typename ClusterType, typename RequestType>
    std::optional<DataModel::ActionReturnStatus> Invoke(ClusterType & cluster, const RequestType & request)
    {
        // Encode request into TLV
        TLV::TLVWriter writer;
        writer.Init(mTlvBuffer);
        TLV::TLVType outerContainer;

        CHIP_ERROR err = writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainer);
        if (err != CHIP_NO_ERROR)
        {
            return std::make_optional(DataModel::ActionReturnStatus(err));
        }

        // Encode the request fields into the structure
        err = request.Encode(writer, TLV::AnonymousTag());
        if (err != CHIP_NO_ERROR)
        {
            return std::make_optional(DataModel::ActionReturnStatus(err));
        }

        err = writer.EndContainer(outerContainer);
        if (err != CHIP_NO_ERROR)
        {
            return std::make_optional(DataModel::ActionReturnStatus(err));
        }

        err = writer.Finalize();
        if (err != CHIP_NO_ERROR)
        {
            return std::make_optional(DataModel::ActionReturnStatus(err));
        }

        // Setup reader
        TLV::TLVReader reader;
        reader.Init(mTlvBuffer, writer.GetLengthWritten());

        err = reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag());
        if (err != CHIP_NO_ERROR)
        {
            return std::make_optional(DataModel::ActionReturnStatus(err));
        }

        // Invoke the command
        return cluster.InvokeCommand(mRequest, reader, mHandler.get());
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
    static constexpr size_t kTlvBufferSize = 256;

    ConcreteCommandPath mCommandPath;
    DataModel::InvokeRequest mRequest;
    std::unique_ptr<MockCommandHandler> mHandler;
    uint8_t mTlvBuffer[kTlvBufferSize];
};

} // namespace Testing
} // namespace app
} // namespace chip

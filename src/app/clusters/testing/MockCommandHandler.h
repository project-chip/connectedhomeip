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
#include <app/data-model/NullObject.h>
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

constexpr FabricIndex kTestFabricIndex = static_cast<FabricIndex>(151);

// Mock class that simulates CommandHandler behavior for unit testing, allowing capture and
// verification of responses and statuses without real network interactions.
class MockCommandHandler : public CommandHandler
{
public:
    struct ResponseRecord
    {
        CommandId commandId;
        System::PacketBufferHandle encodedData;
        ConcreteCommandPath path;
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

    // Encodes and stores response data, returning error if encoding fails (fallible version for robust test handling).
    CHIP_ERROR AddResponseData(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                               const DataModel::EncodableToTLV & aEncodable) override;

    // Encodes and stores response data, without error return (non-fallible version that assumes successful encoding in tests).
    void AddResponse(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                     const DataModel::EncodableToTLV & aEncodable) override;

    bool IsTimedInvoke() const override { return false; }
    void FlushAcksRightAwayOnSlowCommand() override {}
    Access::SubjectDescriptor GetSubjectDescriptor() const override { return Access::SubjectDescriptor{}; }
    Messaging::ExchangeContext * GetExchangeContext() const override { return nullptr; }

    // Helper methods to extract response data
    bool HasResponse() const { return !mResponses.empty(); }
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
    FabricIndex mFabricIndex = kTestFabricIndex; // Default to a clearly test-only fabric index.
};

} // namespace Testing
} // namespace app
} // namespace chip

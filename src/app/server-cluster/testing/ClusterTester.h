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

#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteClusterPath.h>
#include <app/ConcreteCommandPath.h>
#include <app/ConcreteEventPath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/data-model/List.h>
#include <app/data-model/NullObject.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <app/server-cluster/testing/FabricTestFixture.h>
#include <app/server-cluster/testing/MockCommandHandler.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/shared/Attributes.h>
#include <credentials/FabricTable.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLVReader.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/Span.h>
#include <protocols/interaction_model/StatusCode.h>

#include <algorithm>
#include <memory>
#include <optional>
#include <type_traits>
#include <vector>

namespace chip {
namespace Testing {

// Helper class for testing clusters.
//
// This class ensures that data read by attribute is referencing valid memory for all
// read requests until the ClusterTester object goes out of scope. (for the case where the underlying read references a list or
// string that points to TLV data).
//
// Read/Write of all attribute types should work, but make sure to use ::Type for encoding
// and ::DecodableType for decoding structure types.
//
// Example of usage:
//
// ExampleCluster cluster(someEndpointId);
//
// // Possibly steps to setup the cluster
//
// ClusterTester tester(cluster);
// app::Clusters::ExampleCluster::Attributes::FeatureMap::TypeInfo::DecodableType features;
// ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, features), CHIP_NO_ERROR);
//
// app::Clusters::ExampleCluster::Attributes::ExampleListAttribute::TypeInfo::DecodableType list;
// ASSERT_EQ(tester.ReadAttribute(LabelList::Id, list), CHIP_NO_ERROR);
// auto it = list.begin();
// while (it.Next())
// {
//     ASSERT_GT(it.GetValue().label.size(), 0u);
// }
//
class ClusterTester
{
public:
    ClusterTester(app::ServerClusterInterface & cluster) : mCluster(cluster), mFabricTestFixture(nullptr) {}

    // Constructor with FabricHelper
    ClusterTester(app::ServerClusterInterface & cluster, FabricTestFixture * fabricHelper) :
        mCluster(cluster), mFabricTestFixture(fabricHelper)
    {}

    TestServerClusterContext & GetTestContext() { return mTestServerClusterContext; }
    app::ServerClusterContext & GetServerClusterContext() { return mTestServerClusterContext.Get(); }

    // Read attribute into `out` parameter.
    // The `out` parameter must be of the correct type for the attribute being read.
    // Use `app::Clusters::<ClusterName>::Attributes::<AttributeName>::TypeInfo::DecodableType` for the `out` parameter to be spec
    // compliant (see the comment of the class for usage example).
    // Will construct the attribute path using the first path returned by `GetPaths()` on the cluster.
    // @returns `CHIP_ERROR_INCORRECT_STATE` if `GetPaths()` doesn't return a list with one path.
    // @returns `CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute)` if the attribute is not present in AttributeList.
    template <typename T>
    app::DataModel::ActionReturnStatus ReadAttribute(AttributeId attr_id, T & out)
    {
        VerifyOrReturnError(VerifyClusterPathsValid(), CHIP_ERROR_INCORRECT_STATE);

        // Verify that the attribute is present in AttributeList before attempting to read it.
        // This ensures tests match real-world behavior where the Interaction Model checks AttributeList first.
        VerifyOrReturnError(IsAttributeInAttributeList(attr_id), Protocols::InteractionModel::Status::UnsupportedAttribute);

        auto path = mCluster.GetPaths()[0];

        // Store the read operation in a vector<std::unique_ptr<...>> to ensure its lifetime
        // using std::unique_ptr because ReadOperation is non-copyable and non-movable
        // vector reallocation is not an issue since we store unique_ptrs
        std::unique_ptr<chip::Testing::ReadOperation> readOperation =
            std::make_unique<chip::Testing::ReadOperation>(path.mEndpointId, path.mClusterId, attr_id);

        mReadOperations.push_back(std::move(readOperation));
        chip::Testing::ReadOperation & readOperationRef = *mReadOperations.back().get();

        Access::SubjectDescriptor subjectDescriptor{ .fabricIndex = mHandler.GetAccessingFabricIndex() };
        readOperationRef.SetSubjectDescriptor(subjectDescriptor);

        std::unique_ptr<app::AttributeValueEncoder> encoder = readOperationRef.StartEncoding();
        app::DataModel::ActionReturnStatus status           = mCluster.ReadAttribute(readOperationRef.GetRequest(), *encoder);
        VerifyOrReturnError(status.IsSuccess(), status);
        ReturnErrorOnFailure(readOperationRef.FinishEncoding());

        std::vector<chip::Testing::DecodedAttributeData> attributeData;
        ReturnErrorOnFailure(readOperationRef.GetEncodedIBs().Decode(attributeData));
        VerifyOrReturnError(attributeData.size() == 1u, CHIP_ERROR_INCORRECT_STATE);

        return app::DataModel::Decode(attributeData[0].dataReader, out);
    }

    // Write attribute from `value` parameter.
    // The `value` parameter must be of the correct type for the attribute being written.
    // Use `app::Clusters::<ClusterName>::Attributes::<AttributeName>::TypeInfo::Type` for the `value` parameter to be spec
    // compliant (see the comment of the class for usage example).
    // Will construct the attribute path using the first path returned by `GetPaths()` on the cluster.
    // @returns `CHIP_ERROR_INCORRECT_STATE` if `GetPaths()` doesn't return a list with one path.
    // @returns `CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute)` if the attribute is not present in AttributeList.
    template <typename T>
    app::DataModel::ActionReturnStatus WriteAttribute(AttributeId attr, const T & value)
    {
        const auto & paths = mCluster.GetPaths();

        VerifyOrReturnError(paths.size() == 1u, CHIP_ERROR_INCORRECT_STATE);

        // Verify that the attribute is present in AttributeList before attempting to write it.
        // This ensures tests match real-world behavior where the Interaction Model checks AttributeList first.
        VerifyOrReturnError(IsAttributeInAttributeList(attr), Protocols::InteractionModel::Status::UnsupportedAttribute);

        app::ConcreteAttributePath path(paths[0].mEndpointId, paths[0].mClusterId, attr);
        chip::Testing::WriteOperation writeOp(path);

        // Create a stable object on the stack
        Access::SubjectDescriptor subjectDescriptor{ .fabricIndex = mHandler.GetAccessingFabricIndex() };
        writeOp.SetSubjectDescriptor(subjectDescriptor);

        uint8_t buffer[1024];
        TLV::TLVWriter writer;
        writer.Init(buffer);

        // - DataModel::Encode(integral, enum, etc.) for simple types.
        // - DataModel::Encode(List<X>) for lists (which iterates and calls Encode on elements).
        // - DataModel::Encode(Struct) for non-fabric-scoped structs.
        // - Note: For attribute writes, DataModel::EncodeForWrite is usually preferred for fabric-scoped types,
        //         but the generic DataModel::Encode often works as a top-level function.
        //         If you use EncodeForWrite, you ensure fabric-scoped list items are handled correctly:

        if constexpr (app::DataModel::IsFabricScoped<T>::value)
        {
            ReturnErrorOnFailure(chip::app::DataModel::EncodeForWrite(writer, TLV::AnonymousTag(), value));
        }
        else
        {
            ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, TLV::AnonymousTag(), value));
        }

        TLV::TLVReader reader;
        reader.Init(buffer, writer.GetLengthWritten());
        ReturnErrorOnFailure(reader.Next());

        app::AttributeValueDecoder decoder(reader, *writeOp.GetRequest().subjectDescriptor);

        return mCluster.WriteAttribute(writeOp.GetRequest(), decoder);
    }

    // Result structure for Invoke operations, containing both status and decoded response.
    template <typename ResponseType>
    struct InvokeResult
    {
        std::optional<app::DataModel::ActionReturnStatus> status;
        std::optional<ResponseType> response;

        // Returns true if the command was successful and response is available
        bool IsSuccess() const
        {
            if constexpr (std::is_same_v<ResponseType, app::DataModel::NullObjectType>)
                return status.has_value() && status->IsSuccess();
            else
                return status.has_value() && status->IsSuccess() && response.has_value();
        }
    };

    // Invoke a command and return the decoded result.
    // The `request` parameter must be of the correct type for the command being invoked.
    // Use `app::Clusters::<ClusterName>::Commands::<CommandName>::Type` for the `request` parameter to be spec compliant
    // Will construct the command path using the first path returned by `GetPaths()` on the cluster.
    // @returns `CHIP_ERROR_INCORRECT_STATE` if `GetPaths()` doesn't return a list with one path.
    template <typename RequestType, typename ResponseType = typename RequestType::ResponseType>
    [[nodiscard]] InvokeResult<ResponseType> Invoke(chip::CommandId commandId, const RequestType & request)
    {
        InvokeResult<ResponseType> result;

        const auto & paths = mCluster.GetPaths();
        VerifyOrReturnValue(paths.size() == 1u, result);

        mHandler.ClearResponses();
        mHandler.ClearStatuses();

        // Verify that the command is present in AcceptedCommands before attempting to invoke it.
        // This ensures tests match real-world behavior where the Interaction Model checks AcceptedCommands first.
        if (!IsCommandAnAcceptedCommand(commandId))
        {
            result.status = Protocols::InteractionModel::Status::UnsupportedCommand;
            return result;
        }

        const Access::SubjectDescriptor subjectDescriptor{ .fabricIndex = mHandler.GetAccessingFabricIndex() };
        const app::DataModel::InvokeRequest invokeRequest = [&]() {
            app::DataModel::InvokeRequest req;
            req.path              = { paths[0].mEndpointId, paths[0].mClusterId, commandId };
            req.subjectDescriptor = &subjectDescriptor;
            return req;
        }();

        TLV::TLVWriter writer;
        writer.Init(mTlvBuffer);

        TLV::TLVReader reader;

        VerifyOrReturnValue(request.Encode(writer, TLV::AnonymousTag()) == CHIP_NO_ERROR, result);
        VerifyOrReturnValue(writer.Finalize() == CHIP_NO_ERROR, result);

        reader.Init(mTlvBuffer, writer.GetLengthWritten());
        VerifyOrReturnValue(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()) == CHIP_NO_ERROR, result);

        result.status = mCluster.InvokeCommand(invokeRequest, reader, &mHandler);

        // If InvokeCommand returned nullopt, it means the command implementation handled the response.
        // We need to check the mock handler for a data response or a status response.
        if (!result.status.has_value())
        {
            if (mHandler.HasResponse())
            {
                // A data response was added, so the command is successful.
                result.status = app::DataModel::ActionReturnStatus(CHIP_NO_ERROR);
            }
            else if (mHandler.HasStatus())
            {
                // A status response was added. Use the last one.
                result.status = app::DataModel::ActionReturnStatus(mHandler.GetLastStatus().status);
            }
            else
            {
                // Neither response nor status was provided; this is unexpected.
                // This would happen either in error (as mentioned here) or if the command is supposed
                // to be handled asynchronously. ClusterTester does not support such asynchronous processing.
                result.status = app::DataModel::ActionReturnStatus(CHIP_ERROR_INCORRECT_STATE);
                ChipLogError(
                    Test, "InvokeCommand returned nullopt, but neither HasResponse nor HasStatus is true. Setting error status.");
            }
        }

        // If command was successful and there's a response, decode it (skip for NullObjectType)
        if constexpr (!std::is_same_v<ResponseType, app::DataModel::NullObjectType>)
        {
            if (result.status.has_value() && result.status->IsSuccess() && mHandler.HasResponse())
            {
                ResponseType decodedResponse;
                CHIP_ERROR decodeError = mHandler.DecodeResponse(decodedResponse);
                if (decodeError == CHIP_NO_ERROR)
                {
                    result.response = std::move(decodedResponse);
                }
                else
                {
                    // Decode failed; reflect error in status and log
                    result.status = app::DataModel::ActionReturnStatus(decodeError);
                    ChipLogError(Test, "DecodeResponse failed: %s", decodeError.AsString());
                }
            }
        }

        return result;
    }

    // convenience method: most requests have a `GetCommandId` (and GetClusterId() as well).
    template <typename RequestType, typename ResponseType = typename RequestType::ResponseType>
    [[nodiscard]] InvokeResult<ResponseType> Invoke(const RequestType & request)
    {
        return Invoke(RequestType::GetCommandId(), request);
    }

    // Returns the next generated event from the event generator in the test server cluster context
    std::optional<LogOnlyEvents::EventInformation> GetNextGeneratedEvent()
    {
        return mTestServerClusterContext.EventsGenerator().GetNextEvent();
    }

    std::vector<app::AttributePathParams> & GetDirtyList() { return mTestServerClusterContext.ChangeListener().DirtyList(); }

    void SetFabricIndex(FabricIndex fabricIndex) { mHandler.SetFabricIndex(fabricIndex); }

    FabricTestFixture * GetFabricHelper() { return mFabricTestFixture; }

private:
    bool VerifyClusterPathsValid()
    {
        auto paths = mCluster.GetPaths();
        if (paths.size() != 1)
        {
            ChipLogError(Test, "cluster.GetPaths() did not return exactly one path");
            return false;
        }
        return true;
    }

    bool IsAttributeInAttributeList(AttributeId attr_id)
    {
        // Attributes are listed by path, so this is only correct for single-path clusters.
        VerifyOrDie(mCluster.GetPaths().size() == 1);

        ReadOnlyBufferBuilder<app::DataModel::AttributeEntry> builder;
        if (CHIP_ERROR err = mCluster.Attributes(mCluster.GetPaths()[0], builder); err != CHIP_NO_ERROR)
        {
            ChipLogError(Test, "Failed to get attribute list: %" CHIP_ERROR_FORMAT, err.Format());
            return false;
        }

        ReadOnlyBuffer<app::DataModel::AttributeEntry> attributeEntries = builder.TakeBuffer();
        return std::any_of(attributeEntries.begin(), attributeEntries.end(),
                           [&](const app::DataModel::AttributeEntry & entry) { return entry.attributeId == attr_id; });
    }

    bool IsCommandAnAcceptedCommand(CommandId commandId)
    {
        // Commands are listed by path, so this is only correct for single-path clusters.
        VerifyOrDie(mCluster.GetPaths().size() == 1);

        ReadOnlyBufferBuilder<app::DataModel::AcceptedCommandEntry> builder;
        if (CHIP_ERROR err = mCluster.AcceptedCommands(mCluster.GetPaths()[0], builder); err != CHIP_NO_ERROR)
        {
            ChipLogError(Test, "Failed to get accepted commands: %" CHIP_ERROR_FORMAT, err.Format());
            return false;
        }

        ReadOnlyBuffer<app::DataModel::AcceptedCommandEntry> commandEntries = builder.TakeBuffer();
        return std::any_of(commandEntries.begin(), commandEntries.end(),
                           [&](const app::DataModel::AcceptedCommandEntry & entry) { return entry.commandId == commandId; });
    }

    TestServerClusterContext mTestServerClusterContext{};
    app::ServerClusterInterface & mCluster;

    // Buffer size for TLV encoding/decoding of command payloads.
    // 256 bytes was chosen as a conservative upper bound for typical command payloads in tests.
    // All command payloads used in tests must fit within this buffer; tests with larger payloads will fail.
    // If protocol or test requirements change, this value may need to be increased.
    // Increased to 1024 to support certificate management commands which include X.509 certificates (~400+ bytes)
    static constexpr size_t kTlvBufferSize = 1024;

    chip::Testing::MockCommandHandler mHandler;
    uint8_t mTlvBuffer[kTlvBufferSize];
    std::vector<std::unique_ptr<ReadOperation>> mReadOperations;

    FabricTestFixture * mFabricTestFixture;
};

} // namespace Testing
} // namespace chip

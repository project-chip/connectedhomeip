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
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/data-model/NullObject.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <lib/core/TLVReader.h>
#include <lib/support/ReadOnlyBuffer.h>

#include <memory>
#include <optional>
#include <type_traits>
#include <vector>

namespace chip {
namespace Test {

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
    ClusterTester(app::ServerClusterInterface & cluster) : mCluster(cluster) {}

    app::ServerClusterContext & GetServerClusterContext() { return mTestServerClusterContext.Get(); }

    // Read attribute into `out` parameter.
    // The `out` parameter must be of the correct type for the attribute being read.
    // Use `app::Clusters::<ClusterName>::Attributes::<AttributeName>::TypeInfo::DecodableType` for the `out` parameter to be spec
    // compliant (see the comment of the class for usage example).
    // Will construct the attribute path using the first path returned by `GetPaths()` on the cluster.
    // @returns `CHIP_ERROR_INCORRECT_STATE` if `GetPaths()` doesn't return a list with one path.
    template <typename T>
    app::DataModel::ActionReturnStatus ReadAttribute(AttributeId attr_id, T & out)
    {
        VerifyOrReturnError(VerifyClusterPathsValid(), CHIP_ERROR_INCORRECT_STATE);
        auto path = mCluster.GetPaths()[0];

        // Store the read operation in a vector<std::unique_ptr<...>> to ensure its lifetime
        // using std::unique_ptr because ReadOperation is non-copyable and non-movable
        // vector reallocation is not an issue since we store unique_ptrs
        std::unique_ptr<app::Testing::ReadOperation> readOperation =
            std::make_unique<app::Testing::ReadOperation>(path.mEndpointId, path.mClusterId, attr_id);

        mReadOperations.push_back(std::move(readOperation));
        app::Testing::ReadOperation & readOperationRef = *mReadOperations.back().get();

        std::unique_ptr<app::AttributeValueEncoder> encoder = readOperationRef.StartEncoding();
        app::DataModel::ActionReturnStatus status           = mCluster.ReadAttribute(readOperationRef.GetRequest(), *encoder);
        VerifyOrReturnError(status.IsSuccess(), status);
        ReturnErrorOnFailure(readOperationRef.FinishEncoding());

        std::vector<app::Testing::DecodedAttributeData> attributeData;
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
    // multi-fabric writes will not work
    // does not handle structs or ZAP-generated types with Encode / EncodeForWrite(), or DataModel::List.
    template <typename T>
    app::DataModel::ActionReturnStatus WriteAttribute(AttributeId attr_id, const T & value)
    {
        VerifyOrReturnError(VerifyClusterPathsValid(), CHIP_ERROR_INCORRECT_STATE);
        auto path = mCluster.GetPaths()[0];

        app::Testing::WriteOperation writeOperation(path.mEndpointId, path.mClusterId, attr_id);

        app::AttributeValueDecoder decoder = writeOperation.DecoderFor(value);
        return mCluster.WriteAttribute(writeOperation.GetRequest(), decoder);
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

    /*
     * CHIP Test Cluster Write Helpers
     *
     *  - Scalars/enums: written directly using DecoderFor().
     *  - Structs/ZAP structs: encoded into a TLV buffer via Encode() or EncodeForWrite().
     *  - Lists: encoded as TLV arrays. Each element is encoded individually:
     *      * Struct elements must provide EncodeForWrite().
     *      * Simple elements (integral, enum) are handled by generic templated encoding.
     *  - Supports multiple fabrics through WriteOperation subject descriptors.
     *  - Uses cluster.GetPaths() to automatically deduce the endpoint + cluster for convenience.
     */

    template <typename T, typename = void>
    struct HasEncodeForWrite : std::false_type
    {
    };

    template <typename T>
    struct HasEncodeForWrite<
        T,
        std::void_t<decltype(std::declval<T>().EncodeForWrite(std::declval<chip::TLV::TLVWriter &>(), chip::TLV::AnonymousTag()))>>
        : std::true_type
    {
    };

    template <typename T, typename = void>
    struct HasGenericEncode : std::false_type
    {
    };

    template <typename T>
    struct HasGenericEncode<
        T, std::void_t<decltype(std::declval<T>().Encode(std::declval<chip::TLV::TLVWriter &>(), chip::TLV::AnonymousTag()))>>
        : std::true_type
    {
    };

    // Main helper for a single value (scalar or struct)
    template <typename T>
    CHIP_ERROR WriteAttribute(app::ServerClusterInterface & cluster, AttributeId attr, const T & value,
                              FabricIndex fabricIndex = kUndefinedFabricIndex)
    {
        const auto & paths = cluster.GetPaths();
        VerifyOrReturnError(paths.size() == 1u, CHIP_ERROR_INCORRECT_STATE);

        app::ConcreteAttributePath path(paths[0].mEndpointId, paths[0].mClusterId, attr);

        app::Testing::WriteOperation writeOperation(path);
        if (fabricIndex != kUndefinedFabricIndex)
        {
            writeOperation.SetSubjectDescriptor({ fabricIndex });
        }

        if constexpr (std::is_integral_v<T> || std::is_enum_v<T>)
        {
            app::AttributeValueDecoder decoder = writeOperation.DecoderFor(value);
            return cluster.WriteAttribute(writeOperation.GetRequest(), decoder).GetUnderlyingError();
        }
        else
        {
            uint8_t buffer[1024];
            chip::TLV::TLVWriter writer;
            writer.Init(buffer);

            if constexpr (HasGenericEncode<T>::value)
            {
                ReturnErrorOnFailure(value.Encode(writer, chip::TLV::AnonymousTag()));
            }
            else if constexpr (HasEncodeForWrite<T>::value)
            {
                ReturnErrorOnFailure(value.EncodeForWrite(writer, chip::TLV::AnonymousTag()));
            }
            else
            {
                static_assert(!sizeof(T), "Type not supported by WriteAttribute");
                return CHIP_ERROR_NOT_IMPLEMENTED;
            }

            chip::TLV::TLVReader reader;
            reader.Init(buffer, writer.GetLengthWritten());
            ReturnErrorOnFailure(reader.Next());

            app::AttributeValueDecoder decoder(reader, *writeOperation.GetRequest().subjectDescriptor);
            return cluster.WriteAttribute(writeOperation.GetRequest(), decoder).GetUnderlyingError();
        }
    }

    // Overload for DataModel::List
    template <typename ElementT>
    CHIP_ERROR WriteAttribute(app::ServerClusterInterface & cluster, AttributeId attr,
                              const chip::app::DataModel::List<ElementT> & list, FabricIndex fabricIndex = kUndefinedFabricIndex)
    {
        const auto & paths = cluster.GetPaths();
        VerifyOrReturnError(paths.size() == 1u, CHIP_ERROR_INCORRECT_STATE);

        app::ConcreteAttributePath path(paths[0].mEndpointId, paths[0].mClusterId, attr);

        app::Testing::WriteOperation writeOperation(path);
        if (fabricIndex != kUndefinedFabricIndex)
        {
            writeOperation.SetSubjectDescriptor({ fabricIndex });
        }

        uint8_t buffer[1024];
        chip::TLV::TLVWriter writer;
        writer.Init(buffer);

        chip::TLV::TLVType outer;
        ReturnErrorOnFailure(writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Array, outer));

        for (const auto & item : list)
        {
            if constexpr (std::is_integral_v<ElementT> || std::is_enum_v<ElementT>)
            {
                ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, chip::TLV::AnonymousTag(), item));
            }
            else if constexpr (HasGenericEncode<ElementT>::value)
            {
                ReturnErrorOnFailure(item.Encode(writer, chip::TLV::AnonymousTag()));
            }
            else if constexpr (HasEncodeForWrite<ElementT>::value)
            {
                ReturnErrorOnFailure(item.EncodeForWrite(writer, chip::TLV::AnonymousTag()));
            }
            else
            {
                static_assert(!sizeof(ElementT), "Element type not supported in DataModel::List");
            }
        }

        ReturnErrorOnFailure(writer.EndContainer(outer));

        chip::TLV::TLVReader reader;
        reader.Init(buffer, writer.GetLengthWritten());
        ReturnErrorOnFailure(reader.Next());

        app::AttributeValueDecoder decoder(reader, *writeOperation.GetRequest().subjectDescriptor);
        return cluster.WriteAttribute(writeOperation.GetRequest(), decoder).GetUnderlyingError();
    }

    // Prepares an invoke request: sets the command path and clears handler state.
    void InvokeOperation(const app::ConcreteCommandPath & path)
    {
        mCommandPath = path;
        mHandler.ClearResponses();
        mHandler.ClearStatuses();
        mRequest.path = mCommandPath;
    }

    // Invoke a command using a predefined request structure
    template <typename RequestType>
    [[nodiscard]] std::optional<app::DataModel::ActionReturnStatus> Invoke(const RequestType & request)
    {
        TLV::TLVWriter writer;
        writer.Init(mTlvBuffer);

        TLV::TLVReader reader;

        ReturnErrorOnFailure(request.Encode(writer, TLV::AnonymousTag()));
        ReturnErrorOnFailure(writer.Finalize());

        reader.Init(mTlvBuffer, writer.GetLengthWritten());
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

        return mCluster.InvokeCommand(mRequest, reader, &mHandler);
    }

    // Simplified overload to invoke a command with just the command ID and data.
    // Builds the request automatically using the internal cluster's paths.
    // Ideal for quick tests without manual request construction.
    template <typename T>
    [[nodiscard]] std::optional<app::DataModel::ActionReturnStatus> Invoke(chip::CommandId commandId, const T & data,
                                                                           app::CommandHandler * handler)
    {
        InvokeResult<ResponseType> result;

        const auto & paths = mCluster.GetPaths();
        VerifyOrReturnError(paths.size() == 1u, CHIP_ERROR_INCORRECT_STATE);
        const app::DataModel::InvokeRequest request = { .path = { paths[0].mEndpointId, paths[0].mClusterId, commandId } };

        chip::TLV::TLVWriter tlvWriter;
        tlvWriter.Init(mTlvBuffer);
        ReturnErrorOnFailure(data.Encode(tlvWriter, chip::TLV::AnonymousTag()));

        chip::TLV::TLVReader tlvReader;
        tlvReader.Init(mTlvBuffer, tlvWriter.GetLengthWritten());
        ReturnErrorOnFailure(tlvReader.Next());

        // Use provided handler or internal one
        if (handler == nullptr)
        {
            handler = &mHandler;
        }

        return mCluster.InvokeCommand(request, tlvReader, handler);
    }

    // Invoke a command using a predefined request structure
    template <typename T>
    app::DataModel::ActionReturnStatus Invoke(chip::CommandId commandId, const T & data)
    {
        const auto & paths = mCluster.GetPaths();
        VerifyOrDie(paths.size() == 1u);
        const app::DataModel::InvokeRequest request = { .path = { paths[0].mEndpointId, paths[0].mClusterId, commandId } };

        // Clear any previous handler state to avoid stale responses
        mHandler.ClearResponses();
        mHandler.ClearStatuses();

        chip::TLV::TLVWriter tlvWriter;
        tlvWriter.Init(mTlvBuffer);
        CHIP_ERROR err = data.Encode(tlvWriter, chip::TLV::AnonymousTag());
        VerifyOrDie(err == CHIP_NO_ERROR);

        chip::TLV::TLVReader tlvReader;
        tlvReader.Init(mTlvBuffer, tlvWriter.GetLengthWritten());
        VerifyOrDie(tlvReader.Next() == CHIP_NO_ERROR);

        auto statusOpt = mCluster.InvokeCommand(request, tlvReader, &mHandler);
        if (statusOpt.has_value())
        {
            return statusOpt.value();
        }

        // A response was produced; consider this a success from a status perspective
        return Protocols::InteractionModel::Status::Success;
    }

    // Simplified overload to invoke a command with just the command ID and data.
    // Builds the request automatically using the internal cluster's paths.
    // Ideal for quick tests without manual request construction.
    template <typename ResponseType, typename T>
    ResponseType Invoke(chip::CommandId commandId, const T & data)
    {
        const auto & paths = mCluster.GetPaths();
        VerifyOrDie(paths.size() == 1u);
        const app::DataModel::InvokeRequest request = { .path = { paths[0].mEndpointId, paths[0].mClusterId, commandId } };

        // Clear any previous handler state to avoid stale responses
        mHandler.ClearResponses();
        mHandler.ClearStatuses();

        chip::TLV::TLVWriter tlvWriter;
        tlvWriter.Init(mTlvBuffer);
        CHIP_ERROR err = data.Encode(tlvWriter, chip::TLV::AnonymousTag());
        VerifyOrDie(err == CHIP_NO_ERROR);

        chip::TLV::TLVReader tlvReader;
        tlvReader.Init(mTlvBuffer, tlvWriter.GetLengthWritten());
        VerifyOrDie(tlvReader.Next() == CHIP_NO_ERROR);

        auto statusOpt = mCluster.InvokeCommand(request, tlvReader, &mHandler);
        // Expect a response to be produced (statusOpt must be empty)
        VerifyOrDie(!statusOpt.has_value());

        ResponseType response;
        err = mHandler.DecodeResponse(response);
        VerifyOrDie(err == CHIP_NO_ERROR);
        return response;
    }

    const app::DataModel::InvokeRequest & GetRequest() const { return mRequest; }

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

    TestServerClusterContext mTestServerClusterContext{};
    app::ServerClusterInterface & mCluster;
    std::unique_ptr<app::Testing::ReadOperation> mReadOperation;

    // Buffer size for TLV encoding/decoding of command payloads.
    // 256 bytes was chosen as a conservative upper bound for typical command payloads in tests.
    // All command payloads used in tests must fit within this buffer; tests with larger payloads will fail.
    // If protocol or test requirements change, this value may need to be increased.
    static constexpr size_t kTlvBufferSize = 256;

    app::ConcreteCommandPath mCommandPath;
    app::DataModel::InvokeRequest mRequest;
    app::Testing::MockCommandHandler mHandler;
    uint8_t mTlvBuffer[kTlvBufferSize];
};
} // namespace Test
} // namespace chip

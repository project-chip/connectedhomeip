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
#include <app/clusters/testing/MockCommandHandler.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/data-model/NullObject.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
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

// Detection traits for Encode() and EncodeForWrite()
template <typename...>
using void_t = void;

// Detection traits for Encode() and EncodeForWrite()
template <typename T, typename = void>
struct HasEncodeForWrite : std::false_type
{
};

template <typename T>
struct HasEncodeForWrite<
    T, void_t<decltype(std::declval<T>().EncodeForWrite(std::declval<chip::TLV::TLVWriter &>(), chip::TLV::AnonymousTag()))>>
    : std::true_type
{
};

template <typename T, typename = void>
struct HasGenericEncode : std::false_type
{
};

template <typename T>
struct HasGenericEncode<
    T, void_t<decltype(std::declval<T>().Encode(std::declval<chip::TLV::TLVWriter &>(), chip::TLV::AnonymousTag()))>>
    : std::true_type
{
};

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

    // multi-fabric writes will not work
    // does not handle structs or ZAP-generated types with Encode / EncodeForWrite(), or DataModel::List.
    // Write attribute from `value` parameter.
    // The `value` parameter must be of the correct type for the attribute being written.
    // Use `app::Clusters::<ClusterName>::Attributes::<AttributeName>::TypeInfo::Type` for the `value` parameter to be spec
    // compliant (see the comment of the class for usage example).
    // Will construct the attribute path using the first path returned by `GetPaths()` on the cluster.
    // @returns `CHIP_ERROR_INCORRECT_STATE` if `GetPaths()` doesn't return a list with one path.
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

    // Invoke a command and return the decoded result.
    // The `RequestType`, `ResponseType` type-parameters must be of the correct type for the command being invoked.
    // Use `app::Clusters::<ClusterName>::Commands::<CommandName>::Type` for the `RequestType` type-parameter to be spec compliant
    // Use `app::Clusters::<ClusterName>::Commands::<CommandName>::Type::ResponseType` for the `ResponseType` type-parameter to be
    // spec compliant Will construct the command path using the first path returned by `GetPaths()` on the cluster.
    // @returns `CHIP_ERROR_INCORRECT_STATE` if `GetPaths()` doesn't return a list with one path.
    template <typename ResponseType, typename RequestType>
    [[nodiscard]] InvokeResult<ResponseType> Invoke(chip::CommandId commandId, const RequestType & request)
    {
        InvokeResult<ResponseType> result;

        const auto & paths = mCluster.GetPaths();
        VerifyOrReturnValue(paths.size() == 1u, result);

        mHandler.ClearResponses();
        mHandler.ClearStatuses();

        const app::DataModel::InvokeRequest invokeRequest = { .path = { paths[0].mEndpointId, paths[0].mClusterId, commandId } };

        TLV::TLVWriter writer;
        writer.Init(mTlvBuffer);

        TLV::TLVReader reader;

        VerifyOrReturnValue(request.Encode(writer, TLV::AnonymousTag()) == CHIP_NO_ERROR, result);
        VerifyOrReturnValue(writer.Finalize() == CHIP_NO_ERROR, result);

        reader.Init(mTlvBuffer, writer.GetLengthWritten());
        VerifyOrReturnValue(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()) == CHIP_NO_ERROR, result);

        result.status = mCluster.InvokeCommand(invokeRequest, reader, &mHandler);

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

    // Write single value
    template <typename T>
    CHIP_ERROR WriteAttribute(AttributeId attr, const T & value, chip::FabricIndex fabricIndex)
    {
        const auto & paths = mCluster.GetPaths();
        VerifyOrReturnError(paths.size() == 1u, CHIP_ERROR_INCORRECT_STATE);

        app::ConcreteAttributePath path(paths[0].mEndpointId, paths[0].mClusterId, attr);
        app::Testing::WriteOperation writeOp(path);

        // Create a stable object on the stack beware of a dangling ptr fabric index will change
        chip::Access::SubjectDescriptor subjectDescriptor{ fabricIndex };
        writeOp.SetSubjectDescriptor(subjectDescriptor);

        uint8_t buffer[1024];
        chip::TLV::TLVWriter writer;
        writer.Init(buffer);

        ReturnErrorOnFailure(EncodeValueToTLV(writer, value));

        chip::TLV::TLVReader reader;
        reader.Init(buffer, writer.GetLengthWritten());
        ReturnErrorOnFailure(reader.Next());

        app::AttributeValueDecoder decoder(reader, *writeOp.GetRequest().subjectDescriptor);
        return mCluster.WriteAttribute(writeOp.GetRequest(), decoder).GetUnderlyingError();
    }
    // Write list of values
    template <typename ElementT>
    CHIP_ERROR WriteAttribute(AttributeId attr, const chip::app::DataModel::List<ElementT> & list, chip::FabricIndex fabricIndex)
    {
        const auto & paths = mCluster.GetPaths();
        VerifyOrReturnError(paths.size() == 1u, CHIP_ERROR_INCORRECT_STATE);

        app::ConcreteAttributePath path(paths[0].mEndpointId, paths[0].mClusterId, attr);
        app::Testing::WriteOperation writeOp(path);
        // Create a stable object on the stack beware of a dangling ptr fabric index will change
        chip::Access::SubjectDescriptor subjectDescriptor{ fabricIndex };
        writeOp.SetSubjectDescriptor(subjectDescriptor);
        uint8_t buffer[1024];
        chip::TLV::TLVWriter writer;
        writer.Init(buffer);

        chip::TLV::TLVType outer;
        ReturnErrorOnFailure(writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Array, outer));

        for (const auto & item : list)
        {
            ReturnErrorOnFailure(EncodeValueToTLV(writer, item));
        }

        ReturnErrorOnFailure(writer.EndContainer(outer));

        chip::TLV::TLVReader reader;
        reader.Init(buffer, writer.GetLengthWritten());
        ReturnErrorOnFailure(reader.Next());

        app::AttributeValueDecoder decoder(reader, *writeOp.GetRequest().subjectDescriptor);
        return mCluster.WriteAttribute(writeOp.GetRequest(), decoder).GetUnderlyingError();
    }

private:
    // Helper to encode any value to TLV
    template <typename T>
    static CHIP_ERROR EncodeValueToTLV(chip::TLV::TLVWriter & writer, const T & value)
    {
        if constexpr (std::is_integral_v<T> || std::is_enum_v<T>)
        {
            return chip::app::DataModel::Encode(writer, chip::TLV::AnonymousTag(), value);
        }
        else if constexpr (HasEncodeForWrite<T>::value) // <-- Path B
        {
            return value.EncodeForWrite(writer, chip::TLV::AnonymousTag());
        }
        else if constexpr (HasGenericEncode<T>::value) // <-- Path A
        {
            return value.Encode(writer, chip::TLV::AnonymousTag());
        }
        else
        {
            static_assert(!sizeof(T), "Type not supported for TLV encoding");
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }
    }

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

    // Buffer size for TLV encoding/decoding of command payloads.
    // 256 bytes was chosen as a conservative upper bound for typical command payloads in tests.
    // All command payloads used in tests must fit within this buffer; tests with larger payloads will fail.
    // If protocol or test requirements change, this value may need to be increased.
    static constexpr size_t kTlvBufferSize = 256;

    app::Testing::MockCommandHandler mHandler;
    uint8_t mTlvBuffer[kTlvBufferSize];
    std::vector<std::unique_ptr<app::Testing::ReadOperation>> mReadOperations;
};

} // namespace Test
} // namespace chip

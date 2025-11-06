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

#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteClusterPath.h>
#include <app/ConcreteEventPath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLVReader.h>
#include <lib/support/ReadOnlyBuffer.h>

#include <memory>
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
    template <typename T>
    app::DataModel::ActionReturnStatus WriteAttribute(AttributeId attr_id, const T & value)
    {
        VerifyOrReturnError(VerifyClusterPathsValid(), CHIP_ERROR_INCORRECT_STATE);
        auto path = mCluster.GetPaths()[0];

        app::Testing::WriteOperation writeOperation(path.mEndpointId, path.mClusterId, attr_id);

        app::AttributeValueDecoder decoder = writeOperation.DecoderFor(value);
        return mCluster.WriteAttribute(writeOperation.GetRequest(), decoder);
    }

    // Invoke a command with `data` as arguments.
    // The `data` parameter must be of the correct type for the command being invoked.
    // Use `app::Clusters::<ClusterName>::Commands::<CommandName>::Type` for the `data` parameter to be spec compliant (see the
    // comment of the class for usage example).
    // Will construct the command path using the first path returned by `GetPaths()` on the cluster.
    // @returns `CHIP_ERROR_INCORRECT_STATE` if `GetPaths()` doesn't return a list with one path.
    template <typename T>
    std::optional<app::DataModel::ActionReturnStatus> InvokeCommand(CommandId commandId, const T & data,
                                                                    app::CommandHandler * handler)
    {
        VerifyOrReturnError(VerifyClusterPathsValid(), CHIP_ERROR_INCORRECT_STATE);
        auto path                                   = mCluster.GetPaths()[0];
        const app::DataModel::InvokeRequest request = { .path = { path.mEndpointId, path.mClusterId, commandId } };

        constexpr size_t kTlvBufferSize = 128; // Typically CommanderSender will use a TLV of size kMaxSecureSduLengthBytes. For
                                               // now, just use 128 for the unit test.
        uint8_t buffer[kTlvBufferSize];
        TLV::TLVWriter tlvWriter;
        tlvWriter.Init(buffer);
        ReturnErrorOnFailure(data.Encode(tlvWriter, TLV::AnonymousTag()));

        TLV::TLVReader tlvReader;
        tlvReader.Init(buffer, tlvWriter.GetLengthWritten());
        ReturnErrorOnFailure(tlvReader.Next());

        return mCluster.InvokeCommand(request, tlvReader, handler);
    }

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
    std::vector<std::unique_ptr<app::Testing::ReadOperation>> mReadOperations;
};

} // namespace Test
} // namespace chip

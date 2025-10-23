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
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <data-model-providers/codedriven/endpoint/EndpointInterface.h>
#include <data-model-providers/codedriven/endpoint/EndpointInterfaceRegistry.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLVReader.h>
#include <lib/support/ReadOnlyBuffer.h>

#include <memory>
#include <list>

namespace chip {
namespace Test {

// This will be used for testing commands
struct CommandResponse {
    app::ConcreteCommandPath path;
    CommandId responseId;
    template <typename T>
    CHIP_ERROR GetResponseData(T & dest);

private:
    uint8_t mResponseEncodeBuffer[128];
    ByteSpan mEncodedSpan;
};

// Helper class for testing clusters.
//
// This class ensures that data read by attribute is referencing valid memory for all
// read requests until the ClusterTester object goes out of scope. (for the case where the underlying read references a list or string that
// points to TLV data).
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
//
// Almost every function has 4 overloads:
// 1) specifying the cluster and a full request structure
// 2) specifying only the cluster and minimal parameters (e.g. attribute id)
// 3) specifying no cluster but a full request structure (uses default cluster)
// 4) specifying no cluster and minimal parameters (uses default cluster)
class ClusterTester {
public:
    ClusterTester() {}

    // Sets the default cluster to be used for Read/Write/Invoke operations that do not specify a cluster.
    ClusterTester(app::ServerClusterInterface & defaultCluster) :
        mDefaultCluster(&defaultCluster)
    {}

    // Sets the default cluster to be used for Read/Write/Invoke operations that do not specify a cluster.
    void SetDefaultCluster(app::ServerClusterInterface & cluster) { mDefaultCluster = &cluster; }

    app::ServerClusterContext & GetServerClusterContext() { return mTestServerClusterContext.Get(); }

    // Read attribute into `out` parameter.
    // The `out` parameter must be of the correct type for the attribute being read.
    // Use `app::Clusters::<ClusterName>::Attributes::<AttributeName>::TypeInfo::DecodableType` for the `out` parameter to be spec compliant (see the comment of the class for usage example).
    template <typename T>
    app::DataModel::ActionReturnStatus ReadAttribute(app::ServerClusterInterface & cluster, const app::DataModel::ReadAttributeRequest & path, T & out)
    {
        std::unique_ptr<app::Testing::ReadOperation> readOperation = std::make_unique<app::Testing::ReadOperation>(path.path);
        readOperation->SetReadFlags(path.readFlags).SetOperationFlags(path.operationFlags);
        if (path.subjectDescriptor) readOperation->SetSubjectDescriptor(*path.subjectDescriptor);

        mReadOperations.push_back(std::move(readOperation));
        app::Testing::ReadOperation& readOpeationRef = *mReadOperations.back().get();

        std::unique_ptr<app::AttributeValueEncoder> encoder = readOpeationRef.StartEncoding();
        app::DataModel::ActionReturnStatus status = cluster.ReadAttribute(readOpeationRef.GetRequest(), *encoder);
        VerifyOrReturnError(status.IsSuccess(), status);
        ReturnErrorOnFailure(readOpeationRef.FinishEncoding());

        std::vector<app::Testing::DecodedAttributeData> attributeData;
        ReturnErrorOnFailure(readOpeationRef.GetEncodedIBs().Decode(attributeData));
        VerifyOrReturnError(attributeData.size() == 1u, CHIP_ERROR_INCORRECT_STATE);

        return app::DataModel::Decode(attributeData[0].dataReader, out);
    }

    // Read attribute into `out` parameter.
    // The `out` parameter must be of the correct type for the attribute being read.
    // Use `app::Clusters::<ClusterName>::Attributes::<AttributeName>::TypeInfo::DecodableType` for the `out` parameter to be spec compliant (see the comment of the class for usage example).
    // @returns `CHIP_ERROR_INCORRECT_STATE` if no default cluster is set. (see constructor or `SetDefaultCluster` or use other overload specifying the cluster)
    template <typename T>
    app::DataModel::ActionReturnStatus ReadAttribute(const app::DataModel::ReadAttributeRequest & path, T & out)
    {
        VerifyOrReturnError(verifyDefaultClusterSet(), CHIP_ERROR_INCORRECT_STATE);
        return ReadAttribute(*mDefaultCluster, path, out);
    }

    // Read attribute into `out` parameter.
    // The `out` parameter must be of the correct type for the attribute being read.
    // Use `app::Clusters::<ClusterName>::Attributes::<AttributeName>::TypeInfo::DecodableType` for the `out` parameter to be spec compliant (see the comment of the class for usage example).
    // Will construct the attribute path using the first path returned by `GetPaths()` on the cluster.
    // For specifying a different endpoint or cluster, or some read or other flags, use the overload taking a `ReadAttributeRequest`
    template <typename T>
    app::DataModel::ActionReturnStatus ReadAttribute(app::ServerClusterInterface & cluster, AttributeId attr_id, T & out)
    {
        VerifyOrReturnError(verifyClusterPathsValid(cluster), CHIP_ERROR_INCORRECT_STATE);
        const auto & paths = cluster.GetPaths();
        app::ConcreteAttributePath attributePath(paths[0].mEndpointId, paths[0].mClusterId, attr_id);
        app::DataModel::ReadAttributeRequest request;
        request.path = attributePath;
        return ReadAttribute(cluster, request, out);
    }

    // Read attribute into `out` parameter.
    // The `out` parameter must be of the correct type for the attribute being read.
    // Use `app::Clusters::<ClusterName>::Attributes::<AttributeName>::TypeInfo::DecodableType` for the `out` parameter to be spec compliant (see the comment of the class for usage example).
    // Will construct the attribute path using the first path returned by `GetPaths()` on the cluster.
    // For specifying a different endpoint or cluster, or some read or other flags, use the overload taking a `ReadAttributeRequest`
    // @returns `CHIP_ERROR_INCORRECT_STATE` if no default cluster is set. (see constructor or `SetDefaultCluster` or use other overload specifying the cluster)
    template <typename T>
    app::DataModel::ActionReturnStatus ReadAttribute(AttributeId attr_id, T & out)
    {
        VerifyOrReturnError(verifyDefaultClusterSet(), CHIP_ERROR_INCORRECT_STATE);
        return ReadAttribute(*mDefaultCluster, attr_id, out);
    }


    // Write attribute from `value` parameter.
    // The `value` parameter must be of the correct type for the attribute being written.
    // Use `app::Clusters::<ClusterName>::Attributes::<AttributeName>::TypeInfo::Type` for the `value` parameter to be spec compliant (see the comment of the class for usage example).
    template <typename T>
    app::DataModel::ActionReturnStatus WriteAttribute(app::ServerClusterInterface & cluster, const app::DataModel::WriteAttributeRequest & path, T & value)
    {
        app::Testing::WriteOperation writeOperation(path.path);
        writeOperation.SetWriteFlags(path.writeFlags).SetOperationFlags(path.operationFlags);
        if (path.subjectDescriptor) writeOperation.SetSubjectDescriptor(*path.subjectDescriptor);

        app::AttributeValueDecoder decoder = writeOperation.DecoderFor(value);
        return cluster.WriteAttribute(writeOperation.GetRequest(), decoder);
    }

    // Write attribute from `value` parameter.
    // The `value` parameter must be of the correct type for the attribute being written.
    // Use `app::Clusters::<ClusterName>::Attributes::<AttributeName>::TypeInfo::Type` for the `value` parameter to be spec compliant (see the comment of the class for usage example).
    // @returns `CHIP_ERROR_INCORRECT_STATE` if no default cluster is set. (see constructor or `SetDefaultCluster` or use other overload specifying the cluster)
    template <typename T>
    app::DataModel::ActionReturnStatus WriteAttribute(const app::DataModel::WriteAttributeRequest & path, T & value)
    {
        VerifyOrReturnError(verifyDefaultClusterSet(), CHIP_ERROR_INCORRECT_STATE);
        return WriteAttribute(*mDefaultCluster, path, value);
    }

    // Write attribute from `value` parameter.
    // The `value` parameter must be of the correct type for the attribute being written.
    // Use `app::Clusters::<ClusterName>::Attributes::<AttributeName>::TypeInfo::Type` for the `value` parameter to be spec compliant (see the comment of the class for usage example).
    // Will construct the attribute path using the first path returned by `GetPaths()` on the cluster.
    // For specifying a different endpoint or cluster, or some write or other flags, use the overload taking a `WriteAttributeRequest`
    template <typename T>
    app::DataModel::ActionReturnStatus WriteAttribute(app::ServerClusterInterface & cluster, AttributeId attr_id, T & value)
    {
        VerifyOrReturnError(verifyClusterPathsValid(cluster), CHIP_ERROR_INCORRECT_STATE);
        const auto & paths = cluster.GetPaths();
        app::ConcreteAttributePath attributePath(paths[0].mEndpointId, paths[0].mClusterId, attr_id);
        app::DataModel::WriteAttributeRequest request;
        request.path = attributePath;
        return WriteAttribute(cluster, request, value);
    }

    // Write attribute from `value` parameter.
    // The `value` parameter must be of the correct type for the attribute being written.
    // Use `app::Clusters::<ClusterName>::Attributes::<AttributeName>::TypeInfo::Type` for the `value` parameter to be spec compliant (see the comment of the class for usage example).
    // Will construct the attribute path using the first path returned by `GetPaths()` on the cluster.
    // For specifying a different endpoint or cluster, or some write or other flags, use the overload taking a `WriteAttributeRequest`
    // @returns `CHIP_ERROR_INCORRECT_STATE` if no default cluster is set. (see constructor or `SetDefaultCluster` or use other overload specifying the cluster)
    template <typename T>
    app::DataModel::ActionReturnStatus WriteAttribute(AttributeId attr_id, T & value)
    {
        VerifyOrReturnError(verifyDefaultClusterSet(), CHIP_ERROR_INCORRECT_STATE);
        return WriteAttribute(*mDefaultCluster, attr_id, value);
    }

    // Invoke a command with `arguments`.
    // The `arguments` parameter must be of the correct type for the command being invoked.
    // Use `app::Clusters::<ClusterName>::Commands::<CommandName>::Type` for the `arguments` parameter to be spec compliant (see the comment of the class for usage example).
    // TODO: Not Implemented
    template <typename T>
    std::optional<std::variant<app::DataModel::ActionReturnStatus, CommandResponse>> InvokeCommand(app::ServerClusterInterface & cluster, const app::DataModel::InvokeRequest & request, const T & arguments);

    // Invoke a command with `arguments`.
    // The `arguments` parameter must be of the correct type for the command being invoked.
    // Use `app::Clusters::<ClusterName>::Commands::<CommandName>::Type` for the `arguments` parameter to be spec compliant (see the comment of the class for usage example).
    // @returns `CHIP_ERROR_INCORRECT_STATE` if no default cluster is set. (see constructor or `SetDefaultCluster` or use other overload specifying the cluster)
    template <typename T>
    std::optional<std::variant<app::DataModel::ActionReturnStatus, CommandResponse>> InvokeCommand(const app::DataModel::InvokeRequest & request, const T & arguments)
    {
        VerifyOrReturnError(verifyDefaultClusterSet(), CHIP_ERROR_INCORRECT_STATE);
        return InvokeCommand(*mDefaultCluster, request, arguments);
    }

    // Invoke a command with `arguments`.
    // The `arguments` parameter must be of the correct type for the command being invoked.
    // Use `app::Clusters::<ClusterName>::Commands::<CommandName>::Type` for the `arguments` parameter to be spec compliant (see the comment of the class for usage example).
    // Will construct the command path using the first path returned by `GetPaths()` on the cluster.
    // For specifying a different endpoint or cluster, or some invoke or other flags, use the overload taking a `InvokeRequest`
    template <typename T>
    std::optional<std::variant<app::DataModel::ActionReturnStatus, CommandResponse>> InvokeCommand(app::ServerClusterInterface & cluster, CommandId command_id, const T & arguments)
    {
        VerifyOrReturnError(verifyClusterPathsValid(cluster), CHIP_ERROR_INCORRECT_STATE);
        const auto & paths = cluster.GetPaths();
        app::ConcreteCommandPath commandPath(paths[0].mEndpointId, paths[0].mClusterId, command_id);
        app::DataModel::InvokeRequest request;
        request.path = commandPath;
        return InvokeCommand(cluster, request, arguments);
    }

    // Invoke a command with `arguments`.
    // The `arguments` parameter must be of the correct type for the command being invoked.
    // Use `app::Clusters::<ClusterName>::Commands::<CommandName>::Type` for the `arguments` parameter to be spec compliant (see the comment of the class for usage example).
    // Will construct the command path using the first path returned by `GetPaths()` on the cluster.
    // For specifying a different endpoint or cluster, or some invoke or other flags, use the overload taking a `InvokeRequest`
    // @returns `CHIP_ERROR_INCORRECT_STATE` if no default cluster is set. (see constructor or `SetDefaultCluster` or use other overload specifying the cluster)
    template <typename T>
    std::optional<std::variant<app::DataModel::ActionReturnStatus, CommandResponse>> InvokeCommand(CommandId command_id, const T & arguments)
    {
        VerifyOrReturnError(verifyDefaultClusterSet(), CHIP_ERROR_INCORRECT_STATE);
        return InvokeCommand(*mDefaultCluster, command_id, arguments);
    }

    // Returns the next generated event from the event generator in the test server cluster context
    std::optional<LogOnlyEvents::EventInformation> GetNextGeneratedEvent() { return mTestServerClusterContext.EventsGenerator().GetNextEvent(); }

    // Compare the attributes of the cluster against the expected set.
    bool TestAttributes(app::ServerClusterInterface & cluster, const app::ConcreteClusterPath & path, Span<app::DataModel::AttributeEntry> expected)
    {
        ReadOnlyBufferBuilder<app::DataModel::AttributeEntry> attributesBuilder;
        if (cluster.Attributes(path, attributesBuilder) != CHIP_NO_ERROR) return false;
        return Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expected);
    }

    // Compare the attributes of the cluster against the expected set.
    // @returns `false` if no default cluster is set. (see constructor or `SetDefaultCluster` or use other overload specifying the cluster)
    bool TestAttributes(const app::ConcreteClusterPath & path, Span<app::DataModel::AttributeEntry> expected)
    {
        if (!verifyDefaultClusterSet()) return false;
        return TestAttributes(*mDefaultCluster, path, expected);
    }

    // Compare the attributes of the cluster against the expected set.
    // Will use the first path returned by `GetPaths()` on the cluster.
    // For specifying a different cluster path, use the overload taking a `ConcreteClusterPath`
    bool TestAttributes(app::ServerClusterInterface & cluster, Span<app::DataModel::AttributeEntry> expected)
    {
        if(!verifyClusterPathsValid(cluster)) return false;
        const auto & paths = cluster.GetPaths();
        return TestAttributes(cluster, app::ConcreteClusterPath(paths[0].mEndpointId, paths[0].mClusterId), expected);
    }

    // Compare the attributes of the cluster against the expected set.
    // Will use the first path returned by `GetPaths()` on the cluster.
    // For specifying a different cluster path, use the overload taking a `ConcreteClusterPath`
    // @returns `false` if no default cluster is set. (see constructor or `SetDefaultCluster` or use other overload specifying the cluster)
    bool TestAttributes(Span<app::DataModel::AttributeEntry> expected)
    {
        if(verifyDefaultClusterSet()) return false;
        return TestAttributes(*mDefaultCluster, expected);
    }

    // Compare the accepted commands of the cluster against the expected set.
    bool TestAcceptedCommands(app::ServerClusterInterface & cluster, const app::ConcreteClusterPath & path, Span<app::DataModel::AcceptedCommandEntry> expected)
    {
        ReadOnlyBufferBuilder<app::DataModel::AcceptedCommandEntry> commandsBuilder;
        if (cluster.AcceptedCommands(path, commandsBuilder) != CHIP_NO_ERROR) return false;
        return Testing::EqualAcceptedCommandSets(commandsBuilder.TakeBuffer(), expected);
    }

    // Compare the accepted commands of the cluster against the expected set.
    // @returns `false` if no default cluster is set. (see constructor or `SetDefaultCluster` or use other overload specifying the cluster)
    bool TestAcceptedCommands(const app::ConcreteClusterPath & path, Span<app::DataModel::AcceptedCommandEntry> expected)
    {
        if (!verifyDefaultClusterSet()) return false;
        return TestAcceptedCommands(*mDefaultCluster, path, expected);
    }

    // Compare the accepted commands of the cluster against the expected set.
    // Will use the first path returned by `GetPaths()` on the cluster.
    // For specifying a different cluster path, use the overload taking a `ConcreteClusterPath`
    bool TestAcceptedCommands(app::ServerClusterInterface & cluster, Span<app::DataModel::AcceptedCommandEntry> expected)
    {
        if(!verifyClusterPathsValid(cluster)) return false;
        const auto & paths = cluster.GetPaths();
        return TestAcceptedCommands(cluster, app::ConcreteClusterPath(paths[0].mEndpointId, paths[0].mClusterId), expected);
    }

    // Compare the accepted commands of the cluster against the expected set.
    // Will use the first path returned by `GetPaths()` on the cluster.
    // For specifying a different cluster path, use the overload taking a `ConcreteClusterPath`
    // @returns `false` if no default cluster is set. (see constructor or `SetDefaultCluster` or use other overload specifying the cluster)
    bool TestAcceptedCommands(Span<app::DataModel::AcceptedCommandEntry> expected)
    {
        if(!verifyDefaultClusterSet()) return false;
        return TestAcceptedCommands(*mDefaultCluster, expected);
    }


    // Compare the generated commands of the cluster against the expected set.
    bool TestGeneratedCommands(app::ServerClusterInterface & cluster, const app::ConcreteClusterPath & path, Span<CommandId> expected)
    {
        ReadOnlyBufferBuilder<CommandId> commandsBuilder;
        if (cluster.GeneratedCommands(path, commandsBuilder) != CHIP_NO_ERROR) return false;
        return Testing::EqualGeneratedCommandSets(commandsBuilder.TakeBuffer(), expected);
    }

    // Compare the generated commands of the cluster against the expected set.
    // @returns `false` if no default cluster is set. (see constructor or `SetDefaultCluster` or use other overload specifying the cluster)
    bool TestGeneratedCommands(const app::ConcreteClusterPath & path, Span<CommandId> expected)
    {
        if (!verifyDefaultClusterSet()) return false;
        return TestGeneratedCommands(*mDefaultCluster, path, expected);
    }

    // Compare the generated commands of the cluster against the expected set.
    // Will use the first path returned by `GetPaths()` on the cluster.
    // For specifying a different cluster path, use the overload taking a `ConcreteClusterPath`
    bool TestGeneratedCommands(app::ServerClusterInterface & cluster, Span<CommandId> expected)
    {
        if(!verifyClusterPathsValid(cluster)) return false;
        const auto & paths = cluster.GetPaths();
        return TestGeneratedCommands(cluster, app::ConcreteClusterPath(paths[0].mEndpointId, paths[0].mClusterId), expected);
    }

    // Compare the generated commands of the cluster against the expected set.
    // Will use the first path returned by `GetPaths()` on the cluster.
    // For specifying a different cluster path, use the overload taking a `ConcreteClusterPath`
    // @returns `false` if no default cluster is set. (see constructor or `SetDefaultCluster` or use other overload specifying the cluster)
    bool TestGeneratedCommands(Span<CommandId> expected)
    {
        if(!verifyDefaultClusterSet()) return false;
        return TestGeneratedCommands(*mDefaultCluster, expected);
    }

private:

    bool verifyDefaultClusterSet()
    {
        if (!mDefaultCluster)
        {
            ChipLogError(Test, "Default cluster not set. (see ClusterTester::SetDefaultCluster, or use another overload specifying the cluster)");
            return false;
        }
        return true;
    }

    bool verifyClusterPathsValid(app::ServerClusterInterface & cluster)
    {
        const auto & paths = cluster.GetPaths();
        if (paths.size() == 0)
        {
            ChipLogError(Test, "Cluster has no paths registered (GetPaths returned empty list)");
            return false;
        }
        return true;
    }

    TestServerClusterContext mTestServerClusterContext{};
    app::ServerClusterInterface * mDefaultCluster{nullptr};
    std::list<std::unique_ptr<app::Testing::ReadOperation>> mReadOperations;
};

} // namespace Test
} // namespace chip
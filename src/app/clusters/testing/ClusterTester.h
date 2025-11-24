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
#include <access/AccessControl.h>
#include <access/examples/ExampleAccessControlDelegate.h>
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
#include <app/data-model-provider/tests/TestConstants.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/data-model/NullObject.h>
#include <app/reporting/ReportSchedulerImpl.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server/Server.h>
#include <credentials/tests/CHIPCert_unit_test_vectors.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLVReader.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <platform/DefaultTimerDelegate.h>

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
    class ClusterTester {
    public:
        ClusterTester(app::ServerClusterInterface & cluster)
            : mCluster(cluster)
        {
        }

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
            std::unique_ptr<app::Testing::ReadOperation> readOperation = std::make_unique<app::Testing::ReadOperation>(path.mEndpointId, path.mClusterId, attr_id);

            mReadOperations.push_back(std::move(readOperation));
            app::Testing::ReadOperation & readOperationRef = *mReadOperations.back().get();

            std::unique_ptr<app::AttributeValueEncoder> encoder = readOperationRef.StartEncoding();
            app::DataModel::ActionReturnStatus status = mCluster.ReadAttribute(readOperationRef.GetRequest(), *encoder);
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

        // Result structure for Invoke operations, containing both status and decoded response.
        template <typename ResponseType>
        struct InvokeResult {
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

            const app::DataModel::InvokeRequest invokeRequest = { .path = { paths[0].mEndpointId, paths[0].mClusterId, commandId } };

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
            if (!result.status.has_value()) {
                if (mHandler.HasResponse()) {
                    // A data response was added, so the command is successful.
                    result.status = app::DataModel::ActionReturnStatus(CHIP_NO_ERROR);
                } else if (mHandler.HasStatus()) {
                    // A status response was added. Use the last one.
                    result.status = app::DataModel::ActionReturnStatus(mHandler.GetLastStatus().status);
                } else {
                    // Neither response nor status was provided; this is unexpected.
                    // This would happen either in error (as mentioned here) or if the command is supposed
                    // to be handled asynchronously. ClusterTester does not support such asynchronous processing.
                    result.status = app::DataModel::ActionReturnStatus(CHIP_ERROR_INCORRECT_STATE);
                    ChipLogError(
                        Test, "InvokeCommand returned nullopt, but neither HasResponse nor HasStatus is true. Setting error status.");
                }
            }

            // If command was successful and there's a response, decode it (skip for NullObjectType)
            if constexpr (!std::is_same_v<ResponseType, app::DataModel::NullObjectType>) {
                if (result.status.has_value() && result.status->IsSuccess() && mHandler.HasResponse()) {
                    ResponseType decodedResponse;
                    CHIP_ERROR decodeError = mHandler.DecodeResponse(decodedResponse);
                    if (decodeError == CHIP_NO_ERROR) {
                        result.response = std::move(decodedResponse);
                    } else {
                        // Decode failed; reflect error in status and log
                        result.status = app::DataModel::ActionReturnStatus(decodeError);
                        ChipLogError(Test, "DecodeResponse failed: %s", decodeError.AsString());
                    }
                }
            }

            return result;
        }

        // Returns the next generated event from the event generator in the test server cluster context
        std::optional<LogOnlyEvents::EventInformation> GetNextGeneratedEvent()
        {
            return mTestServerClusterContext.EventsGenerator().GetNextEvent();
        }

        // Static helper methods for initializing Server and FabricTable for fabric-scoped attribute testing
        //
        // These methods should be called in SetUpTestSuite() and TearDownTestSuite() respectively
        // when testing fabric-scoped attributes that require Server::GetInstance().GetFabricTable()
        // to be populated (e.g., Access Control Cluster ACL attribute).
        //
        // Example usage:
        //   struct TestMyCluster : public ::testing::Test
        //   {
        //       static void SetUpTestSuite()
        //       {
        //           ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        //           ASSERT_EQ(chip::Test::ClusterTester::InitServerForFabricScopedTesting(), CHIP_NO_ERROR);
        //       }
        //       static void TearDownTestSuite()
        //       {
        //           chip::Test::ClusterTester::ShutdownServerForFabricScopedTesting();
        //           chip::Platform::MemoryShutdown();
        //       }
        //   };
        //
        // This initializes Server::GetInstance() with minimal configuration and adds a test fabric
        // with index matching the fabric index used in test subject descriptors (kDenySubjectDescriptor).
        static CHIP_ERROR InitServerForFabricScopedTesting()
        {
            auto & state = GetFabricScopedTestServerState();

            // Prevent multiple initializations
            if (state.serverInitialized) {
                return CHIP_NO_ERROR;
            }

            // Allocate static resources
            state.storage = new chip::TestPersistentStorageDelegate();
            state.timerDelegate = new chip::app::DefaultTimerDelegate();
            state.reportScheduler = new chip::app::reporting::ReportSchedulerImpl(state.timerDelegate);

            // Initialize Server::GetInstance() to enable fabric-scoped attribute reading
            // This is needed because some clusters (e.g., Access Control, Group Key Management) iterate over
            // Server::GetInstance().GetFabricTable() when reading fabric-scoped attributes
            chip::CommonCaseDeviceServerInitParams initParams;
            initParams.persistentStorageDelegate = state.storage; // Use test storage instead of KVS
            initParams.reportScheduler = state.reportScheduler;
            ReturnErrorOnFailure(initParams.InitializeStaticResourcesBeforeServerInit());
            // InitializeStaticResourcesBeforeServerInit() already sets accessDelegate to a default value
            // which is sufficient for most clusters. If a specific cluster needs a custom AccessControl
            // delegate, it should be set up separately in the test.
            initParams.dataModelProvider = chip::app::CodegenDataModelProviderInstance(state.storage);
            initParams.operationalServicePort = 0; // Use any available port

            // Initialize Server
            ReturnErrorOnFailure(chip::Server::GetInstance().Init(initParams));

            // Add a test fabric with index matching kDenySubjectDescriptor
            // This allows fabric-scoped attribute readers to find the fabric when reading entries
            const chip::FabricIndex testFabricIndex = chip::app::Testing::kDenySubjectDescriptor.fabricIndex;
            chip::FabricTable & fabricTable = chip::Server::GetInstance().GetFabricTable();

            // Set the fabric index for the next addition
            ReturnErrorOnFailure(fabricTable.SetFabricIndexForNextAddition(testFabricIndex));

            // Add fabric using test certificates
            using namespace chip::TestCerts;
            chip::ByteSpan rcacSpan = GetRootACertAsset().mCert;
            chip::ByteSpan icacSpan = GetIAA1CertAsset().mCert;
            chip::ByteSpan nocSpan = GetNodeA1CertAsset().mCert;
            chip::ByteSpan opKeySpan = GetNodeA1CertAsset().mKey;

            chip::FabricIndex addedFabricIndex = chip::kUndefinedFabricIndex;
            ReturnErrorOnFailure(fabricTable.AddNewFabricForTest(rcacSpan, icacSpan, nocSpan, opKeySpan, &addedFabricIndex));
            VerifyOrReturnError(addedFabricIndex == testFabricIndex, CHIP_ERROR_INTERNAL);

            state.serverInitialized = true;
            return CHIP_NO_ERROR;
        }

        static void ShutdownServerForFabricScopedTesting()
        {
            auto & state = GetFabricScopedTestServerState();

            if (!state.serverInitialized) {
                return;
            }

            chip::Server::GetInstance().Shutdown();
            // AccessControl is cleaned up by Server::Shutdown(), no need to call Finish() separately

            // Clean up static resources
            delete state.reportScheduler;
            state.reportScheduler = nullptr;
            delete state.timerDelegate;
            state.timerDelegate = nullptr;
            delete state.storage;
            state.storage = nullptr;

            state.serverInitialized = false;
        }

    private:
        // Shared state for fabric-scoped testing server initialization
        struct FabricScopedTestServerState {
            chip::TestPersistentStorageDelegate * storage = nullptr;
            chip::app::DefaultTimerDelegate * timerDelegate = nullptr;
            chip::app::reporting::ReportSchedulerImpl * reportScheduler = nullptr;
            bool serverInitialized = false;
        };

        static FabricScopedTestServerState & GetFabricScopedTestServerState()
        {
            static FabricScopedTestServerState state;
            return state;
        }

        bool VerifyClusterPathsValid()
        {
            auto paths = mCluster.GetPaths();
            if (paths.size() != 1) {
                ChipLogError(Test, "cluster.GetPaths() did not return exactly one path");
                return false;
            }
            return true;
        }

        TestServerClusterContext mTestServerClusterContext {};
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

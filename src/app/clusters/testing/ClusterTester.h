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
#include <app/ConcreteCommandPath.h>
#include <app/clusters/testing/CommandTesting.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLVReader.h>
#include <memory>

namespace chip {
namespace Test {

    // Helper class for testing clusters.
    //
    // This class ensures that data read by attribute is referencing valid memory for one
    // read request (e.g. for case where the underlying read references a list or string that
    // points to TLV data).
    //
    // Calling `ReadAttribute` again may invalidate the result of a previous `ReadAttribute`.
    //
    // Read/Write of all attribute types should work, but make sure to use ::Type for encoding
    // and ::DecodableType for decoding structure types.
    //
    // Example of usage:
    //
    // FixedLabelCluster fixedLabel(kRootEndpointId);
    //
    // ClusterTester tester(fixedLabel);
    // uint32_t features{};
    // ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, features), CHIP_NO_ERROR);
    //
    // DataModel::DecodableList<Structs::LabelStruct::DecodableType> labelList;
    // ASSERT_EQ(tester.ReadAttribute(LabelList::Id, labelList), CHIP_NO_ERROR);
    // auto it = labelList.begin();
    // while (it.Next())
    // {
    //     ASSERT_GT(it.GetValue().label.size(), 0u);
    // }
    class ClusterTester {
    public:
        ClusterTester(chip::app::ServerClusterInterface & cluster)
            : mCluster(cluster)
        {
        }

        template <typename T>
        CHIP_ERROR ReadAttribute(AttributeId attr, T & value)
        {
            const auto & paths = mCluster.GetPaths();
            // To make the API simpler, we require the attribute id only as input and deduce
            // the full path based on the cluster GetPaths(). This does require a single path
            // supported to not be ambigous. This is most often the case (e.g. DefaultServerCluster
            // does this).
            VerifyOrReturnError(paths.size() == 1u, CHIP_ERROR_INCORRECT_STATE);
            chip::app::ConcreteAttributePath attributePath(paths[0].mEndpointId, paths[0].mClusterId, attr);
            mReadOperation = std::make_unique<chip::app::Testing::ReadOperation>(attributePath);

            std::unique_ptr<chip::app::AttributeValueEncoder> encoder = mReadOperation->StartEncoding();
            ReturnErrorOnFailure(mCluster.ReadAttribute(mReadOperation->GetRequest(), *encoder).GetUnderlyingError());
            ReturnErrorOnFailure(mReadOperation->FinishEncoding());

            std::vector<chip::app::Testing::DecodedAttributeData> attributeData;
            ReturnErrorOnFailure(mReadOperation->GetEncodedIBs().Decode(attributeData));
            VerifyOrReturnError(attributeData.size() == 1u, CHIP_ERROR_INCORRECT_STATE);

            return chip::app::DataModel::Decode(attributeData[0].dataReader, value);
        }

        template <typename T>
        CHIP_ERROR WriteAttribute(AttributeId attr, const T & value)
        {
            const auto & paths = mCluster.GetPaths();
            // To make the API simpler, we require the attribute id only as input and deduce
            // the full path based on the cluster GetPaths(). This does require a single path
            // supported to not be ambigous. This is most often the case (e.g. DefaultServerCluster
            // does this).
            VerifyOrReturnError(paths.size() == 1u, CHIP_ERROR_INCORRECT_STATE);
            chip::app::ConcreteAttributePath attributePath(paths[0].mEndpointId, paths[0].mClusterId, attr);
            chip::app::Testing::WriteOperation writeOperation(attributePath);
            chip::app::AttributeValueDecoder decoder = writeOperation.DecoderFor(value);
            return mCluster.WriteAttribute(writeOperation.GetRequest(), decoder).GetUnderlyingError();
        };

        void InvokeOperation(EndpointId endpoint, ClusterId cluster, CommandId command)
        {
            mCommandPath = chip::app::ConcreteCommandPath(endpoint, cluster, command);
            mHandler = std::make_unique<chip::app::Testing::MockCommandHandler>();
            mRequest.path = mCommandPath;
        }

        void InvokeOperation(const app::ConcreteCommandPath & path)
        {
            mCommandPath = path;
            mHandler = std::make_unique<chip::app::Testing::MockCommandHandler>();
            mRequest.path = mCommandPath;
        }

        void InvokeOperation(const app::ConcreteCommandPath & path, std::unique_ptr<chip::app::Testing::MockCommandHandler> handler)
        {
            mCommandPath = path;
            mHandler = std::move(handler);
            mRequest.path = mCommandPath;
        }

        void InvokeOperation(EndpointId endpoint, ClusterId cluster, CommandId command,
            std::unique_ptr<chip::app::Testing::MockCommandHandler> handler)
        {
            mCommandPath = app::ConcreteCommandPath(endpoint, cluster, command);
            mHandler = std::move(handler);
            mRequest.path = mCommandPath;
        }

        // Invoke a command using a predefined request structure
        template <typename RequestType>
        std::optional<app::DataModel::ActionReturnStatus> Invoke(const RequestType & request)
        {
            CHIP_ERROR err = CHIP_NO_ERROR;
            TLV::TLVWriter writer;
            writer.Init(mTlvBuffer);

            // Setup reader (declare before any SuccessOrExit calls)
            TLV::TLVReader reader;

            // Use the request's built-in Encode method
            SuccessOrExit(err = request.Encode(writer, TLV::AnonymousTag()));
            SuccessOrExit(err = writer.Finalize());

            reader.Init(mTlvBuffer, writer.GetLengthWritten());
            SuccessOrExit(err = reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

            // Invoke the command
            return mCluster.InvokeCommand(mRequest, reader, mHandler.get());

        exit:
            return std::make_optional(app::DataModel::ActionReturnStatus(err));
        }

        // Simplified overload to invoke a command with just the command ID and data.
        // Builds the request automatically using the internal cluster's paths.
        // Ideal for quick tests without manual request construction.
        template <typename T>
        std::optional<chip::app::DataModel::ActionReturnStatus> Invoke(chip::CommandId commandId, const T & data,
            app::CommandHandler * handler = nullptr)
        {
            const auto & paths = mCluster.GetPaths();
            VerifyOrReturnError(paths.size() == 1u, CHIP_ERROR_INCORRECT_STATE);
            const chip::app::DataModel::InvokeRequest request = { .path = { paths[0].mEndpointId, paths[0].mClusterId, commandId } };

            uint8_t buffer[kTlvBufferSize];
            chip::TLV::TLVWriter tlvWriter;
            tlvWriter.Init(buffer);
            ReturnErrorOnFailure(data.Encode(tlvWriter, chip::TLV::AnonymousTag()));

            chip::TLV::TLVReader tlvReader;
            tlvReader.Init(buffer, tlvWriter.GetLengthWritten());
            ReturnErrorOnFailure(tlvReader.Next());

            // Use provided handler or internal one
            app::CommandHandler * effectiveHandler = handler ? handler : mHandler.get();

            return mCluster.InvokeCommand(request, tlvReader, effectiveHandler);
        }

        // Get the command handler to inspect responses
        chip::app::Testing::MockCommandHandler & GetHandler() { return *mHandler; }
        const chip::app::Testing::MockCommandHandler & GetHandler() const { return *mHandler; }

        // Convenience method to decode the response
        template <typename ResponseType>
        CHIP_ERROR DecodeResponse(ResponseType & response)
        {
            return mHandler->DecodeResponse(response);
        }

        // Get the invoke request (path and metadata)
        const app::DataModel::InvokeRequest & GetRequest() const { return mRequest; }

    private:
        chip::app::ServerClusterInterface & mCluster;
        std::unique_ptr<chip::app::Testing::ReadOperation> mReadOperation;

        // Buffer size for TLV encoding/decoding of command payloads.
        // 256 bytes was chosen as a conservative upper bound for typical command payloads in tests.
        // All command payloads used in tests must fit within this buffer; tests with larger payloads will fail.
        // If protocol or test requirements change, this value may need to be increased.
        static constexpr size_t kTlvBufferSize = 256;

        app::ConcreteCommandPath mCommandPath;
        app::DataModel::InvokeRequest mRequest;
        std::unique_ptr<chip::app::Testing::MockCommandHandler> mHandler;
        uint8_t mTlvBuffer[kTlvBufferSize];
    };

} // namespace Test
} // namespace chip

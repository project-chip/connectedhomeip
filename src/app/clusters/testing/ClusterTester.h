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
            mHandler.ClearResponses();
            mHandler.ClearStatuses();
            mRequest.path = mCommandPath;
        }

        void InvokeOperation(const chip::app::ConcreteCommandPath & path)
        {
            mCommandPath = path;
            mHandler.ClearResponses();
            mHandler.ClearStatuses();
            mRequest.path = mCommandPath;
        }

        // Invoke a command using a predefined request structure
        template <typename RequestType>
        [[nodiscard]] std::optional<chip::app::DataModel::ActionReturnStatus> Invoke(const RequestType & request)
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
        [[nodiscard]] std::optional<chip::app::DataModel::ActionReturnStatus> Invoke(chip::CommandId commandId, const T & data,
            chip::app::CommandHandler * handler = nullptr)
        {
            const auto & paths = mCluster.GetPaths();
            VerifyOrReturnError(paths.size() == 1u, CHIP_ERROR_INCORRECT_STATE);
            const chip::app::DataModel::InvokeRequest request = { .path = { paths[0].mEndpointId, paths[0].mClusterId, commandId } };

            chip::TLV::TLVWriter tlvWriter;
            tlvWriter.Init(mTlvBuffer);
            ReturnErrorOnFailure(data.Encode(tlvWriter, chip::TLV::AnonymousTag()));

            chip::TLV::TLVReader tlvReader;
            tlvReader.Init(mTlvBuffer, tlvWriter.GetLengthWritten());
            ReturnErrorOnFailure(tlvReader.Next());

            // Use provided handler or internal one
            if (handler == nullptr) {
                handler = &mHandler;
            }

            return mCluster.InvokeCommand(request, tlvReader, handler);
        }

        chip::app::Testing::MockCommandHandler & GetHandler() { return mHandler; }
        const chip::app::Testing::MockCommandHandler & GetHandler() const { return mHandler; }

        template <typename ResponseType>
        CHIP_ERROR DecodeResponse(ResponseType & response)
        {
            return mHandler.DecodeResponse(response);
        }

        const chip::app::DataModel::InvokeRequest & GetRequest() const { return mRequest; }

    private:
        chip::app::ServerClusterInterface & mCluster;
        std::unique_ptr<chip::app::Testing::ReadOperation> mReadOperation;

        // Buffer size for TLV encoding/decoding of command payloads.
        // 256 bytes was chosen as a conservative upper bound for typical command payloads in tests.
        // All command payloads used in tests must fit within this buffer; tests with larger payloads will fail.
        // If protocol or test requirements change, this value may need to be increased.
        static constexpr size_t kTlvBufferSize = 256;

        chip::app::ConcreteCommandPath mCommandPath;
        chip::app::DataModel::InvokeRequest mRequest;
        chip::app::Testing::MockCommandHandler mHandler;
        uint8_t mTlvBuffer[kTlvBufferSize];
    };

} // namespace Test
} // namespace chip

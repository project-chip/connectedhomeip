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
#include <lib/support/CodeUtils.h>
#include <messaging/ExchangeContext.h>

#include <memory>
#include <vector>

namespace chip {
namespace app {
    namespace Testing {

        // Mock CommandHandler for testing command invocations
        class MockCommandHandler : public CommandHandler {
        public:
            struct ResponseRecord {
                ConcreteCommandPath path;
                CommandId commandId;
                chip::System::PacketBufferHandle encodedData;
            };

            struct StatusRecord {
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

            CHIP_ERROR AddResponseData(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                const DataModel::EncodableToTLV & aEncodable) override;

            void AddResponse(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                const DataModel::EncodableToTLV & aEncodable) override;

            bool IsTimedInvoke() const override { return false; }
            void FlushAcksRightAwayOnSlowCommand() override {}
            Access::SubjectDescriptor GetSubjectDescriptor() const override { return Access::SubjectDescriptor {}; }
            Messaging::ExchangeContext * GetExchangeContext() const override { return nullptr; }

            // Helper methods to extract response data
            bool HasResponse() const { return !mResponse.encodedData.IsNull(); }
            CommandId GetResponseCommandId() const { return mResponse.commandId; }
            const ResponseRecord & GetResponse() const { return mResponse; }

            // Helper methods to access stored statuses
            bool HasStatus() const { return !mStatuses.empty(); }
            const std::vector<StatusRecord> & GetStatuses() const { return mStatuses; }
            const StatusRecord & GetLastStatus() const { return mStatuses.back(); }
            void ClearStatuses() { mStatuses.clear(); }

            // Get a TLV reader positioned at the response data fields
            CHIP_ERROR GetResponseReader(TLV::TLVReader & reader) const;

            // Decode response into a specific DecodableType
            template <typename ResponseType>
            CHIP_ERROR DecodeResponse(ResponseType & response) const
            {
                TLV::TLVReader reader;
                ReturnErrorOnFailure(GetResponseReader(reader));
                return response.Decode(reader);
            }

            // Configuration methods
            void SetFabricIndex(FabricIndex index) { mFabricIndex = index; }

        private:
            ResponseRecord mResponse;
            std::vector<StatusRecord> mStatuses;
            FabricIndex mFabricIndex = 1; // Default to 1 to maintain backward compatibility
        };

        // Helper class for invoking commands in tests.
        // Manages TLV encoding/decoding and command handler setup.
        class InvokeOperation {
        public:
            InvokeOperation(EndpointId endpoint, ClusterId cluster, CommandId command)
                : mCommandPath(endpoint, cluster, command)
                , mHandler(std::make_unique<MockCommandHandler>())
            {
                mRequest.path = mCommandPath;
            }

            InvokeOperation(const ConcreteCommandPath & path)
                : mCommandPath(path)
                , mHandler(std::make_unique<MockCommandHandler>())
            {
                mRequest.path = mCommandPath;
            }

            // Invoke a command with parameters using variadic template
            // Use for commands that take no parameters (empty TLV structure) or multiple parameters
            template <typename ClusterType, typename... Args>
            std::optional<DataModel::ActionReturnStatus> Invoke(ClusterType & cluster, const Args &... args)
            {
                CHIP_ERROR err = CHIP_NO_ERROR;
                TLV::TLVWriter writer;
                writer.Init(mTlvBuffer);

                // Setup reader (declare before any SuccessOrExit calls)
                TLV::TLVReader reader;

                if constexpr (sizeof...(args) == 0) {
                    // No parameters - create empty TLV structure
                    TLV::TLVType outerContainer;
                    SuccessOrExit(err = writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainer));
                    SuccessOrExit(err = writer.EndContainer(outerContainer));
                } else if constexpr (sizeof...(args) == 1) {
                    // Single parameter - use the existing Encode method if available
                    using FirstArg = std::tuple_element_t<0, std::tuple<Args...>>;
                    if constexpr (std::is_same_v<decltype(std::declval<FirstArg>().Encode(std::declval<TLV::TLVWriter &>(),
                                                     std::declval<TLV::Tag>())),
                                      CHIP_ERROR>) {
                        // It's an encodable object
                        SuccessOrExit(err = std::get<0>(std::tie(args...)).Encode(writer, TLV::AnonymousTag()));
                    } else {
                        // It's a primitive type, encode as structure with single field
                        TLV::TLVType outerContainer;
                        SuccessOrExit(err = writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainer));
                        SuccessOrExit(err = DataModel::Encode(writer, TLV::ContextTag(0), std::get<0>(std::tie(args...))));
                        SuccessOrExit(err = writer.EndContainer(outerContainer));
                    }
                } else {
                    // Multiple parameters - encode as structure with multiple fields
                    TLV::TLVType outerContainer;
                    SuccessOrExit(err = writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainer));

                    int fieldIndex = 0;
                    auto encodeField = [&](const auto & arg) -> CHIP_ERROR {
                        return DataModel::Encode(writer, TLV::ContextTag(fieldIndex++), arg);
                    };

                    // Encode each field and check for errors
                    auto encodeAllFields = [&]() -> CHIP_ERROR {
                        CHIP_ERROR fieldErr = CHIP_NO_ERROR;
                        (
                            [&](const auto & arg) {
                                if (fieldErr == CHIP_NO_ERROR) {
                                    fieldErr = encodeField(arg);
                                }
                            }(args),
                            ...);
                        return fieldErr;
                    };

                    SuccessOrExit(err = encodeAllFields());
                    SuccessOrExit(err = writer.EndContainer(outerContainer));
                }
                SuccessOrExit(err = writer.Finalize());

                reader.Init(mTlvBuffer, writer.GetLengthWritten());
                SuccessOrExit(err = reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

                // Invoke the command
                return cluster.InvokeCommand(mRequest, reader, mHandler.get());

            exit:
                return std::make_optional(DataModel::ActionReturnStatus(err));
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

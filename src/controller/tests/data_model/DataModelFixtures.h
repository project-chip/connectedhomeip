/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

// This module provides shared fixture implementations of global functions
// for data model tests as well as global variables to control them.

#pragma once

#include <app/CommandHandler.h>
#include <app/data-model-provider/Provider.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Scoped.h>

namespace chip {
namespace app {
namespace DataModelTests {

constexpr EndpointId kTestEndpointId = 1;

constexpr ClusterId kPerpetualClusterId     = Test::MockClusterId(2);
constexpr AttributeId kPerpetualAttributeid = Test::MockAttributeId(1);

constexpr DataVersion kRejectedDataVersion = 1;
constexpr DataVersion kAcceptedDataVersion = 5;
constexpr DataVersion kDataVersion         = kAcceptedDataVersion;

constexpr uint8_t kExampleClusterSpecificSuccess = 11u;
constexpr uint8_t kExampleClusterSpecificFailure = 12u;

constexpr ClusterStatus kTestSuccessClusterStatus = 1;
constexpr ClusterStatus kTestFailureClusterStatus = 2;

// Controls how the fixture responds to attribute reads
enum class ReadResponseDirective
{
    kSendDataResponse,
    kSendManyDataResponses,          // Many data blocks, for a single concrete path
                                     // read, simulating a malicious server.
    kSendManyDataResponsesWrongPath, // Many data blocks, all using the wrong
                                     // path, for a single concrete path
                                     // read, simulating a malicious server.
    kSendDataError,
    kSendTwoDataErrors, // Multiple errors, for a single concrete path,
                        // simulating a malicious server.
};
extern ScopedChangeOnly<ReadResponseDirective> gReadResponseDirective;

// Number of reads of Clusters::UnitTesting::Attributes::Int16u that we have observed.
// Every read will increment this count by 1 and return the new value.
extern uint16_t gInt16uTotalReadCount;

// Controls the ICD operating mode for the fixture
extern ScopedChangeOnly<bool> gIsLitIcd;

// Controls how the fixture responds to attribute writes
enum class WriteResponseDirective
{
    kSendAttributeSuccess,
    kSendAttributeError,
    kSendMultipleSuccess,
    kSendMultipleErrors,
    kSendClusterSpecificSuccess,
    kSendClusterSpecificFailure,
};
extern ScopedChangeOnly<WriteResponseDirective> gWriteResponseDirective;

// Controls how the fixture responds to commands
enum class CommandResponseDirective
{
    kSendDataResponse,
    kSendSuccessStatusCode,
    kSendMultipleSuccessStatusCodes,
    kSendError,
    kSendMultipleErrors,
    kSendSuccessStatusCodeWithClusterStatus,
    kSendErrorWithClusterStatus,
    kAsync,
};
extern ScopedChangeOnly<CommandResponseDirective> gCommandResponseDirective;

// Populated with the command handle when gCommandResponseDirective == kAsync
extern CommandHandler::Handle gAsyncCommandHandle;

/// A customized class for read/write/invoke that matches functionality
/// with the ember-compatibility-functions functionality here.
///
/// TODO: these functions currently redirect to ember functions, so could
///       be merged with test-interaction-model-api.h/cpp as well. This is not done since
///       if we remove the direct ember dependency from IM, we can implement
///       distinct functional classes.
/// TODO items for above:
///      - once IM only supports DataModel
///      - break ember-overrides in this h/cpp file
class CustomDataModel : public CodegenDataModelProvider
{
public:
    static CustomDataModel & Instance();

    CHIP_ERROR Shutdown() override { return CHIP_NO_ERROR; }

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;
};

} // namespace DataModelTests
} // namespace app
} // namespace chip

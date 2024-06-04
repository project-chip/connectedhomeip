/*
 *    Copyright (c) 2024 Project CHIP Authors
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

/// test-interaction-model-api was created to consolidate and centralize stub functions that are used by the Interaction Model
/// during unit-testing.

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/WriteHandler.h>
#include <app/util/attribute-storage.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>

namespace chip {

namespace Test {

constexpr chip::ClusterId kTestDeniedClusterId1 = 1000;
constexpr chip::ClusterId kTestDeniedClusterId2 = 3;

constexpr chip::ClusterId kTestClusterId      = 6;
constexpr uint8_t kTestFieldValue1            = 1;
constexpr chip::EndpointId kTestEndpointId    = 1;
constexpr chip::DataVersion kTestDataVersion1 = 3;

constexpr chip::DataVersion kRejectedDataVersion = 1;
extern uint8_t attributeDataTLV[CHIP_CONFIG_DEFAULT_UDP_MTU_SIZE];
extern size_t attributeDataTLVLen;

} // namespace Test
namespace app {

CHIP_ERROR ReadSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, bool aIsFabricFiltered,
                                 const ConcreteReadAttributePath & aPath, AttributeReportIBs::Builder & aAttributeReports,
                                 AttributeEncodeState * apEncoderState);

bool IsClusterDataVersionEqual(const ConcreteClusterPath & aConcreteClusterPath, DataVersion aRequiredVersion);

CHIP_ERROR WriteSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, const ConcreteDataAttributePath & aPath,
                                  TLV::TLVReader & aReader, WriteHandler * aWriteHandler);
const EmberAfAttributeMetadata * GetAttributeMetadata(const ConcreteAttributePath & aConcreteClusterPath);

bool ConcreteAttributePathExists(const ConcreteAttributePath & aPath);
Protocols::InteractionModel::Status CheckEventSupportStatus(const ConcreteEventPath & aPath);

Protocols::InteractionModel::Status ServerClusterCommandExists(const ConcreteCommandPath & aRequestCommandPath);

void DispatchSingleClusterCommand(const ConcreteCommandPath & aRequestCommandPath, chip::TLV::TLVReader & aReader,
                                  CommandHandler * apCommandObj);

bool IsDeviceTypeOnEndpoint(DeviceTypeId deviceType, EndpointId endpoint);

} // namespace app
} // namespace chip

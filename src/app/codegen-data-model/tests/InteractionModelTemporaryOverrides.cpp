/*
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
#include <app/AttributeEncodeState.h>
#include <app/AttributeReportBuilder.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/WriteHandler.h>
#include <app/util/attribute-storage.h>

using chip::Protocols::InteractionModel::Status;

// TODO: most of the functions here are part of EmberCompatibilityFunctions and is NOT decoupled
//       from IM current, but it SHOULD be
//       Open issue https://github.com/project-chip/connectedhomeip/issues/34137 for this work.
namespace chip {
namespace app {

bool ConcreteAttributePathExists(const ConcreteAttributePath & aPath)
{
    // TODO: this is just a noop which may be potentially invalid
    return true;
}

bool IsClusterDataVersionEqual(const ConcreteClusterPath & aConcreteClusterPath, DataVersion aRequiredVersion)
{
    // TODO: this is just a noop which may be potentially invalid
    return true;
}

const EmberAfAttributeMetadata * GetAttributeMetadata(const ConcreteAttributePath & aPath)
{
    return emberAfLocateAttributeMetadata(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
}

Status ServerClusterCommandExists(const ConcreteCommandPath & aCommandPath)
{
    // TODO: this is just a noop which may be potentially invalid
    return Status::Success;
}

Status CheckEventSupportStatus(const ConcreteEventPath & aPath)
{
    // TODO: this is just a noop which may be potentially invalid
    return Status::Success;
}

CHIP_ERROR WriteSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, const ConcreteDataAttributePath & aPath,
                                  TLV::TLVReader & aReader, WriteHandler * apWriteHandler)
{
    // this is just to get things to compile. eventually this method should NOT be used
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ReadSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, bool aIsFabricFiltered,
                                 const ConcreteReadAttributePath & aPath, AttributeReportIBs::Builder & aAttributeReports,
                                 AttributeEncodeState * apEncoderState)
{
    // this is just to get things to compile. eventually this method should NOT be used
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace app
} // namespace chip

void MatterReportingAttributeChangeCallback(const chip::app::ConcreteAttributePath & aPath)
{
    // TODO: should we add logic to track these calls for test purposes?
}

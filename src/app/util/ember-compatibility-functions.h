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

#include <access/SubjectDescriptor.h>
#include <app/AttributeValueEncoder.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/ConcreteEventPath.h>
#include <app/WriteHandler.h>
#include <app/util/attribute-metadata.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {

/**
 *  Check whether the given cluster exists on the given endpoint and supports
 *  the given command.  If it does, Success will be returned.  If it does not,
 *  one of UnsupportedEndpoint, UnsupportedCluster, or UnsupportedCommand
 *  will be returned, depending on how the command fails to exist.
 */
Protocols::InteractionModel::Status ServerClusterCommandExists(const ConcreteCommandPath & aCommandPath);

/**
 *  Check whether concrete attribute path is an "existent attribute path" in spec terms.
 *  @param[in]    aPath                 The concrete path of the data being read.
 *  @retval  boolean   true if the concrete attribute path indicates an attribute that exists on the node.
 */
bool ConcreteAttributePathExists(const ConcreteAttributePath & aPath);

/**
 *  Fetch attribute value and version info and write to the AttributeReport provided.
 *  The ReadSingleClusterData will do everything required for encoding an attribute, i.e. it will try to put one or more
 * AttributeReportIB to the AttributeReportIBs::Builder.
 *  When the endpoint / cluster / attribute data specified by aPath does not exist, corresponding interaction
 * model error code will be put into aAttributeReports, and CHIP_NO_ERROR will be returned. If the data exists on the server, the
 * data (with tag kData) and the data version (with tag kDataVersion) will be put into aAttributeReports. TLVWriter error will be
 * returned if any error occurred while encoding these values. This function is implemented by CHIP as a part of cluster data
 * storage & management.
 *
 *  @param[in]    aSubjectDescriptor    The subject descriptor for the read.
 *  @param[in]    aPath                 The concrete path of the data being read.
 *  @param[in]    aAttributeReports      The TLV Builder for Cluter attribute builder.
 *
 *  @retval  CHIP_NO_ERROR on success
 */
CHIP_ERROR ReadSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, bool aIsFabricFiltered,
                                 const ConcreteReadAttributePath & aPath, AttributeReportIBs::Builder & aAttributeReports,
                                 AttributeEncodeState * apEncoderState);

/**
 * Returns the metadata of the attribute for the given path.
 *
 * @retval The metadata of the attribute, will return null if the given attribute does not exists.
 */
const EmberAfAttributeMetadata * GetAttributeMetadata(const ConcreteAttributePath & aPath);

/**
 * TODO: Document.
 */
CHIP_ERROR WriteSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, const ConcreteDataAttributePath & aPath,
                                  TLV::TLVReader & aReader, WriteHandler * apWriteHandler);

/**
 * Check if the given cluster has the given DataVersion.
 */
bool IsClusterDataVersionEqual(const ConcreteClusterPath & aConcreteClusterPath, DataVersion aRequiredVersion);

/**
 * Returns true if device type is on endpoint, false otherwise.
 */
bool IsDeviceTypeOnEndpoint(DeviceTypeId deviceType, EndpointId endpoint);

/**
 * Returns the event support status for the given event, as an interaction model status.
 */
Protocols::InteractionModel::Status CheckEventSupportStatus(const ConcreteEventPath & aPath);

} // namespace app
} // namespace chip

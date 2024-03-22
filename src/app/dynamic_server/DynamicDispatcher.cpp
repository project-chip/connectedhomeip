/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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
#include <app/util/ember-compatibility-functions.h>

#include <access/SubjectDescriptor.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/GlobalAttributes.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/MessageDef/StatusIB.h>
#include <app/WriteHandler.h>
#include <app/data-model/Decode.h>
#include <app/util/att-storage.h>
#include <app/util/endpoint-config-api.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <lib/core/TLV.h>
#include <protocols/interaction_model/Constants.h>

/**
 * This file defines the APIs needed to handle interaction model dispatch.
 * These are the APIs normally defined in
 * src/app/util/ember-compatibility-functions.cpp and the generated
 * IMClusterCommandHandler.cpp but we want a different implementation of these
 * to enable more dynamic behavior, since not all framework consumers will be
 * implementing the same server clusters.
 */
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

// TODO: Maybe consider making this configurable?  See also
// AccessControl.cpp.
constexpr EndpointId kSupportedEndpoint = 0;

} // anonymous namespace

namespace chip {
namespace app {

using Access::SubjectDescriptor;
using Protocols::InteractionModel::Status;

namespace {

bool IsSupportedGlobalAttribute(AttributeId aAttribute)
{
    // We don't have any non-global attributes.
    using namespace Globals::Attributes;

    for (auto & attr : GlobalAttributesNotInMetadata)
    {
        if (attr == aAttribute)
        {
            return true;
        }
    }

    switch (aAttribute)
    {
    case FeatureMap::Id:
        FALLTHROUGH;
    case ClusterRevision::Id:
        return true;
    }

    return false;
}

Status DetermineAttributeStatus(const ConcreteAttributePath & aPath, bool aIsWrite)
{
    // TODO: Consider making this configurable for applications that are not
    // trying to be an OTA provider, though in practice it just affects which
    // error is returned.
    if (aPath.mEndpointId != kSupportedEndpoint)
    {
        return Status::UnsupportedEndpoint;
    }

    // TODO: Consider making this configurable for applications that are not
    // trying to be an OTA provider, though in practice it just affects which
    // error is returned.
    if (aPath.mClusterId != OtaSoftwareUpdateProvider::Id)
    {
        return Status::UnsupportedCluster;
    }

    if (!IsSupportedGlobalAttribute(aPath.mAttributeId))
    {
        return Status::UnsupportedAttribute;
    }

    // No permissions for this for read, and none of these are writable for
    // write.  The writable-or-not check happens before the ACL check.
    return aIsWrite ? Status::UnsupportedWrite : Status::UnsupportedAccess;
}

} // anonymous namespace

CHIP_ERROR ReadSingleClusterData(const SubjectDescriptor & aSubjectDescriptor, bool aIsFabricFiltered,
                                 const ConcreteReadAttributePath & aPath, AttributeReportIBs::Builder & aAttributeReports,
                                 AttributeValueEncoder::AttributeEncodeState * aEncoderState)
{
    Status status = DetermineAttributeStatus(aPath, /* aIsWrite = */ false);
    return aAttributeReports.EncodeAttributeStatus(aPath, StatusIB(status));
}

bool ConcreteAttributePathExists(const ConcreteAttributePath & aPath)
{
    return DetermineAttributeStatus(aPath, /* aIsWrite = */ false) == Status::UnsupportedAccess;
}

Status ServerClusterCommandExists(const ConcreteCommandPath & aPath)
{
    // TODO: Consider making this configurable for applications that are not
    // trying to be an OTA provider.
    using namespace OtaSoftwareUpdateProvider::Commands;

    if (aPath.mEndpointId != kSupportedEndpoint)
    {
        return Status::UnsupportedEndpoint;
    }

    if (aPath.mClusterId != OtaSoftwareUpdateProvider::Id)
    {
        return Status::UnsupportedCluster;
    }

    switch (aPath.mCommandId)
    {
    case QueryImage::Id:
        FALLTHROUGH;
    case ApplyUpdateRequest::Id:
        FALLTHROUGH;
    case NotifyUpdateApplied::Id:
        return Status::Success;
    }

    return Status::UnsupportedCommand;
}

bool IsClusterDataVersionEqual(const ConcreteClusterPath & aConcreteClusterPath, DataVersion aRequiredVersion)
{
    // Will never be called anyway; we have no attributes.
    return false;
}

const EmberAfAttributeMetadata * GetAttributeMetadata(const ConcreteAttributePath & aConcreteClusterPath)
{
    // Note: This test does not make use of the real attribute metadata.
    static EmberAfAttributeMetadata stub = { .defaultValue = EmberAfDefaultOrMinMaxAttributeValue(uint32_t(0)) };
    return &stub;
}

bool IsDeviceTypeOnEndpoint(DeviceTypeId deviceType, EndpointId endpoint)
{
    return false;
}

CHIP_ERROR WriteSingleClusterData(const SubjectDescriptor & aSubjectDescriptor, const ConcreteDataAttributePath & aPath,
                                  TLV::TLVReader & aReader, WriteHandler * aWriteHandler)
{
    Status status = DetermineAttributeStatus(aPath, /* aIsWrite = */ true);
    return aWriteHandler->AddStatus(aPath, status);
}

// No attribute access overrides on iOS and Android for now.
// TODO (#16806): This function can be moved to InteractionModelEngine.
AttributeAccessInterface * GetAttributeAccessOverride(EndpointId endpointId, ClusterId clusterId)
{
    return nullptr;
}

void DispatchSingleClusterCommand(const ConcreteCommandPath & aPath, TLV::TLVReader & aReader, CommandHandler * aCommandObj)
{
    // This command passed ServerClusterCommandExists so we know it's one of our
    // supported commands.
    using namespace OtaSoftwareUpdateProvider::Commands;

    bool wasHandled = false;
    CHIP_ERROR err  = CHIP_NO_ERROR;

    switch (aPath.mCommandId)
    {
    case QueryImage::Id: {
        QueryImage::DecodableType commandData;
        err = DataModel::Decode(aReader, commandData);
        if (err == CHIP_NO_ERROR)
        {
            wasHandled = emberAfOtaSoftwareUpdateProviderClusterQueryImageCallback(aCommandObj, aPath, commandData);
        }
        break;
    }
    case ApplyUpdateRequest::Id: {
        ApplyUpdateRequest::DecodableType commandData;
        err = DataModel::Decode(aReader, commandData);
        if (err == CHIP_NO_ERROR)
        {
            wasHandled = emberAfOtaSoftwareUpdateProviderClusterApplyUpdateRequestCallback(aCommandObj, aPath, commandData);
        }
        break;
    }
    case NotifyUpdateApplied::Id: {
        NotifyUpdateApplied::DecodableType commandData;
        err = DataModel::Decode(aReader, commandData);
        if (err == CHIP_NO_ERROR)
        {
            wasHandled = emberAfOtaSoftwareUpdateProviderClusterNotifyUpdateAppliedCallback(aCommandObj, aPath, commandData);
        }
        break;
    }
    default:
        break;
    }

    if (CHIP_NO_ERROR != err || !wasHandled)
    {
        aCommandObj->AddStatus(aPath, Status::InvalidCommand);
    }
}

Protocols::InteractionModel::Status CheckEventSupportStatus(const ConcreteEventPath & aPath)
{
    return Protocols::InteractionModel::Status::UnsupportedEvent;
}

} // namespace app
} // namespace chip

/**
 * Called by the OTA provider cluster server to determine an index
 * into its array.
 */
uint16_t emberAfGetClusterServerEndpointIndex(EndpointId endpoint, ClusterId cluster, uint16_t fixedClusterServerEndpointCount)
{
    if (endpoint == kSupportedEndpoint && cluster == OtaSoftwareUpdateProvider::Id)
    {
        return 0;
    }

    return UINT16_MAX;
}

/**
 * Methods used by AttributePathExpandIterator, which need to exist
 * because it is part of libCHIP.  For AttributePathExpandIterator
 * purposes, for now, we just pretend like we have just our one
 * endpoint, the OTA Provider cluster, and no attributes (because we
 * would be erroring out from them anyway).
 */
uint16_t emberAfGetServerAttributeCount(EndpointId endpoint, ClusterId cluster)
{
    return 0;
}

uint16_t emberAfEndpointCount(void)
{
    return 1;
}

uint16_t emberAfIndexFromEndpoint(EndpointId endpoint)
{
    if (endpoint == kSupportedEndpoint)
    {
        return 0;
    }

    return UINT16_MAX;
}

EndpointId emberAfEndpointFromIndex(uint16_t index)
{
    // Index must be valid here, so 0.
    return kSupportedEndpoint;
}

Optional<ClusterId> emberAfGetNthClusterId(EndpointId endpoint, uint8_t n, bool server)
{
    if (endpoint == kSupportedEndpoint && n == 0 && server)
    {
        return MakeOptional(OtaSoftwareUpdateProvider::Id);
    }

    return NullOptional;
}

uint16_t emberAfGetServerAttributeIndexByAttributeId(EndpointId endpoint, ClusterId cluster, AttributeId attributeId)
{
    return UINT16_MAX;
}

bool emberAfContainsAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId)
{
    return false;
}

uint8_t emberAfClusterCount(EndpointId endpoint, bool server)
{
    if (endpoint == kSupportedEndpoint && server)
    {
        return 1;
    }

    return 0;
}

Optional<AttributeId> emberAfGetServerAttributeIdByIndex(EndpointId endpoint, ClusterId cluster, uint16_t attributeIndex)
{
    return NullOptional;
}

uint8_t emberAfClusterIndex(EndpointId endpoint, ClusterId clusterId, EmberAfClusterMask mask)
{
    if (endpoint == kSupportedEndpoint && clusterId == OtaSoftwareUpdateProvider::Id && (mask & CLUSTER_MASK_SERVER))
    {
        return 0;
    }

    return UINT8_MAX;
}

bool emberAfEndpointIndexIsEnabled(uint16_t index)
{
    return index == 0;
}

namespace {
const CommandId acceptedCommands[]  = { Clusters::OtaSoftwareUpdateProvider::Commands::QueryImage::Id,
                                        Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateRequest::Id,
                                        Clusters::OtaSoftwareUpdateProvider::Commands::NotifyUpdateApplied::Id, kInvalidCommandId };
const CommandId generatedCommands[] = { Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::Id,
                                        Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateResponse::Id, kInvalidCommandId };
const EmberAfCluster otaProviderCluster{
    .clusterId            = Clusters::OtaSoftwareUpdateProvider::Id,
    .attributes           = nullptr,
    .attributeCount       = 0,
    .clusterSize          = 0,
    .mask                 = CLUSTER_MASK_SERVER,
    .functions            = nullptr,
    .acceptedCommandList  = acceptedCommands,
    .generatedCommandList = generatedCommands,
    .eventList            = nullptr,
    .eventCount           = 0,
};
const EmberAfEndpointType otaProviderEndpoint{ .cluster = &otaProviderCluster, .clusterCount = 1, .endpointSize = 0 };
} // namespace

const EmberAfEndpointType * emberAfFindEndpointType(EndpointId endpoint)
{
    if (endpoint == kSupportedEndpoint)
    {
        return &otaProviderEndpoint;
    }

    return nullptr;
}

const EmberAfCluster * emberAfFindServerCluster(EndpointId endpoint, ClusterId cluster)
{
    if (endpoint == kSupportedEndpoint && cluster == Clusters::OtaSoftwareUpdateProvider::Id)
    {
        return &otaProviderCluster;
    }

    return nullptr;
}

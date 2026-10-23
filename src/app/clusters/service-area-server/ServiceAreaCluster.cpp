/**
 *
 *    Copyright (c) 2023-2026 Project CHIP Authors
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
 *
 */

#include <app/clusters/service-area-server/ServiceAreaCluster.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/interaction_model/StatusCode.h>

using Status = chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace ServiceArea {

// ****************************************************************************
// Service Area Cluster

ServiceAreaCluster::ServiceAreaCluster(EndpointId endpointId, StorageDelegate & storageDelegate, Delegate & delegate,
                                       BitMask<ServiceArea::Feature> feature, OptionalAttributeSet optionalAttributes) :
    DefaultServerCluster({ endpointId, Id }),
    mStorageDelegate(storageDelegate), mDelegate(delegate), mFeature(feature), mOptionalAttributes(optionalAttributes)
{
    ChipLogProgress(Zcl, "Service Area: cluster constructor");
}

//*************************************************************************
// core functions

DataModel::ActionReturnStatus ServiceAreaCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                AttributeValueEncoder & encoder)
{
    ChipLogDetail(Zcl, "Service Area: Reading attribute with ID " ChipLogFormatMEI, ChipLogValueMEI(request.path.mAttributeId));

    using namespace ServiceArea::Attributes;
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(ServiceArea::kRevision);
    case SupportedAreas::Id:
        return ReadSupportedAreas(encoder);
    case SupportedMaps::Id:
        return ReadSupportedMaps(encoder);
    case SelectedAreas::Id:
        return ReadSelectedAreas(encoder);
    case CurrentArea::Id:
        return encoder.Encode(GetCurrentArea());
    case EstimatedEndTime::Id:
        return encoder.Encode(GetEstimatedEndTime());
    case Progress::Id:
        return ReadProgress(encoder);
    case FeatureMap::Id:
        return encoder.Encode(static_cast<uint32_t>(mFeature.Raw()));
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

std::optional<DataModel::ActionReturnStatus> ServiceAreaCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                               chip::TLV::TLVReader & input_arguments,
                                                                               CommandHandler * handler)
{
    using namespace ServiceArea::Commands;
    switch (request.path.mCommandId)
    {
    case SelectAreas::Id: {
        SelectAreas::DecodableType req;
        ReturnErrorOnFailure(DataModel::Decode(input_arguments, req));
        return HandleSelectAreasCmd(request, req, handler);
    }
    case SkipArea::Id: {
        VerifyOrReturnValue(SupportsSkipArea(), Status::UnsupportedCommand);
        SkipArea::DecodableType req;
        ReturnErrorOnFailure(DataModel::Decode(input_arguments, req));
        return HandleSkipAreaCmd(request, req, handler);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR ServiceAreaCluster::Attributes(const ConcreteClusterPath & path,
                                          ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const DataModel::AttributeEntry optionalAttrs[] = {
        ServiceArea::Attributes::SupportedMaps::kMetadataEntry,
        ServiceArea::Attributes::CurrentArea::kMetadataEntry,
        ServiceArea::Attributes::EstimatedEndTime::kMetadataEntry,
        ServiceArea::Attributes::Progress::kMetadataEntry,
    };

    OptionalAttributeSet enabledOptionalAttributes = mOptionalAttributes;
    if (!mFeature.Has(Feature::kMaps))
    {
        enabledOptionalAttributes.template Set<ServiceArea::Attributes::SupportedMaps::Id>(false);
    }
    if (!mFeature.Has(Feature::kProgressReporting))
    {
        enabledOptionalAttributes.template Set<ServiceArea::Attributes::Progress::Id>(false);
    }

    return listBuilder.Append(Span(ServiceArea::Attributes::kMandatoryMetadata), Span(optionalAttrs), enabledOptionalAttributes);
}

bool ServiceAreaCluster::SupportsSkipArea() const
{
    if (mOptionalAttributes.IsSet(ServiceArea::Attributes::CurrentArea::Id))
    {
        return true;
    }

    return mFeature.Has(Feature::kProgressReporting) && mOptionalAttributes.IsSet(ServiceArea::Attributes::Progress::Id);
}

CHIP_ERROR ServiceAreaCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kSelectAreasCommand = ServiceArea::Commands::SelectAreas::kMetadataEntry;
    static constexpr DataModel::AcceptedCommandEntry kSkipAreaCommand    = ServiceArea::Commands::SkipArea::kMetadataEntry;

    if (SupportsSkipArea())
    {
        static constexpr DataModel::AcceptedCommandEntry kCommands[] = {
            kSelectAreasCommand,
            kSkipAreaCommand,
        };
        return builder.ReferenceExisting(Span(kCommands));
    }

    static constexpr DataModel::AcceptedCommandEntry kCommands[] = {
        kSelectAreasCommand,
    };
    return builder.ReferenceExisting(Span(kCommands));
}

//*************************************************************************
// attribute readers

CHIP_ERROR ServiceAreaCluster::ReadSupportedAreas(AttributeValueEncoder & aEncoder)
{
    if (GetNumberOfSupportedAreas() == 0)
    {
        return aEncoder.EncodeEmptyList();
    }

    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        uint8_t locationIndex = 0;
        AreaStructureWrapper supportedArea;

        while (GetSupportedAreaByIndex(locationIndex++, supportedArea))
        {
            ReturnErrorOnFailure(encoder.Encode(supportedArea));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR ServiceAreaCluster::ReadSupportedMaps(AttributeValueEncoder & aEncoder)
{
    if (GetNumberOfSupportedMaps() == 0)
    {
        return aEncoder.EncodeEmptyList();
    }

    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        uint32_t mapIndex = 0;
        MapStructureWrapper supportedMap;

        while (GetSupportedMapByIndex(mapIndex++, supportedMap))
        {
            ReturnErrorOnFailure(encoder.Encode(supportedMap));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR ServiceAreaCluster::ReadSelectedAreas(AttributeValueEncoder & aEncoder)
{
    if (GetNumberOfSelectedAreas() == 0)
    {
        return aEncoder.EncodeEmptyList();
    }

    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        uint32_t locationIndex = 0;
        uint32_t selectedArea;

        while (GetSelectedAreaByIndex(locationIndex++, selectedArea))
        {
            ReturnErrorOnFailure(encoder.Encode(selectedArea));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR ServiceAreaCluster::ReadProgress(AttributeValueEncoder & aEncoder)
{
    if (GetNumberOfProgressElements() == 0)
    {
        return aEncoder.EncodeEmptyList();
    }

    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        uint32_t locationIndex = 0;
        Structs::ProgressStruct::Type progressElement;

        while (GetProgressElementByIndex(locationIndex++, progressElement))
        {
            ReturnErrorOnFailure(encoder.Encode(progressElement));
        }
        return CHIP_NO_ERROR;
    });
}

//*************************************************************************
// command handlers

std::optional<DataModel::ActionReturnStatus>
ServiceAreaCluster::HandleSelectAreasCmd(const DataModel::InvokeRequest & request, const Commands::SelectAreas::DecodableType & req,
                                         CommandHandler * handler)
{
    ChipLogDetail(Zcl, "Service Area: HandleSelectAreasCmd");

    // On receipt of this command the device SHALL respond with a SelectAreasResponse command.
    auto exitResponse = [&request, handler](SelectAreasStatus status,
                                            CharSpan statusText) -> std::optional<DataModel::ActionReturnStatus> {
        Commands::SelectAreasResponse::Type response{
            .status     = status,
            .statusText = statusText,
        };
        handler->AddResponse(request.path, response);
        return std::nullopt;
    };

    size_t numberOfAreas = 0;
    // Get the number of Selected Areas in the command parameter and check that it is valid.
    {
        if (CHIP_NO_ERROR != req.newAreas.ComputeSize(&numberOfAreas))
        {
            return Status::InvalidCommand;
        }

        // If the device determines that it can't operate at all locations from the list,
        // the SelectAreasResponse command's Status field SHALL indicate InvalidSet.
        if (numberOfAreas > kMaxNumSelectedAreas)
        {
            return exitResponse(SelectAreasStatus::kInvalidSet, "invalid number of locations"_span);
        }
    }

    uint32_t selectedAreasBuffer[kMaxNumSelectedAreas];
    auto selectedAreasSpan         = Span<uint32_t>(selectedAreasBuffer, kMaxNumSelectedAreas);
    uint32_t numberOfSelectedAreas = 0;

    // Closure for checking if an area ID exists in the selectedAreasSpan
    auto areaAlreadyExists = [&numberOfSelectedAreas, &selectedAreasSpan](uint32_t areaId) {
        for (uint32_t i = 0; i < numberOfSelectedAreas; i++)
        {
            if (areaId == selectedAreasSpan[i])
            {
                return true;
            }
        }
        return false;
    };

    // if number of selected locations in parameter matches number in attribute - the locations *might* be the same
    bool matchesCurrentSelectedAreas = (numberOfAreas == GetNumberOfSelectedAreas());

    // do as much parameter validation as we can
    if (numberOfAreas != 0)
    {
        uint32_t ignoredIndex = 0;
        uint32_t oldSelectedArea;
        auto iAreaIter = req.newAreas.begin();
        while (iAreaIter.Next())
        {
            uint32_t selectedArea = iAreaIter.GetValue();

            // If aSelectedArea is already in selectedAreasSpan skip
            if (areaAlreadyExists(selectedArea))
            {
                continue;
            }

            // each item in this list SHALL match the AreaID field of an entry on the SupportedAreas attribute's list
            // If the Status field is set to UnsupportedArea, the StatusText field SHALL be an empty string.
            if (!mStorageDelegate.IsSupportedArea(selectedArea))
            {
                return exitResponse(SelectAreasStatus::kUnsupportedArea, ""_span);
            }

            // check to see if parameter list and attribute still match
            if (matchesCurrentSelectedAreas)
            {
                if (!GetSelectedAreaByIndex(ignoredIndex, oldSelectedArea) || (selectedArea != oldSelectedArea))
                {
                    matchesCurrentSelectedAreas = false;
                }
            }

            selectedAreasSpan[numberOfSelectedAreas] = selectedArea;
            numberOfSelectedAreas += 1;
        }

        // after iterating with Next through DecodableType - check for failure
        if (CHIP_NO_ERROR != iAreaIter.GetStatus())
        {
            return Status::InvalidCommand;
        }
    }

    selectedAreasSpan.reduce_size(numberOfSelectedAreas);

    // If the newAreas field is the same as the value of the SelectedAreas attribute
    // the SelectAreasResponse command SHALL have the Status field set to Success and
    // the StatusText field MAY be supplied with a human-readable string or include an empty string.
    if (matchesCurrentSelectedAreas)
    {
        return exitResponse(SelectAreasStatus::kSuccess, ""_span);
    }

    char delegateStatusBuffer[kMaxSizeStatusText];
    MutableCharSpan delegateStatusText(delegateStatusBuffer);

    // If the current state of the device doesn't allow for the locations to be selected,
    // the SelectAreasResponse command SHALL have the Status field set to InvalidInMode.
    // if the Status field is set to InvalidInMode, the StatusText field SHOULD indicate why the request is not allowed,
    // given the current mode of the device, which may involve other clusters.
    // (note - locationStatusText to be filled out by delegated function for if return value is false)
    if (!mDelegate.IsSetSelectedAreasAllowed(delegateStatusText))
    {
        return exitResponse(SelectAreasStatus::kInvalidInMode, delegateStatusText);
    }

    // Reset in case the delegate accidentally modified this string.
    delegateStatusText = MutableCharSpan(delegateStatusBuffer);

    // ask the device to handle SelectAreas Command
    // (note - locationStatusText to be filled out by delegated function for kInvalidInMode and InvalidSet)
    auto locationStatus = SelectAreasStatus::kSuccess;
    if (!mDelegate.IsValidSelectAreasSet(selectedAreasSpan, locationStatus, delegateStatusText))
    {
        return exitResponse(locationStatus, delegateStatusText);
    }

    {
        // If the device successfully accepts the request, the server will attempt to operate at the location(s)
        // indicated by the entries of the newArea field, when requested to operate,
        // the SelectAreasResponse command SHALL have the Status field set to Success,
        // and the SelectedAreas attribute SHALL be set to the value of the newAreas field.
        mStorageDelegate.ClearSelectedAreasRaw();

        if (numberOfAreas != 0)
        {
            uint32_t ignored;
            for (uint32_t areaId : selectedAreasSpan)
            {
                mStorageDelegate.AddSelectedAreaRaw(areaId, ignored);
            }
        }

        NotifySelectedAreasChanged();
    }

    return exitResponse(SelectAreasStatus::kSuccess, ""_span);
}

std::optional<DataModel::ActionReturnStatus> ServiceAreaCluster::HandleSkipAreaCmd(const DataModel::InvokeRequest & request,
                                                                                   const Commands::SkipArea::DecodableType & req,
                                                                                   CommandHandler * handler)
{
    ChipLogDetail(Zcl, "Service Area: HandleSkipArea");

    // On receipt of this command the device SHALL respond with a SkipAreaResponse command.
    auto exitResponse = [&request, handler](SkipAreaStatus status,
                                            CharSpan statusText) -> std::optional<DataModel::ActionReturnStatus> {
        Commands::SkipAreaResponse::Type response{
            .status     = status,
            .statusText = statusText,
        };
        handler->AddResponse(request.path, response);
        return std::nullopt;
    };

    // If the SelectedAreas attribute is empty, the SkipAreaResponse command’s Status field SHALL indicate InvalidAreaList.
    if (GetNumberOfSelectedAreas() == 0)
    {
        return exitResponse(SkipAreaStatus::kInvalidAreaList, ""_span);
    }

    // If the SkippedArea field does not match an entry in the SupportedAreas attribute, the SkipAreaResponse command’s Status field
    // SHALL indicate InvalidSkippedArea.
    if (!mStorageDelegate.IsSupportedArea(req.skippedArea))
    {
        ChipLogError(Zcl, "SkippedArea (%" PRIu32 ") is not in the SupportedAreas attribute.", req.skippedArea);
        return exitResponse(SkipAreaStatus::kInvalidSkippedArea, ""_span);
    }

    // have the device attempt to skip
    // If the Status field is not set to Success, or InvalidAreaList, the StatusText field SHALL include a vendor defined error
    // description. InvalidInMode | The received request cannot be handled due to the current mode of the device. (skipStatusText to
    // be filled out by delegated function on failure.)
    char skipStatusBuffer[kMaxSizeStatusText];
    MutableCharSpan skipStatusText(skipStatusBuffer);

    if (!mDelegate.HandleSkipArea(req.skippedArea, skipStatusText))
    {
        return exitResponse(SkipAreaStatus::kInvalidInMode, skipStatusText);
    }

    return exitResponse(SkipAreaStatus::kSuccess, ""_span);
}

//*************************************************************************
// attribute notifications

void ServiceAreaCluster::NotifySupportedAreasChanged()
{
    NotifyAttributeChanged(Attributes::SupportedAreas::Id);
}

void ServiceAreaCluster::NotifySupportedMapsChanged()
{
    NotifyAttributeChanged(Attributes::SupportedMaps::Id);
}

void ServiceAreaCluster::NotifySelectedAreasChanged()
{
    NotifyAttributeChanged(Attributes::SelectedAreas::Id);
}

void ServiceAreaCluster::NotifyCurrentAreaChanged()
{
    NotifyAttributeChanged(Attributes::CurrentArea::Id);
}

void ServiceAreaCluster::NotifyEstimatedEndTimeChanged()
{
    NotifyAttributeChanged(Attributes::EstimatedEndTime::Id);
}

void ServiceAreaCluster::NotifyProgressChanged()
{
    NotifyAttributeChanged(Attributes::Progress::Id);
}

// ****************************************************************************
//  Supported Areas manipulators

bool ServiceAreaCluster::IsValidSupportedArea(const AreaStructureWrapper & aArea)
{
    // If the LocationInfo field is null, the LandmarkInfo field SHALL NOT be null.
    // If the LandmarkInfo field is null, the LocationInfo field SHALL NOT be null.
    if (aArea.areaInfo.locationInfo.IsNull() && aArea.areaInfo.landmarkInfo.IsNull())
    {
        ChipLogDetail(Zcl, "IsValidAsSupportedArea %" PRIu32 " - must have locationInfo and/or LandmarkInfo", aArea.areaID);
        return false;
    }

    // If LocationInfo is not null, and its LocationName field is an empty string, at least one of the following SHALL NOT
    // be null: LocationInfo's FloorNumber field, LocationInfo's AreaType field, the LandmarkInfo
    if (!aArea.areaInfo.locationInfo.IsNull())
    {
        if (aArea.areaInfo.locationInfo.Value().locationName.empty() && aArea.areaInfo.locationInfo.Value().floorNumber.IsNull() &&
            aArea.areaInfo.locationInfo.Value().areaType.IsNull() && aArea.areaInfo.landmarkInfo.IsNull())
        {
            ChipLogDetail(
                Zcl, "IsValidAsSupportedArea %" PRIu32 " - AreaName is empty string, FloorNumber, AreaType, LandmarkInfo are null",
                aArea.areaID);
            return false;
        }
    }

    // The mapID field SHALL be null if SupportedMaps is not supported or SupportedMaps is an empty list.
    if (mFeature.Has(Feature::kMaps) && (GetNumberOfSupportedMaps() > 0))
    {
        if (aArea.mapID.IsNull())
        {
            ChipLogDetail(Zcl, "IsValidSupportedArea %" PRIu32 " - map Id should not be null when there are supported maps",
                          aArea.areaID);
            return false;
        }

        // If the SupportedMaps attribute is not null, mapID SHALL be the ID of an entry from the SupportedMaps attribute.
        if (!mStorageDelegate.IsSupportedMap(aArea.mapID.Value()))
        {
            ChipLogError(Zcl, "IsValidSupportedArea %" PRIu32 " - map Id %" PRIu32 " is not in supported map list", aArea.areaID,
                         aArea.mapID.Value());
            return false;
        }
    }
    else
    {
        if (!aArea.mapID.IsNull())
        {
            ChipLogDetail(Zcl, "IsValidSupportedArea %" PRIu32 " - map Id %" PRIu32 " is not in empty supported map list",
                          aArea.areaID, aArea.mapID.Value());
            return false;
        }
    }

    return true;
}

bool ServiceAreaCluster::IsUniqueSupportedArea(const AreaStructureWrapper & aArea, bool ignoreAreaId)
{
    uint32_t locationIndex = 0;
    AreaStructureWrapper entry;
    while (GetSupportedAreaByIndex(locationIndex++, entry))
    {
        if (!ignoreAreaId && (aArea.areaID == entry.areaID))
        {
            return false;
        }

        // If the SupportedMaps attribute is not empty, each entry in this list SHALL have a unique value for the combination of
        // the MapID and AreaInfo fields. If the SupportedMaps attribute is empty, each entry in this list SHALL have a unique
        // value for the AreaInfo field.
        BitMask<AreaStructureWrapper::IsEqualConfig> config;
        config.Set(AreaStructureWrapper::IsEqualConfig::kIgnoreAreaID);
        if (GetNumberOfSupportedMaps() == 0)
        {
            config.Set(AreaStructureWrapper::IsEqualConfig::kIgnoreMapId);
        }

        if (aArea.IsEqual(entry, config))
        {
            return false;
        }
    }

    return true;
}

bool ServiceAreaCluster::ReportEstimatedEndTimeChange(const DataModel::Nullable<uint32_t> & aEstimatedEndTime)
{
    if (mEstimatedEndTime == aEstimatedEndTime)
    {
        return false;
    }

    // The value of this attribute SHALL only be reported in the following cases:
    // - when it changes to or from null.
    if (mEstimatedEndTime.IsNull())
    {
        return true;
    }

    // - when it changes from 0
    if (mEstimatedEndTime.Value() == 0)
    {
        return true;
    }

    // - when it changes to null
    if (aEstimatedEndTime.IsNull())
    {
        return true;
    }

    // From this point we know that mEstimatedEndTime and aEstimatedEndTime are not null and not the same.

    // - when it changes to 0
    if (aEstimatedEndTime.Value() == 0)
    {
        return true;
    }

    // - when it decreases
    return (aEstimatedEndTime.Value() < mEstimatedEndTime.Value());
}

void ServiceAreaCluster::HandleSupportedAreasUpdated()
{
    // If there are no more Supported Areas, clear all selected areas, current area, and progress.
    if (GetNumberOfSupportedAreas() == 0)
    {
        ClearSelectedAreas();
        SetCurrentArea(DataModel::NullNullable);
        ClearProgress();
        return;
    }

    // Remove Selected Areas elements that do not exist is the Supported Areas attribute.
    {
        uint32_t i                  = 0;
        uint32_t areaId             = 0;
        uint32_t areasToRemoveIndex = 0;
        uint32_t areasToRemoveBuffer[kMaxNumSelectedAreas];
        Span<uint32_t> areasToRemoveSpan(areasToRemoveBuffer);

        while (GetSelectedAreaByIndex(i, areaId))
        {
            if (!mStorageDelegate.IsSupportedArea(areaId))
            {
                areasToRemoveSpan[areasToRemoveIndex] = areaId;
                areasToRemoveIndex++;
            }
            i++;
        }
        areasToRemoveSpan.reduce_size(areasToRemoveIndex);

        for (auto id : areasToRemoveSpan)
        {
            if (!RemoveSelectedAreas(id))
            {
                ChipLogError(Zcl, "HandleSupportedAreasUpdated: Failed to remove area %" PRIu32 " from selected areas", id);
            }
        }
    }

    // Set current Area to null if current area is not in the supported areas attribute.
    {
        auto currentAreaId = GetCurrentArea();
        if (!currentAreaId.IsNull() && !mStorageDelegate.IsSupportedArea(currentAreaId.Value()))
        {
            SetCurrentArea(DataModel::NullNullable);
        }
    }

    // Remove Progress elements associated with areas that do not exist is the Supported Areas attribute.
    {
        uint32_t i = 0;
        Structs::ProgressStruct::Type tempProgressElement;
        uint32_t progressToRemoveIndex = 0;
        uint32_t progressToRemoveBuffer[kMaxNumProgressElements];
        Span<uint32_t> progressToRemoveSpan(progressToRemoveBuffer);

        while (mStorageDelegate.GetProgressElementByIndex(i, tempProgressElement))
        {
            if (!mStorageDelegate.IsSupportedArea(tempProgressElement.areaID))
            {
                progressToRemoveSpan[progressToRemoveIndex] = tempProgressElement.areaID;
                progressToRemoveIndex++;
            }
            i++;
        }
        progressToRemoveSpan.reduce_size(progressToRemoveIndex);

        for (auto areaId : progressToRemoveSpan)
        {
            if (!mStorageDelegate.RemoveProgressElementRaw(areaId))
            {
                ChipLogError(Zcl, "HandleSupportedAreasUpdated: Failed to remove progress element with area ID %" PRIu32 "",
                             areaId);
            }
        }
    }
}

uint32_t ServiceAreaCluster::GetNumberOfSupportedAreas()
{
    return mStorageDelegate.GetNumberOfSupportedAreas();
}

bool ServiceAreaCluster::GetSupportedAreaByIndex(uint32_t listIndex, AreaStructureWrapper & aSupportedArea)
{
    return mStorageDelegate.GetSupportedAreaByIndex(listIndex, aSupportedArea);
}

bool ServiceAreaCluster::GetSupportedAreaById(uint32_t aAreaId, uint32_t & listIndex, AreaStructureWrapper & aSupportedArea)
{
    return mStorageDelegate.GetSupportedAreaById(aAreaId, listIndex, aSupportedArea);
}

bool ServiceAreaCluster::AddSupportedArea(AreaStructureWrapper & aNewArea)
{
    // Does device mode allow this attribute to be updated?
    if (!mDelegate.IsSupportedAreasChangeAllowed())
    {
        return false;
    }

    // Check there is space for the entry.
    if (GetNumberOfSupportedAreas() >= kMaxNumSupportedAreas)
    {
        ChipLogError(Zcl, "AddSupportedAreaRaw %" PRIu32 " - too many entries", aNewArea.areaID);
        return false;
    }

    // Verify cluster requirements concerning valid fields and field relationships.
    if (!IsValidSupportedArea(aNewArea))
    {
        ChipLogError(Zcl, "AddSupportedAreaRaw %" PRIu32 " - not a valid location object", aNewArea.areaID);
        return false;
    }

    // Each entry in Supported Areas SHALL have a unique value for the ID field.
    // If the SupportedMaps attribute is not null, each entry in this list SHALL have a unique value for the combination of the
    // MapID and AreaInfo fields. If the SupportedMaps attribute is null, each entry in this list SHALL have a unique value for
    // the AreaInfo field.
    if (!IsUniqueSupportedArea(aNewArea, false))
    {
        ChipLogError(Zcl, "AddSupportedAreaRaw %" PRIu32 " - not a unique location object", aNewArea.areaID);
        return false;
    }

    // Add the SupportedArea to the SupportedAreas attribute.
    uint32_t ignoredIndex;
    if (!mStorageDelegate.AddSupportedAreaRaw(aNewArea, ignoredIndex))
    {
        return false;
    }

    NotifySupportedAreasChanged();
    return true;
}

bool ServiceAreaCluster::ModifySupportedArea(AreaStructureWrapper & aNewArea)
{
    bool mapIDChanged = false;
    uint32_t listIndex;

    // get existing supported location to modify
    AreaStructureWrapper supportedArea;
    if (!GetSupportedAreaById(aNewArea.areaID, listIndex, supportedArea))
    {
        ChipLogError(Zcl, "ModifySupportedAreaRaw %" PRIu32 " - not a supported areaID", aNewArea.areaID);
        return false;
    }

    {
        // check for mapID change
        if ((aNewArea.mapID.IsNull() != supportedArea.mapID.IsNull()) ||
            (!aNewArea.mapID.IsNull() && !supportedArea.mapID.IsNull() && (aNewArea.mapID.Value() != supportedArea.mapID.Value())))
        {
            // does device mode allow this attribute to be updated?
            if (!mDelegate.IsSupportedAreasChangeAllowed())
            {
                return false;
            }
            mapIDChanged = true;
        }

        // verify cluster requirements concerning valid fields and field relationships
        if (!IsValidSupportedArea(aNewArea))
        {
            ChipLogError(Zcl, "ModifySupportedAreaRaw %" PRIu32 " - not a valid location object", aNewArea.areaID);
            return false;
        }

        // Updated location description must not match another existing location description.
        // We ignore comparing the area ID as one of the locations will match this one.
        if (!IsUniqueSupportedArea(aNewArea, true))
        {
            ChipLogError(Zcl, "ModifySupportedAreaRaw %" PRIu32 " - not a unique location object", aNewArea.areaID);
            return false;
        }

        // Replace the supported location with the modified location.
        if (!mStorageDelegate.ModifySupportedAreaRaw(listIndex, aNewArea))
        {
            return false;
        }
    }

    if (mapIDChanged)
    {
        HandleSupportedAreasUpdated();
    }

    NotifySupportedAreasChanged();
    return true;
}

bool ServiceAreaCluster::ClearSupportedAreas()
{
    // does device mode allow this attribute to be updated?
    if (!mDelegate.IsSupportedAreasChangeAllowed())
    {
        return false;
    }

    if (mStorageDelegate.ClearSupportedAreasRaw())
    {
        HandleSupportedAreasUpdated();
        NotifySupportedAreasChanged();
        return true;
    }

    return false;
}

bool ServiceAreaCluster::RemoveSupportedArea(uint32_t areaId)
{
    if (mStorageDelegate.RemoveSupportedAreaRaw(areaId))
    {
        HandleSupportedAreasUpdated();
        NotifySupportedAreasChanged();
        return true;
    }
    return false;
}

//*************************************************************************
// Supported Maps manipulators

uint32_t ServiceAreaCluster::GetNumberOfSupportedMaps()
{
    return mStorageDelegate.GetNumberOfSupportedMaps();
}

bool ServiceAreaCluster::GetSupportedMapByIndex(uint32_t listIndex, MapStructureWrapper & aSupportedMap)
{
    return mStorageDelegate.GetSupportedMapByIndex(listIndex, aSupportedMap);
}

bool ServiceAreaCluster::GetSupportedMapById(uint32_t aMapId, uint32_t & listIndex, MapStructureWrapper & aSupportedMap)
{
    return mStorageDelegate.GetSupportedMapById(aMapId, listIndex, aSupportedMap);
}

bool ServiceAreaCluster::AddSupportedMap(uint32_t aMapId, const CharSpan & aMapName)
{
    // check max# of list entries
    if (GetNumberOfSupportedMaps() >= kMaxNumSupportedMaps)
    {
        ChipLogError(Zcl, "AddSupportedMapRaw %" PRIu32 " - maximum number of entries", aMapId);
        return false;
    }

    //  Map name SHALL include readable text that describes the map name (cannot be empty string).
    if (aMapName.empty())
    {
        ChipLogError(Zcl, "AddSupportedMapRaw %" PRIu32 " - Name must not be empty string", aMapId);
        return false;
    }

    // Each entry in this list SHALL have a unique value for the Name field.
    uint8_t mapIndex = 0;
    MapStructureWrapper entry;

    while (GetSupportedMapByIndex(mapIndex++, entry))
    {
        // the name cannot be the same as an existing map
        if (entry.IsNameEqual(aMapName))
        {
            ChipLogError(Zcl, "AddSupportedMapRaw %" PRIu32 " - A map already exists with same name '%s'", aMapId,
                         NullTerminated(entry.GetName()).c_str());
            return false;
        }

        //  Each entry in this list SHALL have a unique value for the MapID field.
        if (aMapId == entry.mapID)
        {
            ChipLogError(Zcl, "AddSupportedMapRaw - non-unique Id %" PRIu32 "", aMapId);
            return false;
        }
    }

    {
        // add to supported maps attribute
        MapStructureWrapper newMap(aMapId, aMapName);
        uint32_t ignoredIndex;
        if (!mStorageDelegate.AddSupportedMapRaw(newMap, ignoredIndex))
        {
            return false;
        }
    }

    // map successfully added
    NotifySupportedMapsChanged();
    return true;
}

bool ServiceAreaCluster::RenameSupportedMap(uint32_t aMapId, const CharSpan & newMapName)
{
    uint32_t modifiedIndex;
    MapStructureWrapper modifiedMap;

    // get existing entry
    if (!GetSupportedMapById(aMapId, modifiedIndex, modifiedMap))
    {
        ChipLogError(Zcl, "RenameSupportedMap Id %" PRIu32 " - map does not exist", aMapId);
        return false;
    }

    //  Map name SHALL include readable text that describes the map's name. It cannot be empty string.
    if (newMapName.empty())
    {
        ChipLogError(Zcl, "RenameSupportedMap %" PRIu32 " - Name must not be empty string", aMapId);
        return false;
    }

    // update the local copy of the map
    modifiedMap.Set(modifiedMap.mapID, newMapName);

    // Each entry in this list SHALL have a unique value for the Name field.
    uint32_t loopIndex = 0;
    MapStructureWrapper entry;

    while (GetSupportedMapByIndex(loopIndex, entry))
    {
        if (modifiedIndex != loopIndex && entry.IsNameEqual(newMapName))
        {
            ChipLogError(Zcl, "RenameSupportedMap %" PRIu32 " - map already exists with same name '%s'", aMapId,
                         NullTerminated(entry.GetName()).c_str());
            return false;
        }

        ++loopIndex;
    }

    if (!mStorageDelegate.ModifySupportedMapRaw(modifiedIndex, modifiedMap))
    {
        return false;
    }

    // map successfully renamed
    NotifySupportedMapsChanged();
    return true;
}

bool ServiceAreaCluster::ClearSupportedMaps()
{
    // does device mode allow this attribute to be updated?
    if (!mDelegate.IsSupportedMapChangeAllowed())
    {
        return false;
    }

    if (mStorageDelegate.ClearSupportedMapsRaw())
    {
        ClearSupportedAreas();
        NotifySupportedMapsChanged();
        return true;
    }

    return false;
}

bool ServiceAreaCluster::RemoveSupportedMap(uint32_t mapId)
{
    if (!mStorageDelegate.RemoveSupportedMapRaw(mapId))
    {
        return false;
    }

    NotifySupportedMapsChanged();

    // If there are no supported maps left, none of the supported areas are vaild and their MapID needs to be null.
    if (GetNumberOfSupportedMaps() == 0)
    {
        ClearSupportedAreas();
        return true;
    }

    // Get the supported area IDs where the map ID matches the removed map ID.
    uint32_t supportedAreaIdsBuffer[kMaxNumSupportedAreas];
    Span<uint32_t> supportedAreaIdsSpan(supportedAreaIdsBuffer);
    {
        uint32_t supportedAreaIdsSize = 0;
        uint32_t supportedAreasIndex  = 0;
        AreaStructureWrapper tempSupportedArea;
        while (mStorageDelegate.GetSupportedAreaByIndex(supportedAreasIndex, tempSupportedArea))
        {
            if (tempSupportedArea.mapID == mapId)
            {
                supportedAreaIdsSpan[supportedAreaIdsSize] = tempSupportedArea.areaID;
                supportedAreaIdsSize++;
            }
            supportedAreasIndex++;
        }

        supportedAreaIdsSpan.reduce_size(supportedAreaIdsSize);
    }

    if (!supportedAreaIdsSpan.empty())
    {
        for (uint32_t supportedAreaId : supportedAreaIdsSpan)
        {
            mStorageDelegate.RemoveSupportedAreaRaw(supportedAreaId);
        }
        HandleSupportedAreasUpdated();
        NotifySupportedAreasChanged();
    }

    return true;
}

//*************************************************************************
// Selected Areas manipulators

uint32_t ServiceAreaCluster::GetNumberOfSelectedAreas()
{
    return mStorageDelegate.GetNumberOfSelectedAreas();
}

bool ServiceAreaCluster::GetSelectedAreaByIndex(uint32_t listIndex, uint32_t & selectedArea)
{
    return mStorageDelegate.GetSelectedAreaByIndex(listIndex, selectedArea);
}

bool ServiceAreaCluster::AddSelectedArea(uint32_t & aSelectedArea)
{
    // check max# of list entries
    if (GetNumberOfSelectedAreas() >= kMaxNumSelectedAreas)
    {
        ChipLogError(Zcl, "AddSelectedAreaRaw %" PRIu32 " - maximum number of entries", aSelectedArea);
        return false;
    }

    // each item in this list SHALL match the AreaID field of an entry on the SupportedAreas attribute's list
    if (!mStorageDelegate.IsSupportedArea(aSelectedArea))
    {
        ChipLogError(Zcl, "AddSelectedAreaRaw %" PRIu32 " - not a supported location", aSelectedArea);
        return false;
    }

    // each entry in this list SHALL have a unique value
    if (mStorageDelegate.IsSelectedArea(aSelectedArea))
    {
        ChipLogError(Zcl, "AddSelectedAreaRaw %" PRIu32 " - duplicated location", aSelectedArea);
        return false;
    }

    // Does device mode allow modification of selected locations?
    char locationStatusBuffer[kMaxSizeStatusText];
    MutableCharSpan locationStatusText(locationStatusBuffer);

    if (!mDelegate.IsSetSelectedAreasAllowed(locationStatusText))
    {
        ChipLogError(Zcl, "AddSelectedAreaRaw %" PRIu32 " - %s", aSelectedArea, NullTerminated(locationStatusText).c_str());
        return false;
    }

    uint32_t ignoredIndex;
    return mStorageDelegate.AddSelectedAreaRaw(aSelectedArea, ignoredIndex);
}

bool ServiceAreaCluster::ClearSelectedAreas()
{
    if (mStorageDelegate.ClearSelectedAreasRaw())
    {
        NotifySelectedAreasChanged();
        return true;
    }

    return false;
}

bool ServiceAreaCluster::RemoveSelectedAreas(uint32_t areaId)
{
    if (mStorageDelegate.RemoveSelectedAreasRaw(areaId))
    {
        NotifySelectedAreasChanged();
        return true;
    }

    return false;
}

//*************************************************************************
// Current Area manipulators

DataModel::Nullable<uint32_t> ServiceAreaCluster::GetCurrentArea()
{
    return mCurrentArea;
}

bool ServiceAreaCluster::SetCurrentArea(const DataModel::Nullable<uint32_t> & aCurrentArea)
{
    // If not null, the value of this attribute SHALL match the AreaID field of an entry on the SupportedAreas attribute's
    // list.
    if ((!aCurrentArea.IsNull()) && (!mStorageDelegate.IsSupportedArea(aCurrentArea.Value())))
    {
        ChipLogError(Zcl, "SetCurrentArea %" PRIu32 " - location is not supported", aCurrentArea.Value());
        return false;
    }

    bool notifyChange = mCurrentArea != aCurrentArea;

    mCurrentArea = aCurrentArea;
    if (notifyChange)
    {
        NotifyCurrentAreaChanged();
    }

    // EstimatedEndTime SHALL be null if the CurrentArea attribute is null.
    if (mCurrentArea.IsNull())
    {
        SetEstimatedEndTime(DataModel::NullNullable);
    }

    return true;
}

//*************************************************************************
// Estimated End Time manipulators

DataModel::Nullable<uint32_t> ServiceAreaCluster::GetEstimatedEndTime()
{
    return mEstimatedEndTime;
}

bool ServiceAreaCluster::SetEstimatedEndTime(const DataModel::Nullable<uint32_t> & aEstimatedEndTime)
{
    // EstimatedEndTime SHALL be null if the CurrentArea attribute is null.
    if (mCurrentArea.IsNull() && !aEstimatedEndTime.IsNull())
    {
        ChipLogError(Zcl, "SetEstimatedEndTime - must be null if Current Area is null");
        return false;
    }

    bool notifyChange = ReportEstimatedEndTimeChange(aEstimatedEndTime);

    mEstimatedEndTime = aEstimatedEndTime;

    if (notifyChange)
    {
        NotifyEstimatedEndTimeChanged();
    }

    // success
    return true;
}

//*************************************************************************
// Progress list manipulators

uint32_t ServiceAreaCluster::GetNumberOfProgressElements()
{
    return mStorageDelegate.GetNumberOfProgressElements();
}

bool ServiceAreaCluster::GetProgressElementByIndex(uint32_t listIndex, Structs::ProgressStruct::Type & aProgressElement)
{
    return mStorageDelegate.GetProgressElementByIndex(listIndex, aProgressElement);
}

bool ServiceAreaCluster::GetProgressElementById(uint32_t aAreaId, uint32_t & listIndex,
                                                Structs::ProgressStruct::Type & aProgressElement)
{
    return mStorageDelegate.GetProgressElementById(aAreaId, listIndex, aProgressElement);
}

bool ServiceAreaCluster::AddPendingProgressElement(uint32_t aAreaId)
{
    // create progress element
    Structs::ProgressStruct::Type inactiveProgress = { aAreaId, OperationalStatusEnum::kPending };

    // check max# of list entries
    if (GetNumberOfProgressElements() >= kMaxNumProgressElements)
    {
        ChipLogError(Zcl, "AddPendingProgressElement - maximum number of entries");
        return false;
    }

    // For each entry in this list, the AreaID field SHALL match an entry on the SupportedAreas attribute's list.
    if (!mStorageDelegate.IsSupportedArea(aAreaId))
    {
        ChipLogError(Zcl, "AddPendingProgressElement - not a supported location %" PRIu32 "", aAreaId);
        return false;
    }

    // Each entry in this list SHALL have a unique value for the AreaID field.
    if (mStorageDelegate.IsProgressElement(aAreaId))
    {
        ChipLogError(Zcl, "AddPendingProgressElement - progress element already exists for location %" PRIu32 "", aAreaId);
        return false;
    }

    uint32_t ignoredIndex;

    if (!mStorageDelegate.AddProgressElementRaw(inactiveProgress, ignoredIndex))
    {
        return false;
    }

    NotifyProgressChanged();
    return true;
}

bool ServiceAreaCluster::SetProgressStatus(uint32_t aAreaId, OperationalStatusEnum opStatus)
{
    uint32_t listIndex;
    Structs::ProgressStruct::Type progressElement;

    if (!GetProgressElementById(aAreaId, listIndex, progressElement))
    {
        ChipLogError(Zcl, "SetProgressStatus - progress element does not exist for location %" PRIu32 "", aAreaId);
        return false;
    }

    // If the status value is not changing, there in no need to modify the existing element.
    if (progressElement.status == opStatus)
    {
        return true;
    }

    // set the progress status in the local copy
    progressElement.status = opStatus;

    // TotalOperationalTime SHALL be null if the Status field is not set to Completed or Skipped.
    if ((opStatus != OperationalStatusEnum::kCompleted) && (opStatus != OperationalStatusEnum::kSkipped))
    {
        progressElement.totalOperationalTime.ClearValue();
    }

    // add the updated element to the progress attribute
    if (!mStorageDelegate.ModifyProgressElementRaw(listIndex, progressElement))
    {
        return false;
    }

    NotifyProgressChanged();
    return true;
}

bool ServiceAreaCluster::SetProgressTotalOperationalTime(uint32_t aAreaId,
                                                         const DataModel::Nullable<uint32_t> & aTotalOperationalTime)
{
    uint32_t listIndex;
    Structs::ProgressStruct::Type progressElement;

    if (!GetProgressElementById(aAreaId, listIndex, progressElement))
    {
        ChipLogError(Zcl, "SetProgressTotalOperationalTime - progress element does not exist for location %" PRIu32 "", aAreaId);
        return false;
    }

    // If the time value is not changing, there is no need to modify the existing element.
    if (progressElement.totalOperationalTime == aTotalOperationalTime)
    {
        return true;
    }

    // This attribute SHALL be null if the Status field is not set to Completed or Skipped
    if ((progressElement.status != OperationalStatusEnum::kCompleted &&
         progressElement.status != OperationalStatusEnum::kSkipped) &&
        !aTotalOperationalTime.IsNull())
    {
        ChipLogError(Zcl,
                     "SetProgressTotalOperationalTime - location %" PRIu32
                     " opStatus value %u - can be non-null only if opStatus is "
                     "Completed or Skipped",
                     aAreaId, to_underlying(progressElement.status));
        return false;
    }

    // set the time in the local copy
    progressElement.totalOperationalTime.Emplace(aTotalOperationalTime);

    // add the updated element to the progress attribute
    if (!mStorageDelegate.ModifyProgressElementRaw(listIndex, progressElement))
    {
        return false;
    }

    NotifyProgressChanged();
    return true;
}

bool ServiceAreaCluster::SetProgressEstimatedTime(uint32_t aAreaId, const DataModel::Nullable<uint32_t> & aEstimatedTime)
{
    uint32_t listIndex;
    Structs::ProgressStruct::Type progressElement;

    if (!GetProgressElementById(aAreaId, listIndex, progressElement))
    {
        ChipLogError(Zcl, "SetProgressEstimatedTime - progress element does not exist for location %" PRIu32 "", aAreaId);
        return false;
    }

    // If the time value is not changing, there is no need to modify the existing element.
    if (progressElement.estimatedTime == aEstimatedTime)
    {
        return true;
    };

    // set the time in the local copy
    progressElement.estimatedTime.Emplace(aEstimatedTime);

    // add the updated element to the progress attribute
    if (!mStorageDelegate.ModifyProgressElementRaw(listIndex, progressElement))
    {
        return false;
    }

    NotifyProgressChanged();
    return true;
}

bool ServiceAreaCluster::ClearProgress()
{
    if (mStorageDelegate.ClearProgressRaw())
    {
        NotifyProgressChanged();
        return true;
    }

    return false;
}

bool ServiceAreaCluster::RemoveProgressElement(uint32_t areaId)
{
    if (mStorageDelegate.RemoveProgressElementRaw(areaId))
    {
        NotifyProgressChanged();
        return true;
    }

    return false;
}

// attribute manipulators - Feature Map

bool ServiceAreaCluster::HasFeature(Feature feature) const
{
    return mFeature.Has(feature);
}

} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip

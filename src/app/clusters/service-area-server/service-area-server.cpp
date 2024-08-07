/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "service-area-server.h"
#include "service-area-delegate.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/interaction_model/StatusCode.h>

using Status = chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace ServiceArea {

// ****************************************************************************
// Service Area Server Instance

Instance::Instance(Delegate * aDelegate, EndpointId aEndpointId, BitMask<Feature> aFeature) :
    AttributeAccessInterface(MakeOptional(aEndpointId), Id), CommandHandlerInterface(MakeOptional(aEndpointId), Id),
    mDelegate(aDelegate), mEndpointId(aEndpointId), mClusterId(Id), mFeature(aFeature)
{
    ChipLogProgress(Zcl, "Service Area: Instance constructor");
    mDelegate->SetInstance(this);
}

Instance::~Instance()
{
    CommandHandlerInterfaceRegistry::UnregisterCommandHandler(this);
    unregisterAttributeAccessOverride(this);
}

CHIP_ERROR Instance::Init()
{
    ChipLogProgress(Zcl, "Service Area: INIT");

    // Check if the cluster has been selected in zap
    VerifyOrReturnError(emberAfContainsServer(mEndpointId, Id), CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(Zcl, "Service Area: The cluster with Id %lu was not enabled in zap.", long(Id)));

    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::RegisterCommandHandler(this));

    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);

    return mDelegate->Init();
}

//*************************************************************************
// core functions

CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    ChipLogDetail(Zcl, "Service Area: Reading attribute with ID " ChipLogFormatMEI, ChipLogValueMEI(aPath.mAttributeId));

    switch (aPath.mAttributeId)
    {

    case Attributes::SupportedAreas::Id:
        return ReadSupportedAreas(aEncoder);

    case Attributes::SupportedMaps::Id:
        return ReadSupportedMaps(aEncoder);

    case Attributes::SelectedAreas::Id:
        return ReadSelectedAreas(aEncoder);

    case Attributes::CurrentArea::Id:
        return aEncoder.Encode(GetCurrentArea());

    case Attributes::EstimatedEndTime::Id:
        return aEncoder.Encode(GetEstimatedEndTime());

    case Attributes::Progress::Id:
        return ReadProgress(aEncoder);

    case Attributes::FeatureMap::Id:
        return aEncoder.Encode(mFeature);

    default:
        ChipLogProgress(Zcl, "Service Area: Unsupported attribute with ID " ChipLogFormatMEI, ChipLogValueMEI(aPath.mAttributeId));
    }

    return CHIP_NO_ERROR;
}

void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::SelectAreas::Id:
        return CommandHandlerInterface::HandleCommand<Commands::SelectAreas::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleSelectAreasCmd(ctx, req); });

    case Commands::SkipArea::Id:
        return CommandHandlerInterface::HandleCommand<Commands::SkipArea::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleSkipCurrentAreaCmd(ctx); });
    }
}

//*************************************************************************
// attribute readers

CHIP_ERROR Instance::ReadSupportedAreas(AttributeValueEncoder & aEncoder)
{
    if (mDelegate->GetNumberOfSupportedAreas() == 0)
    {
        return aEncoder.EncodeEmptyList();
    }

    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        uint8_t locationIndex = 0;
        AreaStructureWrapper supportedLocation;

        while (mDelegate->GetSupportedLocationByIndex(locationIndex++, supportedLocation))
        {
            ReturnErrorOnFailure(encoder.Encode(supportedLocation));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR Instance::ReadSupportedMaps(AttributeValueEncoder & aEncoder)
{
    if (mDelegate->GetNumberOfSupportedMaps() == 0)
    {
        return aEncoder.EncodeEmptyList();
    }

    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        uint32_t mapIndex = 0;
        MapStructureWrapper supportedMap;

        while (mDelegate->GetSupportedMapByIndex(mapIndex++, supportedMap))
        {
            ReturnErrorOnFailure(encoder.Encode(supportedMap));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR Instance::ReadSelectedAreas(AttributeValueEncoder & aEncoder)
{
    if (mDelegate->GetNumberOfSelectedAreas() == 0)
    {
        return aEncoder.EncodeEmptyList();
    }

    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        uint32_t locationIndex = 0;
        uint32_t selectedLocation;

        while (mDelegate->GetSelectedLocationByIndex(locationIndex++, selectedLocation))
        {
            ReturnErrorOnFailure(encoder.Encode(selectedLocation));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR Instance::ReadProgress(AttributeValueEncoder & aEncoder)
{
    if (mDelegate->GetNumberOfProgressElements() == 0)
    {
        return aEncoder.EncodeEmptyList();
    }

    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        uint32_t locationIndex = 0;
        Structs::ProgressStruct::Type progressElement;

        while (mDelegate->GetProgressElementByIndex(locationIndex++, progressElement))
        {
            ReturnErrorOnFailure(encoder.Encode(progressElement));
        }
        return CHIP_NO_ERROR;
    });
}

//*************************************************************************
// command handlers

void Instance::HandleSelectAreasCmd(HandlerContext & ctx, const Commands::SelectAreas::DecodableType & req)
{
    ChipLogDetail(Zcl, "Service Area: HandleSelectAreasCmd");

    // On receipt of this command the device SHALL respond with a SelectAreasResponse command.
    auto exitResponse = [ctx](SelectAreasStatus status, CharSpan statusText) {
        Commands::SelectAreasResponse::Type response{
            .status     = status,
            .statusText = Optional(statusText),
        };
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    };

    size_t numberOfLocations = 0;
    // Get the number of Selected Locations in the command parameter and check that it is valid.
    {
        if (CHIP_NO_ERROR != req.newAreas.ComputeSize(&numberOfLocations))
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
            return;
        }

        // If the device determines that it can't operate at all locations from the list,
        // the SelectAreasResponse command's Status field SHALL indicate InvalidSet.
        if (numberOfLocations > kMaxNumSelectedAreas)
        {
            exitResponse(SelectAreasStatus::kInvalidSet, "invalid number of locations"_span);
            return;
        }
    }

    // if number of selected locations in parameter matches number in attribute - the locations *might* be the same
    bool matchesCurrentSelectedAreas = (numberOfLocations == mDelegate->GetNumberOfSelectedAreas());

    if (numberOfLocations != 0)
    {
        // do as much parameter validation as we can
        {
            uint32_t ignoredIndex = 0;
            uint32_t oldSelectedLocation;
            uint32_t i         = 0;
            auto iLocationIter = req.newAreas.begin();
            while (iLocationIter.Next())
            {
                uint32_t aSelectedLocation = iLocationIter.GetValue();

                // each item in this list SHALL match the AreaID field of an entry on the SupportedAreas attribute's list
                // If the Status field is set to UnsupportedLocation, the StatusText field SHALL be an empty string.
                if (!IsSupportedLocation(aSelectedLocation))
                {
                    exitResponse(SelectAreasStatus::kUnsupportedArea, ""_span);
                    return;
                }

                // Checking for duplicate locations.
                uint32_t j         = 0;
                auto jLocationIter = req.newAreas.begin();
                while (j < i)
                {
                    jLocationIter
                        .Next(); // Since j < i and i is valid, we can safely call Next() without checking the return value.
                    if (jLocationIter.GetValue() == aSelectedLocation)
                    {
                        exitResponse(SelectAreasStatus::kDuplicatedAreas, ""_span);
                        return;
                    }
                    j += 1;
                }

                // check to see if parameter list and attribute still match
                if (matchesCurrentSelectedAreas)
                {
                    if (!mDelegate->GetSelectedLocationByIndex(ignoredIndex, oldSelectedLocation) ||
                        (aSelectedLocation != oldSelectedLocation))
                    {
                        matchesCurrentSelectedAreas = false;
                    }
                }

                i += 1;
            }

            // after iterating with Next through DecodableType - check for failure
            if (CHIP_NO_ERROR != iLocationIter.GetStatus())
            {
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
                return;
            }
        }
    }

    // If the newAreas field is the same as the value of the SelectedAreas attribute
    // the SelectAreasResponse command SHALL have the Status field set to Success and
    // the StatusText field MAY be supplied with a human-readable string or include an empty string.
    if (matchesCurrentSelectedAreas)
    {
        exitResponse(SelectAreasStatus::kSuccess, ""_span);
        return;
    }

    char delegateStatusBuffer[kMaxSizeStatusText];
    MutableCharSpan delegateStatusText(delegateStatusBuffer);

    // If the current state of the device doesn't allow for the locations to be selected,
    // the SelectAreasResponse command SHALL have the Status field set to InvalidInMode.
    // if the Status field is set to InvalidInMode, the StatusText field SHOULD indicate why the request is not allowed,
    // given the current mode of the device, which may involve other clusters.
    // (note - locationStatusText to be filled out by delegated function for if return value is false)
    if (!mDelegate->IsSetSelectedAreasAllowed(delegateStatusText))
    {
        exitResponse(SelectAreasStatus::kInvalidInMode, delegateStatusText);
        return;
    }

    // Reset in case the delegate accidentally modified this string.
    delegateStatusText = MutableCharSpan(delegateStatusBuffer);

    // ask the device to handle SelectAreas Command
    // (note - locationStatusText to be filled out by delegated function for kInvalidInMode and InvalidSet)
    auto locationStatus = SelectAreasStatus::kSuccess;
    if (!mDelegate->IsValidSelectAreasSet(req, locationStatus, delegateStatusText))
    {
        exitResponse(locationStatus, delegateStatusText);
        return;
    }

    {
        // If the device successfully accepts the request, the server will attempt to operate at the location(s)
        // indicated by the entries of the newArea field, when requested to operate,
        // the SelectAreasResponse command SHALL have the Status field set to Success,
        // and the SelectedAreas attribute SHALL be set to the value of the newAreas field.
        mDelegate->ClearSelectedAreas();

        if (numberOfLocations != 0)
        {
            auto locationIter = req.newAreas.begin();
            uint32_t ignored;
            while (locationIter.Next())
            {
                mDelegate->AddSelectedLocation(locationIter.GetValue(), ignored);
            }
        }

        NotifySelectedAreasChanged();
    }

    exitResponse(SelectAreasStatus::kSuccess, ""_span);
}

void Instance::HandleSkipCurrentAreaCmd(HandlerContext & ctx)
{
    ChipLogDetail(Zcl, "Service Area: HandleSkipCurrentArea");

    // On receipt of this command the device SHALL respond with a SkipCurrentAreaResponse command.
    auto exitResponse = [ctx](SkipAreaStatus status, CharSpan statusText) {
        Commands::SkipAreaResponse::Type response{
            .status     = status,
            .statusText = Optional(statusText),
        };
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    };

    // If the SelectedAreas attribute is null, the response status should be set to InvalidLocationList.
    // If the Status field is set to InvalidLocationList, the StatusText field SHALL be an empty string.
    if (mDelegate->GetNumberOfSelectedAreas() == 0)
    {
        ChipLogError(Zcl, "Selected Locations attribute is null");
        exitResponse(SkipAreaStatus::kInvalidAreaList, ""_span);
        return;
    }

    // If the CurrentArea attribute is null, the status should be set to InvalidInMode.
    // If the Status field is not set to Success, or InvalidLocationList, the StatusText field SHALL include a vendor defined error
    // description.
    if (mCurrentArea.IsNull())
    {
        exitResponse(SkipAreaStatus::kInvalidInMode, "Current Location attribute is null"_span);
        return;
    }

    // have the device attempt to skip
    // If the Status field is not set to Success, or InvalidLocationList, the StatusText field SHALL include a vendor defined error
    // description. InvalidInMode | The received request cannot be handled due to the current mode of the device. (skipStatusText to
    // be filled out by delegated function on failure.)
    char skipStatusBuffer[kMaxSizeStatusText];
    MutableCharSpan skipStatusText(skipStatusBuffer);

    if (!mDelegate->HandleSkipCurrentArea(skipStatusText))
    {
        exitResponse(SkipAreaStatus::kInvalidInMode, skipStatusText);
        return;
    }
}

//*************************************************************************
// attribute notifications

void Instance::NotifySupportedAreasChanged()
{
    MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::SupportedAreas::Id);
}

void Instance::NotifySupportedMapsChanged()
{
    MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::SupportedMaps::Id);
}

void Instance::NotifySelectedAreasChanged()
{
    MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::SelectedAreas::Id);
}

void Instance::NotifyCurrentAreaChanged()
{
    MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::CurrentArea::Id);
}

void Instance::NotifyEstimatedEndTimeChanged()
{
    MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::EstimatedEndTime::Id);
}

void Instance::NotifyProgressChanged()
{
    MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::Progress::Id);
}

// ****************************************************************************
//  Supported Locations manipulators

bool Instance::IsSupportedLocation(uint32_t aAreaId)
{
    uint32_t ignoredIndex;
    AreaStructureWrapper ignoredLocation;

    return mDelegate->GetSupportedLocationById(aAreaId, ignoredIndex, ignoredLocation);
}

bool Instance::IsValidSupportedLocation(const AreaStructureWrapper & aLocation)
{
    // If the HomeLocationInfo field is null, the LandmarkTag field SHALL NOT be null.
    // If the LandmarkTag field is null, the HomeLocationInfo field SHALL NOT be null.
    if (aLocation.areaDesc.locationInfo.IsNull() && aLocation.areaDesc.landmarkTag.IsNull())
    {
        ChipLogDetail(Zcl, "IsValidAsSupportedLocation %u - must have locationInfo and/or LandmarkTag", aLocation.areaID);
        return false;
    }

    // If HomeLocationInfo is not null, and its LocationName field is an empty string, at least one of the following SHALL NOT
    // be null: HomeLocationInfo's FloorNumber field, HomeLocationInfo's AreaType field, the LandmarkTag field
    if (!aLocation.areaDesc.locationInfo.IsNull())
    {
        if (aLocation.areaDesc.locationInfo.Value().locationName.empty() &&
            aLocation.areaDesc.locationInfo.Value().floorNumber.IsNull() &&
            aLocation.areaDesc.locationInfo.Value().areaType.IsNull() && aLocation.areaDesc.landmarkTag.IsNull())
        {
            ChipLogDetail(
                Zcl, "IsValidAsSupportedLocation %u - LocationName is empty string, FloorNumber, AreaType, LandmarkTag are null",
                aLocation.areaID);
            return false;
        }
    }

    // If the LandmarkTag field is null, the PositionTag field SHALL be null.
    if (aLocation.areaDesc.landmarkTag.IsNull() && !aLocation.areaDesc.positionTag.IsNull())
    {
        ChipLogDetail(Zcl, "IsValidAsSupportedLocation %u - PositionTag with no LandmarkTag", aLocation.areaID);
        return false;
    }

    if (mDelegate->GetNumberOfSupportedMaps() == 0)
    {
        // If the SupportedMaps attribute is null, mapid SHALL be null.
        if (!aLocation.mapID.IsNull())
        {
            ChipLogDetail(Zcl, "IsValidSupportedLocation %u - map Id %u is not in empty supported map list", aLocation.areaID,
                          aLocation.mapID.Value());
            return false;
        }
    }
    else
    {
        // If the SupportedMaps attribute is not null, mapID SHALL be the ID of an entry from the SupportedMaps attribute.
        if (!IsSupportedMap(aLocation.mapID.Value()))
        {
            ChipLogError(Zcl, "IsValidSupportedLocation %u - map Id %u is not in supported map list", aLocation.areaID,
                         aLocation.mapID.Value());
            return false;
        }
    }

    return true;
}

bool Instance::IsUniqueSupportedLocation(const AreaStructureWrapper & aLocation, bool ignoreAreaId)
{
    BitMask<AreaStructureWrapper::IsEqualConfig> config;

    if (ignoreAreaId)
    {
        config.Set(AreaStructureWrapper::IsEqualConfig::kIgnoreAreaID);
    }

    // If the SupportedMaps attribute is not null, each entry in this list SHALL have a unique value for the combination of the
    // MapID and LocationInfo fields. If the SupportedMaps attribute is null, each entry in this list SHALL have a unique value for
    // the LocationInfo field.
    if (mDelegate->GetNumberOfSupportedMaps() == 0)
    {
        config.Set(AreaStructureWrapper::IsEqualConfig::kIgnoreMapId);
    }

    uint8_t locationIndex = 0;
    AreaStructureWrapper entry;
    while (mDelegate->GetSupportedLocationByIndex(locationIndex++, entry))
    {
        if (aLocation.IsEqual(entry, config))
        {
            return false;
        }
    }

    return true;
}

bool Instance::ReportEstimatedEndTimeChange(const DataModel::Nullable<uint32_t> & aEstimatedEndTime)
{
    if (mEstimatedEndTime == aEstimatedEndTime)
    {
        return false;
    }

    // The value of this attribute SHALL only be reported in the following cases:
    // - when it changes from null.
    if (mEstimatedEndTime.IsNull())
    {
        return true;
    }

    // - when it changes from 0
    if (mEstimatedEndTime.Value() == 0)
    {
        return true;
    }

    if (aEstimatedEndTime.IsNull())
    {
        return false;
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

bool Instance::AddSupportedLocation(uint32_t aAreaId, const DataModel::Nullable<uint8_t> & aMapId, const CharSpan & aLocationName,
                                    const DataModel::Nullable<int16_t> & aFloorNumber,
                                    const DataModel::Nullable<Globals::AreaTypeTag> & aAreaType,
                                    const DataModel::Nullable<Globals::LandmarkTag> & aLandmarkTag,
                                    const DataModel::Nullable<Globals::PositionTag> & aPositionTag,
                                    const DataModel::Nullable<Globals::FloorSurfaceTag> & aSurfaceTag)
{
    // Create location object for validation.
    AreaStructureWrapper aNewArea(aAreaId, aMapId, aLocationName, aFloorNumber, aAreaType, aLandmarkTag, aPositionTag, aSurfaceTag);

    // Does device mode allow this attribute to be updated?
    if (!mDelegate->IsSupportedAreasChangeAllowed())
    {
        return false;
    }

    // Check there is space for the entry.
    if (mDelegate->GetNumberOfSupportedAreas() >= kMaxNumSupportedAreas)
    {
        ChipLogError(Zcl, "AddSupportedLocation %u - too many entries", aAreaId);
        return false;
    }

    // Verify cluster requirements concerning valid fields and field relationships.
    if (!IsValidSupportedLocation(aNewArea))
    {
        ChipLogError(Zcl, "AddSupportedLocation %u - not a valid location object", aNewArea.areaID);
        return false;
    }

    // Each entry in Supported Locations SHALL have a unique value for the ID field.
    // If the SupportedMaps attribute is not null, each entry in this list SHALL have a unique value for the combination of the
    // MapID and LocationInfo fields. If the SupportedMaps attribute is null, each entry in this list SHALL have a unique value for
    // the LocationInfo field.
    if (!IsUniqueSupportedLocation(aNewArea, false))
    {
        ChipLogError(Zcl, "AddSupportedLocation %u - not a unique location object", aNewArea.areaID);
        return false;
    }

    // Add the SupportedLocation to the SupportedAreas attribute.
    uint32_t ignoredIndex;
    if (!mDelegate->AddSupportedLocation(aNewArea, ignoredIndex))
    {
        return false;
    }

    NotifySupportedAreasChanged();
    return true;
}

bool Instance::ModifySupportedLocation(uint32_t aAreaId, const DataModel::Nullable<uint8_t> & aMapId,
                                       const CharSpan & aLocationName, const DataModel::Nullable<int16_t> & aFloorNumber,
                                       const DataModel::Nullable<Globals::AreaTypeTag> & aAreaType,
                                       const DataModel::Nullable<Globals::LandmarkTag> & aLandmarkTag,
                                       const DataModel::Nullable<Globals::PositionTag> & aPositionTag,
                                       const DataModel::Nullable<Globals::FloorSurfaceTag> & aSurfaceTag)
{
    bool mapIDChanged = false;
    uint32_t listIndex;

    // get existing supported location to modify
    AreaStructureWrapper supportedLocation;
    if (!mDelegate->GetSupportedLocationById(aAreaId, listIndex, supportedLocation))
    {
        ChipLogError(Zcl, "ModifySupportedLocation %u - not a supported areaID", aAreaId);
        return false;
    }

    {
        // check for mapID change
        if ((aMapId.IsNull() != supportedLocation.mapID.IsNull()) ||
            (!aMapId.IsNull() && !supportedLocation.mapID.IsNull() && (aMapId.Value() != supportedLocation.mapID.Value())))
        {
            // does device mode allow this attribute to be updated?
            if (!mDelegate->IsSupportedAreasChangeAllowed())
            {
                return false;
            }
            mapIDChanged = true;
        }

        // create new location object for validation
        AreaStructureWrapper aNewArea(aAreaId, aMapId, aLocationName, aFloorNumber, aAreaType, aLandmarkTag, aPositionTag,
                                      aSurfaceTag);

        // verify cluster requirements concerning valid fields and field relationships
        if (!IsValidSupportedLocation(aNewArea))
        {
            ChipLogError(Zcl, "ModifySupportedLocation %u - not a valid location object", aNewArea.areaID);
            return false;
        }

        // Updated location description must not match another existing location description.
        // We ignore comparing the area ID as one of the locations will match this one.
        if (!IsUniqueSupportedLocation(aNewArea, true))
        {
            ChipLogError(Zcl, "ModifySupportedLocation %u - not a unique location object", aNewArea.areaID);
            return false;
        }

        // Replace the supported location with the modified location.
        if (!mDelegate->ModifySupportedLocation(listIndex, aNewArea))
        {
            return false;
        }
    }

    if (mapIDChanged)
    {
        mDelegate->HandleSupportedAreasUpdated();
    }

    NotifySupportedAreasChanged();
    return true;
}

bool Instance::ClearSupportedAreas()
{
    // does device mode allow this attribute to be updated?
    if (!mDelegate->IsSupportedAreasChangeAllowed())
    {
        return false;
    }

    if (mDelegate->ClearSupportedAreas())
    {
        mDelegate->HandleSupportedAreasUpdated();
        NotifySupportedAreasChanged();
        return true;
    }

    return false;
}

//*************************************************************************
// Supported Maps manipulators

bool Instance::IsSupportedMap(uint8_t aMapId)
{
    uint32_t ignoredIndex;
    MapStructureWrapper ignoredMap;

    return mDelegate->GetSupportedMapById(aMapId, ignoredIndex, ignoredMap);
}

bool Instance::AddSupportedMap(uint8_t aMapId, const CharSpan & aMapName)
{
    // check max# of list entries
    if (mDelegate->GetNumberOfSupportedMaps() >= kMaxNumSupportedMaps)
    {
        ChipLogError(Zcl, "AddSupportedMap %u - maximum number of entries", aMapId);
        return false;
    }

    //  Map name SHALL include readable text that describes the map name (cannot be empty string).
    if (aMapName.empty())
    {
        ChipLogError(Zcl, "AddSupportedMap %u - Name must not be empty string", aMapId);
        return false;
    }

    // Each entry in this list SHALL have a unique value for the Name field.
    uint8_t mapIndex = 0;
    MapStructureWrapper entry;

    while (mDelegate->GetSupportedMapByIndex(mapIndex++, entry))
    {
        // the name cannot be the same as an existing map
        if (entry.IsNameEqual(aMapName))
        {
            ChipLogError(Zcl, "AddSupportedMap %u - A map already exists with same name '%.*s'", aMapId,
                         static_cast<int>(entry.GetName().size()), entry.GetName().data());
            return false;
        }

        //  Each entry in this list SHALL have a unique value for the MapID field.
        if (aMapId == entry.mapID)
        {
            ChipLogError(Zcl, "AddSupportedMap - non-unique Id %u", aMapId);
            return false;
        }
    }

    {
        // add to supported maps attribute
        MapStructureWrapper newMap(aMapId, aMapName);
        uint32_t ignoredIndex;
        if (!mDelegate->AddSupportedMap(newMap, ignoredIndex))
        {
            return false;
        }
    }

    // map successfully added
    NotifySupportedMapsChanged();
    return true;
}

bool Instance::RenameSupportedMap(uint8_t aMapId, const CharSpan & newMapName)
{
    uint32_t modifiedIndex;
    MapStructureWrapper modifiedMap;

    // get existing entry
    if (!mDelegate->GetSupportedMapById(aMapId, modifiedIndex, modifiedMap))
    {
        ChipLogError(Zcl, "RenameSupportedMap Id %u - map does not exist", aMapId);
        return false;
    }

    //  Map name SHALL include readable text that describes the map's name. It cannot be empty string.
    if (newMapName.empty())
    {
        ChipLogError(Zcl, "RenameSupportedMap %u - Name must not be empty string", aMapId);
        return false;
    }

    // update the local copy of the map
    modifiedMap.Set(modifiedMap.mapID, newMapName);

    // Each entry in this list SHALL have a unique value for the Name field.
    uint32_t loopIndex = 0;
    MapStructureWrapper entry;

    while (mDelegate->GetSupportedMapByIndex(loopIndex, entry))
    {
        if (modifiedIndex == loopIndex)
        {
            continue; // don't check local modified map against its own list entry
        }

        if (entry.IsNameEqual(newMapName))
        {
            ChipLogError(Zcl, "RenameSupportedMap %u - map already exists with same name '%.*s'", aMapId,
                         static_cast<int>(entry.GetName().size()), entry.GetName().data());
            return false;
        }

        ++loopIndex;
    }

    if (!mDelegate->ModifySupportedMap(modifiedIndex, modifiedMap))
    {
        return false;
    }

    // map successfully renamed
    NotifySupportedMapsChanged();
    return true;
}

bool Instance::ClearSupportedMaps()
{
    // does device mode allow this attribute to be updated?
    if (!mDelegate->IsSupportedMapChangeAllowed())
    {
        return false;
    }

    if (mDelegate->ClearSupportedMaps())
    {
        ClearSupportedAreas();
        NotifySupportedMapsChanged();
        return true;
    }

    return false;
}

//*************************************************************************
// Selected Locations manipulators

bool Instance::AddSelectedLocation(uint32_t & aSelectedLocation)
{
    // check max# of list entries
    if (mDelegate->GetNumberOfSelectedAreas() >= kMaxNumSelectedAreas)
    {
        ChipLogError(Zcl, "AddSelectedLocation %u - maximum number of entries", aSelectedLocation);
        return false;
    }

    // each item in this list SHALL match the AreaID field of an entry on the SupportedAreas attribute's list
    if (!IsSupportedLocation(aSelectedLocation))
    {
        ChipLogError(Zcl, "AddSelectedLocation %u - not a supported location", aSelectedLocation);
        return false;
    }

    // each entry in this list SHALL have a unique value
    if (mDelegate->IsSelectedLocation(aSelectedLocation))
    {
        ChipLogError(Zcl, "AddSelectedLocation %u - duplicated location", aSelectedLocation);
        return false;
    }

    // Does device mode allow modification of selected locations?
    char locationStatusBuffer[kMaxSizeStatusText];
    MutableCharSpan locationStatusText(locationStatusBuffer);

    if (!mDelegate->IsSetSelectedAreasAllowed(locationStatusText))
    {
        ChipLogError(Zcl, "AddSelectedLocation %u - %.*s", aSelectedLocation, static_cast<int>(locationStatusText.size()),
                     locationStatusText.data());
        return false;
    }

    uint32_t ignoredIndex;
    return mDelegate->AddSelectedLocation(aSelectedLocation, ignoredIndex);
}

bool Instance::ClearSelectedAreas()
{
    if (mDelegate->ClearSelectedAreas())
    {
        NotifySelectedAreasChanged();
        return true;
    }

    return false;
}

//*************************************************************************
// Current Location manipulators

DataModel::Nullable<uint32_t> Instance::GetCurrentArea()
{
    return mCurrentArea;
}

bool Instance::SetCurrentArea(const DataModel::Nullable<uint32_t> & aCurrentArea)
{
    // If not null, the value of this attribute SHALL match the AreaID field of an entry on the SupportedAreas attribute's
    // list.
    if ((!aCurrentArea.IsNull()) && (!IsSupportedLocation(aCurrentArea.Value())))
    {
        ChipLogError(Zcl, "SetCurrentArea %u - location is not supported", aCurrentArea.Value());
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

DataModel::Nullable<uint32_t> Instance::GetEstimatedEndTime()
{
    return mEstimatedEndTime;
}

bool Instance::SetEstimatedEndTime(const DataModel::Nullable<uint32_t> & aEstimatedEndTime)
{
    // EstimatedEndTime SHALL be null if the CurrentArea attribute is null.
    if (mCurrentArea.IsNull() && !aEstimatedEndTime.IsNull())
    {
        ChipLogError(Zcl, "SetEstimatedEndTime - must be null if Current Location is null");
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

bool Instance::AddPendingProgressElement(uint32_t aAreaId)
{
    // create progress element
    Structs::ProgressStruct::Type inactiveProgress = { aAreaId, OperationalStatusEnum::kPending };

    // check max# of list entries
    if (mDelegate->GetNumberOfProgressElements() >= kMaxNumProgressElements)
    {
        ChipLogError(Zcl, "AddPendingProgressElement - maximum number of entries");
        return false;
    }

    // For each entry in this list, the AreaID field SHALL match an entry on the SupportedAreas attribute's list.
    if (!IsSupportedLocation(aAreaId))
    {
        ChipLogError(Zcl, "AddPendingProgressElement - not a supported location %u", aAreaId);
        return false;
    }

    // Each entry in this list SHALL have a unique value for the AreaID field.
    if (mDelegate->IsProgressElement(aAreaId))
    {
        ChipLogError(Zcl, "AddPendingProgressElement - progress element already exists for location %u", aAreaId);
        return false;
    }

    uint32_t ignoredIndex;

    if (!mDelegate->AddProgressElement(inactiveProgress, ignoredIndex))
    {
        return false;
    }

    NotifyProgressChanged();
    return true;
}

bool Instance::SetProgressStatus(uint32_t aAreaId, OperationalStatusEnum opStatus)
{
    uint32_t listIndex;
    Structs::ProgressStruct::Type progressElement;

    if (!mDelegate->GetProgressElementById(aAreaId, listIndex, progressElement))
    {
        ChipLogError(Zcl, "SetProgressStatus - progress element does not exist for location %u", aAreaId);
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
        progressElement.totalOperationalTime.Value().SetNull();
    }

    // add the updated element to the progress attribute
    if (!mDelegate->ModifyProgressElement(listIndex, progressElement))
    {
        return false;
    }

    NotifyProgressChanged();
    return true;
}

bool Instance::SetProgressTotalOperationalTime(uint32_t aAreaId, const DataModel::Nullable<uint32_t> & aTotalOperationalTime)
{
    uint32_t listIndex;
    Structs::ProgressStruct::Type progressElement;

    if (!mDelegate->GetProgressElementById(aAreaId, listIndex, progressElement))
    {
        ChipLogError(Zcl, "SetProgressTotalOperationalTime - progress element does not exist for location %u", aAreaId);
        return false;
    }

    // If the time value is not changing, there is no need to modify the existing element.
    if (progressElement.totalOperationalTime == aTotalOperationalTime)
    {
        return true;
    }

    // This attribute SHALL be null if the Status field is not set to Completed or Skipped
    if (((progressElement.status == OperationalStatusEnum::kCompleted) ||
         (progressElement.status == OperationalStatusEnum::kSkipped)) &&
        !aTotalOperationalTime.IsNull())
    {
        ChipLogError(Zcl,
                     "SetProgressTotalOperationalTime - location %u opStatus value %u - can be non-null only if opStatus is "
                     "Completed or Skipped",
                     aAreaId, to_underlying(progressElement.status));
        return false;
    }

    // set the time in the local copy
    progressElement.totalOperationalTime.Emplace(aTotalOperationalTime);

    // add the updated element to the progress attribute
    if (!mDelegate->ModifyProgressElement(listIndex, progressElement))
    {
        return false;
    }

    NotifyProgressChanged();
    return true;
}

bool Instance::SetProgressEstimatedTime(uint32_t aAreaId, const DataModel::Nullable<uint32_t> & aEstimatedTime)
{
    uint32_t listIndex;
    Structs::ProgressStruct::Type progressElement;

    if (!mDelegate->GetProgressElementById(aAreaId, listIndex, progressElement))
    {
        ChipLogError(Zcl, "SetProgressEstimatedTime - progress element does not exist for location %u", aAreaId);
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
    if (!mDelegate->ModifyProgressElement(listIndex, progressElement))
    {
        return false;
    }

    NotifyProgressChanged();
    return true;
}

bool Instance::ClearProgress()
{
    if (mDelegate->ClearProgress())
    {
        NotifyProgressChanged();
        return true;
    }

    return false;
}

// attribute manipulators - Feature Map

bool Instance::HasFeature(Feature feature) const
{
    return mFeature.Has(feature);
}

} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip

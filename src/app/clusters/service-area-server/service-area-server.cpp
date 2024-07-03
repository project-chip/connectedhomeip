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
    InteractionModelEngine::GetInstance()->UnregisterCommandHandler(this);
    unregisterAttributeAccessOverride(this);
}

CHIP_ERROR Instance::Init()
{
    ChipLogProgress(Zcl, "Service Area: INIT");

    // Check if the cluster has been selected in zap
    VerifyOrReturnError(emberAfContainsServer(mEndpointId, Id), CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(Zcl, "Service Area: The cluster with Id %lu was not enabled in zap.", long(Id)));

    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));

    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);

    return mDelegate->Init();
}

//*************************************************************************
// core functions

CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    ChipLogDetail(Zcl, "Service Area: Reading attribute");

    switch (aPath.mAttributeId)
    {

    case Attributes::SupportedLocations::Id:
        return ReadSupportedLocations(aEncoder);

    case Attributes::SupportedMaps::Id:
        return ReadSupportedMaps(aEncoder);

    case Attributes::SelectedLocations::Id:
        return ReadSelectedLocations(aEncoder);

    case Attributes::CurrentLocation::Id:
        return aEncoder.Encode(GetCurrentLocation());

    case Attributes::EstimatedEndTime::Id:
        return aEncoder.Encode(GetEstimatedEndTime());

    case Attributes::Progress::Id:
        return ReadProgress(aEncoder);

    case Attributes::FeatureMap::Id:
        return aEncoder.Encode(mFeature.Raw());

    default:
        ChipLogProgress(Zcl, "Service Area: Read unsupported attribute %u", aPath.mAttributeId);
    }

    return CHIP_NO_ERROR;
}

void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    ChipLogProgress(Zcl, "%s", __func__);
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::SelectLocations::Id:
        return CommandHandlerInterface::HandleCommand<Commands::SelectLocations::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleSelectLocationsCmd(ctx, req); });

    case Commands::SkipCurrentLocation::Id:
        return CommandHandlerInterface::HandleCommand<Commands::SkipCurrentLocation::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleSkipCurrentLocationCmd(ctx); });
    }
}

//*************************************************************************
// attribute readers

CHIP_ERROR Instance::ReadSupportedLocations(AttributeValueEncoder & aEncoder)
{
    if (mDelegate->GetNumberOfSupportedLocations() == 0)
    {
        return aEncoder.EncodeNull();
    }

    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        uint8_t locationIndex = 0;
        LocationStructureWrapper supportedLocation;

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
        return aEncoder.EncodeNull();
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

CHIP_ERROR Instance::ReadSelectedLocations(AttributeValueEncoder & aEncoder)
{
    if (mDelegate->GetNumberOfSelectedLocations() == 0)
    {
        return aEncoder.EncodeNull();
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
        return aEncoder.EncodeNull();
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

void Instance::HandleSelectLocationsCmd(HandlerContext & ctx, const Commands::SelectLocations::DecodableType & req)
{
    ChipLogDetail(Zcl, "Service Area: HandleSelectLocationsCmd");

    uint32_t listIndex = 0;
    uint32_t oldSelectedLocation;
    size_t numberOfLocations = 0;
    bool matchesCurrentSelectedLocations;

    // On receipt of this command the device SHALL respond with a SelectLocationsResponse command.
    auto exitResponse = [ctx](SelectLocationsStatus status, CharSpan statusText) {
        Commands::SelectLocationsResponse::Type response{
            .status     = status,
            .statusText = Optional(statusText),
        };
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    };

    // Get the number of Selected Locations in the command parameter and check that it is valid.
    if (!req.newLocations.IsNull())
    {
        if (CHIP_NO_ERROR != req.newLocations.Value().ComputeSize(&numberOfLocations))
        {
            exitResponse(SelectLocationsStatus::kInvalidSet, "decoding of newLocations parameter size failed"_span);
            return;
        }

        // If the device determines that it can't operate at all locations from the list,
        // the SelectLocationsResponse command's Status field SHALL indicate InvalidSet.
        if (((numberOfLocations == 0) || (numberOfLocations > kMaxNumSelectedLocations)))
        {
            exitResponse(SelectLocationsStatus::kInvalidSet, "invalid number of locations"_span);
            return;
        }
    }

    // if number of selected locations in parameter matches number in attribute - the locations *might* be the same
    matchesCurrentSelectedLocations = (numberOfLocations == mDelegate->GetNumberOfSelectedLocations());

    if (!req.newLocations.IsNull())
    {
        // do as much parameter validation as we can
        {
            auto locationIter = req.newLocations.Value().begin();
            while (locationIter.Next())
            {
                uint32_t aSelectedLocation = locationIter.GetValue();

                // each item in this list SHALL match the LocationID field of an entry on the SupportedLocations attribute's list
                // If the Status field is set to UnsupportedLocation, the StatusText field SHALL be an empty string.
                if (!IsSupportedLocation(aSelectedLocation))
                {
                    exitResponse(SelectLocationsStatus::kUnsupportedLocation, ""_span);
                    return;
                }

                // todo Should we take the responsibility for checking that there are no duplicate locations
                //  from the delegate's IsValidSelectLocationsSet method?

                // check to see if parameter list and attribute still match
                if (matchesCurrentSelectedLocations)
                {
                    if (!mDelegate->GetSelectedLocationByIndex(listIndex, oldSelectedLocation) ||
                        (aSelectedLocation != oldSelectedLocation))
                    {
                        matchesCurrentSelectedLocations = false;
                    }
                }
            }

            // after iterating with Next through DecodableType - check for failure
            if (CHIP_NO_ERROR != locationIter.GetStatus())
            {
                exitResponse(SelectLocationsStatus::kInvalidSet, "decoding of newLocations parameter failed"_span);
                return;
            }
        }
    }

    // If the NewLocations field is the same as the value of the SelectedLocations attribute
    // the SelectLocationsResponse command SHALL have the Status field set to Success and
    // the StatusText field MAY be supplied with a human-readable string or include an empty string.
    if (matchesCurrentSelectedLocations)
    {
        exitResponse(SelectLocationsStatus::kSuccess, ""_span);
        return;
    }

    char delegateStatusBuffer[kMaxSizeStatusText];
    MutableCharSpan delegateStatusText(delegateStatusBuffer);

    // If the current state of the device doesn't allow for the locations to be selected,
    // the SelectLocationsResponse command SHALL have the Status field set to InvalidInMode.
    // if the Status field is set to InvalidInMode, the StatusText field SHOULD indicate why the request is not allowed,
    // given the current mode of the device, which may involve other clusters.
    // (note - locationStatusText to be filled out by delegated function for if return value is false)
    if (!mDelegate->IsSetSelectedLocationsAllowed(delegateStatusText))
    {
        exitResponse(SelectLocationsStatus::kInvalidInMode, delegateStatusText);
        return;
    }

    // Set to empty string in case the delegate accidentally modified this string.
    chip::CopyCharSpanToMutableCharSpan(""_span, delegateStatusText);

    // ask the device to handle SelectLocations Command
    // (note - locationStatusText to be filled out by delegated function for kInvalidInMode and InvalidSet)
    auto locationStatus = SelectLocationsStatus::kSuccess;
    if (!mDelegate->IsValidSelectLocationsSet(req, locationStatus, delegateStatusText))
    {
        exitResponse(locationStatus, delegateStatusText);
        return;
    }

    {
        // If the device successfully accepts the request, the server will attempt to operate at the location(s)
        // indicated by the entries of the NewLocation field, when requested to operate,
        // the SelectLocationsResponse command SHALL have the Status field set to Success,
        // and the SelectedLocations attribute SHALL be set to the value of the NewLocations field.
        mDelegate->ClearSelectedLocations();

        if (!req.newLocations.IsNull())
        {
            auto locationIter = req.newLocations.Value().begin();
            uint32_t dummyIter;
            while (locationIter.Next())
            {
                mDelegate->AddSelectedLocation(locationIter.GetValue(), dummyIter);
            }
        }

        NotifySelectedLocationsChanged();
    }

    exitResponse(SelectLocationsStatus::kSuccess, ""_span);
}

void Instance::HandleSkipCurrentLocationCmd(HandlerContext & ctx)
{
    ChipLogDetail(Zcl, "Service Area: HandleSkipCurrentLocation");

    // On receipt of this command the device SHALL respond with a SkipCurrentLocationResponse command.
    auto exitResponse = [ctx](SkipCurrentLocationStatus status, CharSpan statusText) {
        Commands::SkipCurrentLocationResponse::Type response{
            .status     = status,
            .statusText = Optional(statusText),
        };
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    };

    // If the SelectedLocations attribute is null, the response status should be set to InvalidLocationList.
    // If the Status field is set to InvalidLocationList, the StatusText field SHALL be an empty string.
    if (mDelegate->GetNumberOfSelectedLocations() == 0)
    {
        ChipLogError(Zcl, "Selected Locations attribute is null");
        exitResponse(SkipCurrentLocationStatus::kInvalidLocationList, ""_span);
        return;
    }

    // If the CurrentLocation attribute is null, the status should be set to InvalidInMode.
    // If the Status field is not set to Success, or InvalidLocationList, the StatusText field SHALL include a vendor defined error
    // description.
    if (mCurrentLocation.IsNull())
    {
        exitResponse(SkipCurrentLocationStatus::kInvalidInMode, "Current Location attribute is null"_span);
        return;
    }

    // have the device attempt to skip
    // If the Status field is not set to Success, or InvalidLocationList, the StatusText field SHALL include a vendor defined error
    // description. InvalidInMode | The received request cannot be handled due to the current mode of the device. (skipStatusText to
    // be filled out by delegated function on failure.)
    char skipStatusBuffer[kMaxSizeStatusText];
    MutableCharSpan skipStatusText(skipStatusBuffer);

    if (!mDelegate->HandleSkipCurrentLocation(skipStatusText))
    {
        exitResponse(SkipCurrentLocationStatus::kInvalidInMode, skipStatusText);
        return;
    }
}

//*************************************************************************
// attribute notifications

void Instance::NotifySupportedLocationsChanged()
{
    MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::SupportedLocations::Id);
}

void Instance::NotifySupportedMapsChanged()
{
    MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::SupportedMaps::Id);
}

void Instance::NotifySelectedLocationsChanged()
{
    MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::SelectedLocations::Id);
}

void Instance::NotifyCurrentLocationChanged()
{
    MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::CurrentLocation::Id);
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

bool Instance::IsSupportedLocation(uint32_t aLocationId)
{
    uint32_t dummyIndex;
    LocationStructureWrapper dummyLocation;

    return mDelegate->GetSupportedLocationById(aLocationId, dummyIndex, dummyLocation);
}

bool Instance::IsValidSupportedLocation(const LocationStructureWrapper & aLocation)
{
    // If the HomeLocationInfo field is null, the LandmarkTag field SHALL NOT be null.
    // If the LandmarkTag field is null, the HomeLocationInfo field SHALL NOT be null.
    if (aLocation.locationInfo.locationInfo.IsNull() && aLocation.locationInfo.landmarkTag.IsNull())
    {
        ChipLogDetail(Zcl, "IsValidAsSupportedLocation %u - must have locationInfo and/or LandmarkTag",
                      aLocation.locationID);
            return false;
    }

    // If HomeLocationInfo is not null, and its LocationName field is an empty string, at least one of the following SHALL NOT
    // be null: HomeLocationInfo's FloorNumber field, HomeLocationInfo's AreaType field, the LandmarkTag field
    if (!aLocation.locationInfo.locationInfo.IsNull())
    {
        if (aLocation.locationInfo.locationInfo.Value().locationName.empty() &&
            aLocation.locationInfo.locationInfo.Value().floorNumber.IsNull() &&
            aLocation.locationInfo.locationInfo.Value().areaType.IsNull() && aLocation.locationInfo.landmarkTag.IsNull())
        {
            ChipLogDetail(
                Zcl, "IsValidAsSupportedLocation %u - LocationName is empty string, FloorNumber, AreaType, LandmarkTag are null",
                aLocation.locationID);
            return false;
        }
    }

    // If the LandmarkTag field is null, the PositionTag field SHALL be null.
    if (aLocation.locationInfo.landmarkTag.IsNull() && !aLocation.locationInfo.positionTag.IsNull())
    {
        ChipLogDetail(Zcl, "IsValidAsSupportedLocation %u - PositionTag with no LandmarkTag", aLocation.locationID);
        return false;
    }

    if (mDelegate->GetNumberOfSupportedMaps() == 0)
    {
        // If the SupportedMaps attribute is null, mapid SHALL be null.
        if (!aLocation.mapID.IsNull())
        {
            ChipLogDetail(Zcl, "IsValidSupportedLocation %u - map Id %u is not in empty supported map list", aLocation.locationID,
                          aLocation.mapID.Value());
            return false;
        }
    }
    else
    {
        // If the SupportedMaps attribute is not null, mapID SHALL be the ID of an entry from the SupportedMaps attribute.
        if (!IsSupportedMap(aLocation.mapID.Value()))
        {
            ChipLogError(Zcl, "IsValidSupportedLocation %u - map Id %u is not in supported map list", aLocation.locationID,
                         aLocation.mapID.Value());
            return false;
        }
    }

    return true;
}

bool Instance::IsUniqueSupportedLocation(const LocationStructureWrapper & aLocation, bool ignoreLocationId)
{
    uint8_t locationIndex = 0;
    LocationStructureWrapper entry;
    BitMask<LocationStructureWrapper::IsEqualConfig> config;

    if (ignoreLocationId)
    {
        config.Set(LocationStructureWrapper::IsEqualConfig::kIgnoreLocationId);
    }

    // If the SupportedMaps attribute is not null, each entry in this list SHALL have a unique value for the combination of the
    // MapID and LocationInfo fields. If the SupportedMaps attribute is null, each entry in this list SHALL have a unique value for
    // the LocationInfo field.
    if (mDelegate->GetNumberOfSupportedMaps() == 0)
    {
        config.Set(LocationStructureWrapper::IsEqualConfig::kIgnoreMapId);
    }

    while (mDelegate->GetSupportedLocationByIndex(locationIndex++, entry))
    {
        if (aLocation.IsEqual(entry, config))
        {
            return false;
        }
    }

    return true;
}

bool Instance::AddSupportedLocation(uint32_t aLocationId, const DataModel::Nullable<uint8_t> & aMapId,
                                    const CharSpan & aLocationName, const DataModel::Nullable<int16_t> & aFloorNumber,
                                    const DataModel::Nullable<AreaTypeTag> & aAreaType,
                                    const DataModel::Nullable<LandmarkTag> & aLandmarkTag,
                                    const DataModel::Nullable<PositionTag> & aPositionTag,
                                    const DataModel::Nullable<FloorSurfaceTag> & aSurfaceTag)
{
    // Create location object for validation.
    LocationStructureWrapper aNewLocation(aLocationId, aMapId, aLocationName, aFloorNumber, aAreaType, aLandmarkTag, aPositionTag,
                                          aSurfaceTag);

    // Does device mode allow this attribute to be updated?
    if (!mDelegate->IsSupportedLocationsChangeAllowed())
    {
        return false;
    }

    // Check there is space for the entry.
    if (mDelegate->GetNumberOfSupportedLocations() >= kMaxNumSupportedLocations)
    {
        ChipLogError(Zcl, "AddSupportedLocation %u - too many entries", aLocationId);
        return false;
    }

    // Verify cluster requirements concerning valid fields and field relationships.
    if (!IsValidSupportedLocation(aNewLocation))
    {
        ChipLogError(Zcl, "AddSupportedLocation %u - not a valid location object", aNewLocation.locationID);
        return false;
    }

    // Each entry in Supported Locations SHALL have a unique value for the ID field.
    // If the SupportedMaps attribute is not null, each entry in this list SHALL have a unique value for the combination of the
    // MapID and LocationInfo fields. If the SupportedMaps attribute is null, each entry in this list SHALL have a unique value for
    // the LocationInfo field.
    if (!IsUniqueSupportedLocation(aNewLocation, false))
    {
        ChipLogError(Zcl, "AddSupportedLocation %u - not a unique location object", aNewLocation.locationID);
        return false;
    }

    // Add the SupportedLocation to the SupportedLocations attribute.
    uint32_t dummyIndex;
    if (!mDelegate->AddSupportedLocation(aNewLocation, dummyIndex))
    {
        return false;
    }

    NotifySupportedLocationsChanged();
    return true;
}

bool Instance::ModifySupportedLocation(uint32_t aLocationId, const DataModel::Nullable<uint8_t> & aMapId,
                                       const CharSpan & aLocationName, const DataModel::Nullable<int16_t> & aFloorNumber,
                                       const DataModel::Nullable<AreaTypeTag> & aAreaType,
                                       const DataModel::Nullable<LandmarkTag> & aLandmarkTag,
                                       const DataModel::Nullable<PositionTag> & aPositionTag,
                                       const DataModel::Nullable<FloorSurfaceTag> & aSurfaceTag)
{
    bool mapIDChanged = false;
    uint32_t listIndex;

    // get existing supported location to modify
    LocationStructureWrapper supportedLocation;
    if (!mDelegate->GetSupportedLocationById(aLocationId, listIndex, supportedLocation))
    {
        ChipLogError(Zcl, "ModifySupportedLocation %u - not a supported locationID", aLocationId);
        return false;
    }

    {
        // check for mapID change
        if ((aMapId.IsNull() != supportedLocation.mapID.IsNull()) ||
            (!aMapId.IsNull() && !supportedLocation.mapID.IsNull() && (aMapId.Value() != supportedLocation.mapID.Value())))
        {
            // does device mode allow this attribute to be updated?
            if (!mDelegate->IsSupportedLocationsChangeAllowed())
            {
                return false;
            }
            mapIDChanged = true;
        }

        // create new location object for validation
        LocationStructureWrapper aNewLocation(aLocationId, aMapId, aLocationName, aFloorNumber, aAreaType, aLandmarkTag,
                                              aPositionTag, aSurfaceTag);

        // verify cluster requirements concerning valid fields and field relationships
        if (!IsValidSupportedLocation(aNewLocation))
        {
            ChipLogError(Zcl, "ModifySupportedLocation %u - not a valid location object", aNewLocation.locationID);
            return false;
        }

        // Updated location description must not match another existing location description.
        // We ignore comparing the location ID as one of the locations will match this one.
        if (!IsUniqueSupportedLocation(aNewLocation, true))
        {
            ChipLogError(Zcl, "ModifySupportedLocation %u - not a unique location object", aNewLocation.locationID);
            return false;
        }

        // Replace the supported location with the modified location.
        if (!mDelegate->ModifySupportedLocation(listIndex, aNewLocation))
        {
            return false;
        }
    }

    if (mapIDChanged)
    {
        mDelegate->HandleSupportedLocationsUpdated();
    }

    NotifySupportedLocationsChanged();
    return true;
}

bool Instance::ClearSupportedLocations()
{
    // does device mode allow this attribute to be updated?
    if (!mDelegate->IsSupportedLocationsChangeAllowed())
    {
        return false;
    }

    if (mDelegate->ClearSupportedLocations())
    {
        mDelegate->HandleSupportedLocationsUpdated();
        NotifySupportedLocationsChanged();
        return true;
    }

    return false;
}

//*************************************************************************
// Supported Maps manipulators

bool Instance::IsSupportedMap(uint8_t aMapId)
{
    uint32_t dummyIndex;
    MapStructureWrapper dummyMap;

    return mDelegate->GetSupportedMapById(aMapId, dummyIndex, dummyMap);
}

bool Instance::AddSupportedMap(uint8_t aMapId, const CharSpan & aMapName)
{
    uint8_t mapIndex = 0;
    MapStructureWrapper entry;

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
    while (mDelegate->GetSupportedMapByIndex(mapIndex++, entry))
    {
        // the name cannot be the same as an existing map
        if (entry.IsNameEqual(aMapName))
        {
            ChipLogError(Zcl, "AddSupportedMap %u - A map already exists with same name '%s'", aMapId, entry.GetNameAsCString());
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
        uint32_t dummyIndex;
        if (!mDelegate->AddSupportedMap(newMap, dummyIndex))
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
    uint32_t loopIndex = 0;
    MapStructureWrapper modifiedMap;
    MapStructureWrapper entry;

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
    while (mDelegate->GetSupportedMapByIndex(loopIndex, entry))
    {
        if (modifiedIndex == loopIndex)
        {
            continue; // don't check local modified map against its own list entry
        }

        if (entry.IsNameEqual(newMapName))
        {
            ChipLogError(Zcl, "AddSupportedMap %u - map already exists with same name '%s'", aMapId, entry.GetNameAsCString());
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
        ClearSupportedLocations();
        NotifySupportedMapsChanged();
        return true;
    }

    return false;
}

//*************************************************************************
// Selected Locations manipulators

bool Instance::AddSelectedLocation(uint32_t & aSelectedLocation)
{
    uint32_t dummyIndex;

    char locationStatusBuffer[kMaxSizeStatusText];
    MutableCharSpan locationStatusText(locationStatusBuffer);

    // check max# of list entries
    if (mDelegate->GetNumberOfSelectedLocations() >= kMaxNumSelectedLocations)
    {
        ChipLogError(Zcl, "AddSelectedLocation %u - maximum number of entries", aSelectedLocation);
        return false;
    }

    // each item in this list SHALL match the LocationID field of an entry on the SupportedLocations attribute's list
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
    if (!mDelegate->IsSetSelectedLocationsAllowed(locationStatusText))
    {
        //        ChipLogError(Zcl, "AddSelectedLocation %u - %s", aSelectedLocation, locationStatusText);
        return false;
    }

    return mDelegate->AddSelectedLocation(aSelectedLocation, dummyIndex);
}

bool Instance::ClearSelectedLocations()
{
    if (mDelegate->ClearSelectedLocations())
    {
        NotifySelectedLocationsChanged();
        return true;
    }

    return false;
}

//*************************************************************************
// Current Location manipulators

DataModel::Nullable<uint32_t> Instance::GetCurrentLocation()
{
    return mCurrentLocation;
}

bool Instance::SetCurrentLocation(const DataModel::Nullable<uint32_t> & aCurrentLocation)
{
    // If not null, the value of this attribute SHALL match the LocationID field of an entry on the SupportedLocations attribute's
    // list.
    if ((!aCurrentLocation.IsNull()) && (!IsSupportedLocation(aCurrentLocation.Value())))
    {
        ChipLogError(Zcl, "SetCurrentLocation %u - location is not supported", aCurrentLocation.Value());
        return false;
    }

    mCurrentLocation = aCurrentLocation;
    NotifyCurrentLocationChanged();

    // EstimatedEndTime SHALL be null if the CurrentLocation attribute is null.
    if (mCurrentLocation.IsNull())
    {
        SetEstimatedEndTime(DataModel::Nullable<uint32_t>());
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
    bool doChangeNotify = false;

    // EstimatedEndTime SHALL be null if the CurrentLocation attribute is null.
    if (mCurrentLocation.IsNull() && !aEstimatedEndTime.IsNull())
    {
        ChipLogError(Zcl, "SetEstimatedEndTime - must be null if Current Location is null");
        return false;
    }

    // The value of this attribute SHALL only be reported in the following cases:
    // - when it changes from 0 to any other value and vice versa
    // - when it decreases
    // - when it changes from null to any non-zero value
    if ((!mEstimatedEndTime.IsNull() && !aEstimatedEndTime.IsNull() && (aEstimatedEndTime.Value() == 0) &&
         mEstimatedEndTime.Value() != 0) ||
        (!mEstimatedEndTime.IsNull() && !aEstimatedEndTime.IsNull() && (aEstimatedEndTime.Value() != 0) &&
         mEstimatedEndTime.Value() == 0) ||
        (!mEstimatedEndTime.IsNull() && !aEstimatedEndTime.IsNull() && (aEstimatedEndTime.Value() < mEstimatedEndTime.Value())) ||
        (mEstimatedEndTime.IsNull() && !aEstimatedEndTime.IsNull() && (aEstimatedEndTime.Value() != 0)))
    {
        doChangeNotify = true;
    }

    mEstimatedEndTime = aEstimatedEndTime;

    if (doChangeNotify)
    {
        NotifyEstimatedEndTimeChanged();
    }

    // success
    return true;
}

//*************************************************************************
// Progress list manipulators

bool Instance::AddPendingProgressElement(uint32_t aLocationId)
{
    uint32_t dummyIndex;

    // create progress element
    Structs::ProgressStruct::Type inactiveProgress = { aLocationId, OperationalStatusEnum::kPending };

    // check max# of list entries
    if (mDelegate->GetNumberOfProgressElements() >= kMaxNumProgressElements)
    {
        ChipLogError(Zcl, "AddPendingProgressElement - maximum number of entries");
        return false;
    }

    // For each entry in this list, the LocationID field SHALL match an entry on the SupportedLocations attribute's list.
    if (!IsSupportedLocation(aLocationId))
    {
        ChipLogError(Zcl, "AddPendingProgressElement - not a supported location %u", aLocationId);
        return false;
    }

    // Each entry in this list SHALL have a unique value for the LocationID field.
    if (mDelegate->IsProgressElement(aLocationId))
    {
        ChipLogError(Zcl, "AddPendingProgressElement - progress element already exists for location %u", aLocationId);
        return false;
    }

    if (!mDelegate->AddProgressElement(inactiveProgress, dummyIndex))
    {
        return false;
    }

    NotifyProgressChanged();
    return true;
}

bool Instance::SetProgressStatus(uint32_t aLocationId, OperationalStatusEnum opStatus)
{
    uint32_t listIndex;

    Structs::ProgressStruct::Type progressElement;

    if (!mDelegate->GetProgressElementById(aLocationId, listIndex, progressElement))
    {
        ChipLogError(Zcl, "SetProgressStatus - progress element does not exist for location %u", aLocationId);
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

bool Instance::SetProgressTotalOperationalTime(uint32_t aLocationId, const DataModel::Nullable<uint32_t> & aTotalOperationalTime)
{
    uint32_t listIndex;

    Structs::ProgressStruct::Type progressElement;

    if (!mDelegate->GetProgressElementById(aLocationId, listIndex, progressElement))
    {
        ChipLogError(Zcl, "SetProgressTotalOperationalTime - progress element does not exist for location %u", aLocationId);
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
                     aLocationId, to_underlying(progressElement.status));
        return false;
    }

    // set the time in the local copy
    progressElement.totalOperationalTime.Value() = aTotalOperationalTime;

    // add the updated element to the progress attribute
    if (!mDelegate->ModifyProgressElement(listIndex, progressElement))
    {
        return false;
    }

    NotifyProgressChanged();
    return true;
}

bool Instance::SetProgressEstimatedTime(uint32_t aLocationId, const DataModel::Nullable<uint32_t> & aEstimatedTime)
{
    uint32_t listIndex;

    Structs::ProgressStruct::Type progressElement;

    if (!mDelegate->GetProgressElementById(aLocationId, listIndex, progressElement))
    {
        ChipLogError(Zcl, "SetProgressEstimatedTime - progress element does not exist for location %u", aLocationId);
        return false;
    }

    // If the time value is not changing, there is no need to modify the existing element.
    if (progressElement.estimatedTime == aEstimatedTime)
    {
        return true;
    };

    // set the time in the local copy
    progressElement.estimatedTime.Value() = aEstimatedTime;

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

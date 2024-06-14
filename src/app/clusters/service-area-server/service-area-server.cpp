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


void MatterServiceAreaPluginServerInitCallback() {};

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ServiceArea;
using namespace chip::app::Clusters::ServiceArea::Commands;
using namespace chip::app::Clusters::ServiceArea::Attributes;
using Status = Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace ServiceArea {


// ****************************************************************************
// Service Area Server Instance

Instance::Instance(Delegate * aDelegate, EndpointId aEndpointId, BitMask<ServiceArea::Feature> aFeature, ClusterId aClusterId) :
    AttributeAccessInterface(MakeOptional(aEndpointId), aClusterId),
    CommandHandlerInterface(MakeOptional(aEndpointId), aClusterId),
    mDelegate(aDelegate),
    mEndpointId(aEndpointId),
    mClusterId(aClusterId),
    mFeature(aFeature)
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
    VerifyOrReturnError(
        emberAfContainsServer(mEndpointId, chip::app::Clusters::ServiceArea::Id), CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(Zcl, "Service Area: The cluster with Id %lu was not enabled in zap.", long(chip::app::Clusters::ServiceArea::Id)));

    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);

    // features are independent, any combination of supported/not-supported is valid
    VerifyOrReturnError((mFeature.Raw() && 
                        !(to_underlying(Feature::kListOrder) || to_underlying(Feature::kSelectWhileRunning))) == 0, 
                        CHIP_ERROR_INVALID_ARGUMENT);

    return CHIP_NO_ERROR;
}


//*************************************************************************
// core functions

CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    ChipLogError(Zcl, "Service Area: Reading");

    switch (aPath.mAttributeId)
    {

    case ServiceArea::Attributes::SupportedLocations::Id: {
        ChipLogProgress(Zcl, "Service Area: Read SupportedLocations");
        ReturnErrorOnFailure(ReadSupportedLocations(aEncoder));
        break;
    }

    case ServiceArea::Attributes::SupportedMaps::Id: {
        ChipLogProgress(Zcl, "Service Area: Read SupportedMaps");
        ReturnErrorOnFailure(ReadSupportedMaps(aEncoder));
        break;
    } 

    case ServiceArea::Attributes::SelectedLocations::Id: {
        ChipLogProgress(Zcl, "Service Area: Read SelectedLocations");
        ReturnErrorOnFailure(ReadSelectedLocations(aEncoder));
        break;
    } 

    case ServiceArea::Attributes::CurrentLocation::Id: {
        ChipLogProgress(Zcl, "Service Area: Read CurrentLocation");
        ReturnErrorOnFailure(aEncoder.Encode(GetCurrentLocation()));
        break;     
    }

    case ServiceArea::Attributes::EstimatedEndTime::Id: {
        ChipLogProgress(Zcl, "Service Area: Read EstimatedEndTime");
        ReturnErrorOnFailure(aEncoder.Encode(GetEstimatedEndTime()));
        break;
    }

    case ServiceArea::Attributes::Progress::Id: {
        ChipLogProgress(Zcl, "Service Area: Read Progress");
        ReturnErrorOnFailure(ReadProgress(aEncoder));
        break;
    }

    case ServiceArea::Attributes::FeatureMap::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mFeature.Raw()));
        break;
    }
  
    default:
        ChipLogProgress(Zcl, "Service Area: Read unsupported attribute %u", aPath.mAttributeId);
        break;
    }

    return CHIP_NO_ERROR;
}

void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    ChipLogProgress(Zcl, "%s", __func__); 
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::SelectLocations::Id:

        CommandHandlerInterface::HandleCommand<Commands::SelectLocations::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleSelectLocationsCmd(ctx, req); });
        break;

    case Commands::SkipCurrentLocation::Id:

        CommandHandlerInterface::HandleCommand<Commands::SkipCurrentLocation::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleSkipCurrentLocationCmd(ctx); });
        break;

    default:
        break;
    }

    return;
}


//*************************************************************************
// attribute readers

CHIP_ERROR Instance::ReadSupportedLocations(chip::app::AttributeValueEncoder & aEncoder)
{
    if (mDelegate->GetNumberOfSupportedLocations() == 0)
    {
        return aEncoder.EncodeNull();
    }
    else
    {
        return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR 
        {
            uint8_t                   locationIndex = 0;
            LocationStructureWrapper supportedLocation;

            while (mDelegate->GetSupportedLocationByIndex(locationIndex++, supportedLocation))
            {
                ReturnErrorOnFailure(encoder.Encode(supportedLocation));
            }
            return CHIP_NO_ERROR; 
        });
    }
}

CHIP_ERROR Instance::ReadSupportedMaps(chip::app::AttributeValueEncoder & aEncoder) 
{
    if (mDelegate->GetNumberOfSupportedMaps() == 0)
    {
        return aEncoder.EncodeNull();
    }
    else
    {
        return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR 
        {
            uint32_t              mapIndex = 0;
            MapStructureWrapper supportedMap;

            while (mDelegate->GetSupportedMapByIndex(mapIndex++, supportedMap))
            {
                ReturnErrorOnFailure(encoder.Encode(supportedMap));
            }
            return CHIP_NO_ERROR; 
        });
    }
}

CHIP_ERROR Instance::ReadSelectedLocations(chip::app::AttributeValueEncoder & aEncoder) 
{
    if (mDelegate->GetNumberOfSelectedLocations() == 0)
    {
        return aEncoder.EncodeNull();
    }
    else
    {
        return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR 
        {
            uint32_t    locationIndex = 0;
            uint32_t    selectedLocation;

            while (mDelegate->GetSelectedLocationByIndex(locationIndex++, selectedLocation))
            {
                ReturnErrorOnFailure(encoder.Encode(selectedLocation));
            }
            return CHIP_NO_ERROR; 
        });
    }
}

CHIP_ERROR Instance::ReadProgress(chip::app::AttributeValueEncoder & aEncoder) 
{
    if (mDelegate->GetNumberOfProgressElements() == 0)
    {
        return aEncoder.EncodeNull();
    }
    else
    {
        return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR 
        {
            uint32_t                       locationIndex = 0;
            Structs::ProgressStruct::Type  progressElement;

            while (mDelegate->GetProgressElementByIndex(locationIndex++, progressElement))
            {
                ReturnErrorOnFailure(encoder.Encode(progressElement));
            }
            return CHIP_NO_ERROR; 
        });
    }
}


//*************************************************************************
// command handlers

void Instance::HandleSelectLocationsCmd(HandlerContext & ctx, const Commands::SelectLocations::DecodableType & req)
{
    ChipLogDetail(Zcl, "HandleSelectLocationsCmd");

    SelectLocationsStatus locationStatus            = SelectLocationsStatus::kSuccess;
    char locationStatusText[kMaxSizeStatusText + 1] = {'\0'};
    bool useLocationStatusText                      = false;
    Status cmdStatus                                = Status::Success;

    uint32_t listIndex = 0;
    uint32_t oldSelectedLocation;

    // On receipt of this command the device SHALL respond with a SelectLocationsResponse command.
    Commands::SelectLocationsResponse::Type response;

    size_t numberOfLocations = 0;
    bool   matchesCurrentSelectedLocations;

    // get number of Selected Locations in command parameter
    VerifyOrExit((req.newLocations.IsNull() || (CHIP_NO_ERROR == req.newLocations.Value().ComputeSize(&numberOfLocations))), 
                                    locationStatus = SelectLocationsStatus::kInvalidSet;
                                    strncat(locationStatusText, "Select Locations command - newLocations parameter size decoding failed", kMaxSizeStatusText);
                                    useLocationStatusText = true;
                                    ChipLogError(Zcl, "%s", locationStatusText)  );

    // If the device determines that it can't operate at all locations from the list,
    // the SelectLocationsResponse command's Status field SHALL indicate InvalidSet.
    // If this field is null, that indicates that the device is to operate without being constrained to any specific location(s).
    VerifyOrExit((req.newLocations.IsNull() || ((numberOfLocations != 0) && (numberOfLocations <= kMaxNumSelectedLocations))),
                                locationStatus = SelectLocationsStatus::kInvalidSet;
                                strncat(locationStatusText, "Select Locations command - invalid number of locations", kMaxSizeStatusText);
                                ChipLogError(Zcl, "%s", locationStatusText)   );


    // if number of selected locations in parameter matchs number in attribute - the locations *might* be the same 
    matchesCurrentSelectedLocations = (numberOfLocations == mDelegate->GetNumberOfSelectedLocations());

    // do as much parameter validation as we can
    if (!req.newLocations.IsNull())
    {
        auto locationIter = req.newLocations.Value().begin();
        while (locationIter.Next()) 
        {
            uint32_t aSelectedLocation = locationIter.GetValue();

            // each item in this list SHALL match the LocationID field of an entry on the SupportedLocations attribute's list
            // If the Status field is set to UnsupportedLocation, the StatusText field SHALL be an empty string.
            VerifyOrExit(IsSupportedLocation(aSelectedLocation), 
                            locationStatus = SelectLocationsStatus::kInvalidSet;
                            useLocationStatusText = true;
                            ChipLogError(Zcl, "HandleSelectLocationsCmd - unsupported location %u", aSelectedLocation));

            // check to see if parameter list and attribute still match
            if (matchesCurrentSelectedLocations)
            {
                if (!mDelegate->GetSelectedLocationByIndex(listIndex, oldSelectedLocation) ||
                    (aSelectedLocation != oldSelectedLocation)  )
                {
                    matchesCurrentSelectedLocations = false;
                }
            }
        }

        // after iterating with Next through DecodableType - check for failure
        VerifyOrExit(CHIP_NO_ERROR == locationIter.GetStatus(), 
                                locationStatus = SelectLocationsStatus::kInvalidSet;
                                strncat(locationStatusText, "SelectLocations command - newLocations parameter value decoding failed", kMaxSizeStatusText);
                                ChipLogError(Zcl, "%s", locationStatusText)  );
    }


    // If the NewLocations field is the same as the value of the SelectedLocations attribute
    // the SelectLocationsResponse command SHALL have the Status field set to Success and
    // the StatusText field MAY be supplied with a human readable string or include an empty string.
    VerifyOrExit(!matchesCurrentSelectedLocations, cmdStatus = Status::Success);

    // If the current state of the device doesn't allow for the locations to be selected,
    // the SelectLocationsResponse command SHALL have the Status field set to InvalidInMode.
    // if the Status field is set to InvalidInMode, the StatusText field SHOULD indicate why the request is not allowed, 
    // given the current mode of the device, which may involve other clusters. 
    // (note - locationStatusText to be filled out by delegated function for if return value is false)
    VerifyOrExit(mDelegate->IsSetSelectedLocationsAllowed(locationStatusText),
                                                            locationStatus = SelectLocationsStatus::kInvalidInMode;  
                                                            cmdStatus = Status::Failure);


    // ask the device to handle SelectLocations Command
    // (note - locationStatusText to be filled out by delegated function for kInvalidInMode and InvalidSet)
    VerifyOrExit(mDelegate->HandleSetSelectLocations(req, locationStatus, locationStatusText, useLocationStatusText), 
                                    cmdStatus = Status::Failure);

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


exit:
    response.status = locationStatus;

    switch (locationStatus)
    {
    // If the Status field is set to UnsupportedLocation, the StatusText field SHALL be an empty string.
    // If the Status field is set to DuplicatedLocations, the StatusText field SHALL be an empty string.
    case SelectLocationsStatus::kUnsupportedLocation:
    case SelectLocationsStatus::kDuplicatedLocations:
        response.statusText = chip::Optional(chip::CharSpan::fromCharString(""));
        break;

    // If the Status field is set to Success, the StatusText field is optional.
    case SelectLocationsStatus::kSuccess:
        if (useLocationStatusText)
        {
            response.statusText = chip::Optional(chip::CharSpan::fromCharString( locationStatusText));
        }
        break;

    // If the Status field is not set to Success, or UnsupportedLocation, or DuplicatedLocations,
    // the StatusText field SHALL include a vendor-defined error description
    default:
        response.statusText = chip::Optional(chip::CharSpan::fromCharString( locationStatusText));
        break;

    } // end switch


    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, cmdStatus);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}


void Instance::HandleSkipCurrentLocationCmd(HandlerContext & ctx)
{
    ChipLogDetail(Zcl, "Service Area: HandleSkipCurrentLocation");

    SkipCurrentLocationStatus skipStatus                  = SkipCurrentLocationStatus::kSuccess;
    char   skipStatusText[kMaxSizeStatusText + 1] = {'\0'};
    Status cmdStatus                              = Status::Success;

    // On receipt of this command the device SHALL respond with a SkipCurrentLocationResponse command. 
    Commands::SkipCurrentLocationResponse::Type response;

    // If the SelectedLocations attribute is null, the response status should be set to InvalidLocationList.
    // If the Status field is set to InvalidLocationList, the StatusText field SHALL be an empty string.
    VerifyOrExit((mDelegate->GetNumberOfSelectedLocations() != 0), 
                            skipStatus  = SkipCurrentLocationStatus::kInvalidLocationList;
                            ChipLogError(Zcl, "Skip Current Location command - Selected Locations atttribute is null");
                            cmdStatus = Status::Failure );

    // If the CurrentLocation attribute is null, the status should be set to InvalidInMode.
    // If the Status field is not set to Success, or InvalidLocationList, the StatusText field SHALL include a vendor defined error description.
    VerifyOrExit(!mCurrentLocation.IsNull(), 
                            skipStatus  = SkipCurrentLocationStatus::kInvalidInMode;
                            strncat(skipStatusText, "SkipCurrentLocation command - Current Location attribute is null", kMaxSizeStatusText);
                            ChipLogError(Zcl, "%s", skipStatusText);
                            cmdStatus = Status::Failure );

    // have the device attempt to skip     
    // If the Status field is not set to Success, or InvalidLocationList, the StatusText field SHALL include a vendor defined error description. 
    // InvalidInMode | The received request cannot be handled due to the current mode of the device.    
    // (skipStatusText to be filled out by delegated function on failure.)   
    VerifyOrExit(mDelegate->HandleSkipCurrentLocation(skipStatusText), skipStatus  = SkipCurrentLocationStatus::kInvalidInMode; cmdStatus = Status::Failure);


exit:
    response.status = skipStatus;

    switch (skipStatus)
    {
    // If the Status field is set to InvalidLocationList, the StatusText field SHALL be an empty string
    case SkipCurrentLocationStatus::kInvalidLocationList:
        response.statusText = chip::Optional(chip::CharSpan::fromCharString(""));
        break;

    case SkipCurrentLocationStatus::kSuccess:
        break;

    // If the Status field is not set to Success, or InvalidLocationList,
    // the StatusText field SHALL include a vendor defined error description
    default:
        response.statusText = chip::Optional(chip::CharSpan::fromCharString(skipStatusText));
        break;

    } // end switch

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, cmdStatus);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
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
    bool ret_value = false;

    // If the HomeLocationInfo field is null, the LandmarkTag field SHALL NOT be null.
    // If the LandmarkTag field is null, the HomeLocationInfo field SHALL NOT be null.
    VerifyOrExit((!aLocation.locationInfo.locationInfo.IsNull()) || (!aLocation.locationInfo.landmarkTag.IsNull()),
                ChipLogError(Zcl,  "IsValidSupportedLocation %u - must have locationInfo and/or LandmarkTag", aLocation.locationID));


    if (!aLocation.locationInfo.locationInfo.IsNull())
    {
        // If HomeLocationInfo is not null, and its LocationName field is an empty string, at least one of the following SHALL NOT be null:
        // HomeLocationInfo's FloorNumber field, HomeLocationInfo's AreaType field, the LandmarkTag field

        // If all three of the following are null, HomeLocationInfo's LocationName field SHALL NOT be an empty string:
        // HomeLocationInfo's FloorNumber field HomeLocationInfo's AreaType field, the LandmarkTag field
        VerifyOrExit(  ((aLocation.locationInfo.locationInfo.Value().locationName.size() != 0) ||
                        !aLocation.locationInfo.locationInfo.Value().floorNumber.IsNull() || 
                        !aLocation.locationInfo.locationInfo.Value().areaType.IsNull() ||
                        !aLocation.locationInfo.landmarkTag.IsNull()),
                    ChipLogError(Zcl,  "IsValidSupportedLocation %u - LocationName is empty string, Floornumber, AreaType, LandmarkTag are null", aLocation.locationID));
    }

    // If the LandmarkTag field is null, the PositionTag field SHALL be null.
    VerifyOrExit((!aLocation.locationInfo.landmarkTag.IsNull() || aLocation.locationInfo.positionTag.IsNull()),
                ChipLogError(Zcl,  "IsValidSupportedLocation %u - PositionTag with no LandmarkTag", aLocation.locationID));

    if (mDelegate->GetNumberOfSupportedMaps()== 0)
    {
        // If the SupportedMaps attribute is null, mapid SHALL be null.
        VerifyOrExit(aLocation.mapID.IsNull(),
                    ChipLogError(Zcl,  "IsValidSupportedLocation %u - map Id %u is not in empty supported map list ", aLocation.locationID, aLocation.mapID.Value()));
    }
    else
    {
        // If the SupportedMaps attribute is not null, mapID SHALL be the ID of an entry from the SupportedMaps attribute.
        VerifyOrExit((IsSupportedMap(aLocation.mapID.Value())),
                    ChipLogError(Zcl,  "IsValidSupportedLocation %u - map Id %u is not in supported map list ", aLocation.locationID, aLocation.mapID.Value()));
    }

    //success
    ret_value = true;


exit:
    return ret_value;
}


bool Instance::IsUniqueSupportedLocation(const LocationStructureWrapper & aLocation, bool ignoreLocationId)
{
    uint8_t                   locationIndex = 0;
    LocationStructureWrapper  entry;

    // If the SupportedMaps attribute is not null, each entry in this list SHALL have a unique value for the combination of the MapID and LocationInfo fields.
    // If the SupportedMaps attribute is null, each entry in this list SHALL have a unique value for the LocationInfo field.
    bool isSupportedMapsNull = mDelegate->GetNumberOfSupportedMaps() == 0;

    while (mDelegate->GetSupportedLocationByIndex(locationIndex++, entry))
    {
        if (aLocation.IsEqual(entry, ignoreLocationId, isSupportedMapsNull))
        {
            return false;
        }
    }

    return true;
}

bool Instance::AddSupportedLocation( uint32_t                                     aLocationId,
                                     const DataModel::Nullable<uint8_t>         & aMapId,
                                     const CharSpan                             & aLocationName,
                                     const DataModel::Nullable<int16_t>         & aFloorNumber,
                                     const DataModel::Nullable<AreaTypeTag>     & aAreaType,
                                     const DataModel::Nullable<LandmarkTag>     & aLandmarkTag,
                                     const DataModel::Nullable<PositionTag>     & aPositionTag,
                                     const DataModel::Nullable<FloorSurfaceTag> & aSurfaceTag  )
{
    bool ret_value = false;
    uint32_t dummyIndex;

    // create location object for validation
    LocationStructureWrapper aNewLocation(  aLocationId, aMapId,
                                            aLocationName, aFloorNumber, aAreaType, 
                                            aLandmarkTag, aPositionTag, aSurfaceTag);

    // does device mode allow this attribute to be updated?
    VerifyOrExit(mDelegate->IsSupportedLocationsChangeAllowed(), /* if false, should be logged as error in delegate function */);

    // check max# of list entries
    VerifyOrExit((kMaxNumSupportedLocations > mDelegate->GetNumberOfSupportedLocations()),
                ChipLogError(Zcl,  "AddSupportedLocation %u - too many entries", aLocationId));


    // verify cluster requirements concerning valid fields and field relationships
    VerifyOrExit(IsValidSupportedLocation(aNewLocation), 
                ChipLogError(Zcl,  "AddSupportedLocation %u - not a valid location object", aNewLocation.locationID));

    // Each entry in Supported Locations SHALL have a unique value for the ID field.
    // If the SupportedMaps attribute is not null, each entry in this list SHALL have a unique value for the combination of the MapID and LocationInfo fields.
    // If the SupportedMaps attribute is null, each entry in this list SHALL have a unique value for the LocationInfo field.
    VerifyOrExit(IsUniqueSupportedLocation(aNewLocation, false),
                ChipLogError(Zcl,  "AddSupportedLocation %u - not a unique location object", aNewLocation.locationID));

    // add to supported locations attribute
    VerifyOrExit( mDelegate->AddSupportedLocation(aNewLocation, dummyIndex), /* log error in delegate function*/);


    // success!
    ret_value = true;
    NotifySupportedLocationsChanged();

exit:
    return ret_value;
}


bool Instance::ModifySupportedLocation( uint32_t                                     aLocationId, 
                                        const DataModel::Nullable<uint8_t>         & aMapId, 
                                        const CharSpan                             & aLocationName,
                                        const DataModel::Nullable<int16_t>         & aFloorNumber,
                                        const DataModel::Nullable<AreaTypeTag>     & aAreaType,
                                        const DataModel::Nullable<LandmarkTag>     & aLandmarkTag,
                                        const DataModel::Nullable<PositionTag>     & aPositionTag,
                                        const DataModel::Nullable<FloorSurfaceTag> & aSurfaceTag  )
{
    bool ret_value = false;
    bool mapIDChanged = false;
    uint32_t listIndex;

    // get existing supported location to modify
    LocationStructureWrapper supportedLocation;
    bool locationExists = mDelegate->GetSupportedLocationById(aLocationId, listIndex, supportedLocation);

    VerifyOrExit(locationExists,
            ChipLogError(Zcl,  "ModifySupportedLocation %u - not a supported locationID", aLocationId)); 

    {
        // check for mapID change
        if ((aMapId.IsNull() != supportedLocation.mapID.IsNull())  ||

            (!aMapId.IsNull() && !supportedLocation.mapID.IsNull()
             && (aMapId.Value() != supportedLocation.mapID.Value()))  )    
        {
            // does device mode allow this attribute to be updated?
            VerifyOrExit(mDelegate->IsSupportedLocationsChangeAllowed(), /* if false, should be logged as error in delegate function */);

            mapIDChanged = true;
        }

        // create new location object for validation
        LocationStructureWrapper aNewLocation(  aLocationId, aMapId,
                                                aLocationName, aFloorNumber, aAreaType, 
                                                aLandmarkTag, aPositionTag, aSurfaceTag);

        // verify cluster requirements concerning valid fields and field relationships
        VerifyOrExit(IsValidSupportedLocation(aNewLocation), 
                    ChipLogError(Zcl,  "ModifySupportedLocation %u - not a valid location object", aNewLocation.locationID));

        // updated location description must not match another existing location description.
        // We ignore comparing the location ID as one of the locations will match this one.
        VerifyOrExit(IsUniqueSupportedLocation(aNewLocation, true),
                    ChipLogError(Zcl,  "ModifySupportedLocation %u - not a unique location object", aNewLocation.locationID));

        // note: we already checked locationExists, we don't need to do it again here.

        // replace the supported location with the modified location 
        VerifyOrExit(mDelegate->ModifySupportedLocation(listIndex, aNewLocation), /* if false, should be logged as error in delegate function */);
    }

    ret_value = true;
    if (mapIDChanged)
    {
        mDelegate->HandleSupportedLocationsUpdated();
    }

    NotifySupportedLocationsChanged();


exit:
    return ret_value; 
}


bool Instance::ClearSupportedLocations()
{
    bool ret_value = false;

    // does device mode allow this attribute to be updated?
    VerifyOrExit(mDelegate->IsSupportedLocationsChangeAllowed(), /* if false, should be logged as error in delegate function */);

    if (mDelegate->ClearSupportedLocations())
    {
        mDelegate->HandleSupportedLocationsUpdated();
        NotifySupportedLocationsChanged();

        ret_value = true;
    }

exit:
    return ret_value;
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
    bool ret_value = false;

    uint8_t              mapIndex = 0;
    MapStructureWrapper  entry;

    // does device mode allow this attribute to be updated?
    VerifyOrExit(mDelegate->IsSupportedMapChangeAllowed(), /* if false, should be logged as error in delegate function */);

    // check max# of list entries
    VerifyOrExit((mDelegate->GetNumberOfSupportedMaps() < kMaxNumSupportedMaps), 
                ChipLogError(Zcl,  "AddSupportedMap %u - maximum number of entries", aMapId));

    //  Map name SHALL include readable text that describes the mapname (cannot be empty string)
    VerifyOrExit((aMapName.size() != 0),
                ChipLogError(Zcl,  "AddSupportedMap %u - Name must not be empty string", aMapId));


    // Each entry in this list SHALL have a unique value for the Name field.
    while (mDelegate->GetSupportedMapByIndex(mapIndex++, entry))
    {
        // the name cannot be the same as an existing map
        VerifyOrExit(!entry.DoesNameMatch(aMapName),
                ChipLogError(Zcl,  "AddSupportedMap %u - map already exists with same name '%s'", aMapId, entry.name_c_str()));

        //  Each entry in this list SHALL have a unique value for the MapID field.
        VerifyOrExit((aMapId != entry.mapID),
                ChipLogError(Zcl,  "AddSupportedMap - non-unique Id %u", aMapId));
    }
 
    {
        // add to supported maps attribute
        MapStructureWrapper newMap(aMapId, aMapName);
        uint32_t dummyIndex;
        VerifyOrExit( mDelegate->AddSupportedMap(newMap, dummyIndex), /* log error in delegate function*/);
    }

    // map successfully added
    ret_value = true;
    NotifySupportedMapsChanged();
    
exit:
    return ret_value;
}


bool Instance::RenameSupportedMap(uint8_t aMapId, const CharSpan & newMapName)
{
    bool ret_value = false;

    uint32_t modifiedIndex;
    uint32_t loopIndex = 0;
    MapStructureWrapper modifiedMap;
    MapStructureWrapper entry;

    // get existing entry 
    bool mapExists = mDelegate->GetSupportedMapById(aMapId, modifiedIndex, modifiedMap);

    VerifyOrExit(mapExists, ChipLogError(Zcl,  "RenameSupportedMap Id %u - map does not exist", aMapId));

    //  Map name SHALL include readable text that describes the mapname (cannot be empty string)
    VerifyOrExit((newMapName.size() != 0),
                ChipLogError(Zcl,  "RenameSupportedMap %u - Name must not be empty string", aMapId));


    // update the local copy of the map
    modifiedMap.Set(modifiedMap.mapID, newMapName);

    // Each entry in this list SHALL have a unique value for the Name field.
    while (mDelegate->GetSupportedMapByIndex(loopIndex, entry))
    {
        if (modifiedIndex == loopIndex)
        {
            continue; // don't check local modified map against it's own list entry
        }

        VerifyOrExit(!entry.DoesNameMatch(newMapName),
                ChipLogError(Zcl,  "AddSupportedMap %u - map already exists with same name '%s'", aMapId, entry.name_c_str()));

        ++loopIndex;
    }

    VerifyOrExit( mDelegate->ModifySupportedMap(modifiedIndex, modifiedMap), /* log error in delegate function*/);

    // map successfully renamed
    ret_value = true;
    NotifySupportedMapsChanged();
    // note - no need to modifiy other lists when a map is renamed


exit:
    return ret_value;
}


bool Instance::ClearSupportedMaps()
{
    bool ret_value = false;

    // does device mode allow this attribute to be updated?
    VerifyOrExit(mDelegate->IsSupportedMapChangeAllowed(), /* if false, should be logged as error in delegate function */);

    if (mDelegate->ClearSupportedMaps())
    {
        ret_value = true;
        ClearSupportedLocations();
        NotifySupportedMapsChanged();
    }

exit:
    return ret_value;
}


//*************************************************************************
// Selected Locations manipulators

bool Instance::AddSelectedLocation(uint32_t & aSelectedLocation)
{
    bool     ret_value;
    uint32_t dummyIndex;

    char locationStatusText[kMaxSizeStatusText + 1] = {'\0'};

    // check max# of list entries
    VerifyOrExit((mDelegate->GetNumberOfSelectedLocations() < kMaxNumSelectedLocations), 
                    ChipLogError(Zcl,  "AddSelectedLocation %u - maximum number of entries", aSelectedLocation));

    // each item in this list SHALL match the LocationID field of an entry on the SupportedLocations attribute's list
    VerifyOrExit(IsSupportedLocation(aSelectedLocation), 
                    ChipLogError(Zcl, "AddSelectedLocation - unsupported location %u", aSelectedLocation));

    // each entry in this list SHALL have a unique value
    VerifyOrExit(!mDelegate->IsSelectedLocation(aSelectedLocation), 
                    ChipLogError(Zcl, "AddSelectedLocation %u - duplicated location", aSelectedLocation));

    // Does device mode allow modification of selected locations?
    VerifyOrExit(mDelegate->IsSetSelectedLocationsAllowed(locationStatusText),
                    ChipLogError(Zcl, "AddSelectedLocation %u - %s", aSelectedLocation, locationStatusText));


    VerifyOrExit(mDelegate->AddSelectedLocation(aSelectedLocation, dummyIndex), /* if false, should be logged as error in delegate function */);


    // success
    ret_value = true;

exit:
    return ret_value;
}


bool Instance::ClearSelectedLocations()
{
    bool ret_value = false;

    if (mDelegate->ClearSelectedLocations())
    {
        ret_value = true;
        NotifySelectedLocationsChanged();
    }

    return ret_value;
}


//*************************************************************************
// Current Location manipulators

DataModel::Nullable<uint32_t> Instance::GetCurrentLocation()
{
    return mCurrentLocation;
}

bool Instance::SetCurrentLocation(const DataModel::Nullable<uint32_t> & aCurrentLocation)
{
    // If not null, the value of this attribute SHALL match the LocationID field of an entry on the SupportedLocations attribute's list.
    if ((!aCurrentLocation.IsNull()) &&
        (!IsSupportedLocation(aCurrentLocation.Value())))
    {
        ChipLogError(Zcl,  "SetCurrentLocation %u - location is not supported", aCurrentLocation.Value());
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
    bool ret_value = false;
    bool doChangeNotify = false;

    // EstimatedEndTime SHALL be null if the CurrentLocation attribute is null.
    VerifyOrExit((!mCurrentLocation.IsNull() || aEstimatedEndTime.IsNull()),
        ChipLogError(Zcl,  "SetEstimatedEndTime - must be null if Current Location is null"));

    // The value of this attribute SHALL only be reported in the following cases:
    // - when it changes from 0 to any other value and vice versa
    // - when it decreases
    // - when it changes from null to any non-zero value
    if ((!mEstimatedEndTime.IsNull() && !aEstimatedEndTime.IsNull() && (aEstimatedEndTime.Value() == 0) && mEstimatedEndTime.Value()!= 0)   ||
        (!mEstimatedEndTime.IsNull() && !aEstimatedEndTime.IsNull() && (aEstimatedEndTime.Value() != 0) && mEstimatedEndTime.Value()== 0)   ||
        (!mEstimatedEndTime.IsNull() && !aEstimatedEndTime.IsNull() && (aEstimatedEndTime.Value() < mEstimatedEndTime.Value()))             ||
        (mEstimatedEndTime.IsNull()  && !aEstimatedEndTime.IsNull() && (aEstimatedEndTime.Value() != 0))  )
    {
        doChangeNotify = true;
    }

    mEstimatedEndTime = aEstimatedEndTime;

    if (doChangeNotify)
    {
        NotifyEstimatedEndTimeChanged();
    }

    // success
    ret_value = true;


exit:
    return ret_value;
}


//*************************************************************************
// Progress list manipulators

bool Instance::AddPendingProgressElement(uint32_t aLocationId)
{
    bool     ret_value = false;
    uint32_t dummyIndex;

    // create progress element
    Structs::ProgressStruct::Type inactiveProgress = { aLocationId, OperationalStatusEnum::kPending};

    // check max# of list entries
    VerifyOrExit((mDelegate->GetNumberOfProgressElements() < kMaxNumProgressElements),  // Note: progress elements must uniquely map to supported locations, so max size is the same
                ChipLogError(Zcl,  "AddPendingProgressElement - maximum number of entries"));

    // For each entry in this list, the LocationID field SHALL match an entry on the SupportedLocations attribute's list.
    VerifyOrExit(IsSupportedLocation(aLocationId),
                ChipLogError(Zcl,  "AddPendingProgressElement - not a supported location %u", aLocationId));

    // Each entry in this list SHALL have a unique value for the LocationID field.

    VerifyOrExit(!IsProgressElement(aLocationId),
                ChipLogError(Zcl,  "AddPendingProgressElement - progress element already exists for location %u", aLocationId));         


    VerifyOrExit(mDelegate->AddProgressElement(inactiveProgress, dummyIndex), /* if false, should be logged as error in delegate function */);


    // success
    ret_value = true;
    NotifyProgressChanged();

exit:
    return ret_value;
}


bool Instance::SetProgressStatus(uint32_t aLocationId, OperationalStatusEnum opStatus)
{
    bool     ret_value = false;
    uint32_t listIndex;

    Structs::ProgressStruct::Type progressElement;

    VerifyOrExit(mDelegate->GetProgressElementById(aLocationId, listIndex, progressElement),
                ChipLogError(Zcl,  "SetProgressStatus - progress element does not exist for location %u", aLocationId));


    // if status value not changing, no need to modify the existing element
    VerifyOrExit((progressElement.status != opStatus), ret_value = true);

    // set the progress status in the local copy
    progressElement.status = opStatus;

    // TotalOperationalTime SHALL be null if the Status field is not set to Completed or Skipped.
    if ((opStatus != OperationalStatusEnum::kCompleted) &&
        (opStatus != OperationalStatusEnum::kSkipped))
    {
        progressElement.totalOperationalTime.Value().SetNull();
    }

    // add the updated element to the progress attribute
    VerifyOrExit(mDelegate->ModifyProgressElement(listIndex, progressElement), /* if false, should be logged as error in delegate function */);


    // success
    ret_value = true;
    NotifyProgressChanged();

 exit:
    return ret_value;
}


bool Instance::SetProgressTotalOperationalTime(uint32_t aLocationId, const DataModel::Nullable<uint32_t> & aTotalOperationalTime)
{
    bool     ret_value = false;
    uint32_t listIndex;

    Structs::ProgressStruct::Type progressElement;

    VerifyOrExit(mDelegate->GetProgressElementById(aLocationId, listIndex, progressElement),
                ChipLogError(Zcl,  "SetProgressTotalOperationalTime - progress element does not exist for location %u", aLocationId));

    // if time value not changing, no need to modify the existing element
    VerifyOrExit((progressElement.totalOperationalTime != aTotalOperationalTime), ret_value = true);

    // This attribute SHALL be null if the Status field is not set to Completed or Skipped
    VerifyOrExit((aTotalOperationalTime.IsNull() || (progressElement.status == OperationalStatusEnum::kCompleted) || (progressElement.status == OperationalStatusEnum::kSkipped)),
            ChipLogError(Zcl,  "SetProgressTotalOperationalTime - location %u opStatus value %u - can be non-null only if opStatus is Completed or Skipped",
                                aLocationId, to_underlying(progressElement.status)));

    // set the time in the local copy
    progressElement.totalOperationalTime.Value() = aTotalOperationalTime;

    // add the updated element to the progress attribute
    VerifyOrExit(mDelegate->ModifyProgressElement(listIndex, progressElement), /* if false, should be logged as error in delegate function */);


    // success
    ret_value = true;
    NotifyProgressChanged();

 exit:
    return ret_value;
}


bool Instance::SetProgressEstimatedTime(uint32_t aLocationId, const DataModel::Nullable<uint32_t> & aEstimatedTime)
{
    bool     ret_value = false;
    uint32_t listIndex;

    Structs::ProgressStruct::Type progressElement;

    VerifyOrExit(mDelegate->GetProgressElementById(aLocationId, listIndex, progressElement),
                ChipLogError(Zcl,  "SetProgressEstimatedTime - progress element does not exist for location %u", aLocationId));

    // if time value not changing, no need to modify the existing element
    VerifyOrExit((progressElement.estimatedTime != aEstimatedTime), ret_value = true);

    // set the time in the local copy
    progressElement.estimatedTime.Value() = aEstimatedTime;

    // add the updated element to the progress attribute
    VerifyOrExit(mDelegate->ModifyProgressElement(listIndex, progressElement), /* if false, should be logged as error in delegate function */);


    // success
    ret_value = true;
    NotifyProgressChanged();

 exit:
    return ret_value;
}


bool Instance::ClearProgress()
{
    bool ret_value = false;

    if (mDelegate->ClearProgress())
    {
        ret_value = true;
        NotifyProgressChanged();
    }

    return ret_value;
}


// attribute manipulators - Feature Map

bool Instance::HasFeature(ServiceArea::Feature feature) const
{
    return mFeature.Has(feature);
}


} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip

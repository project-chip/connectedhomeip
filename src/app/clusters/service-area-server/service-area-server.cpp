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
    ChipLogProgress(Zcl, "Location Location: Instance constructor");
    mDelegate->SetInstance(this);
}

Instance::~Instance()
{
    InteractionModelEngine::GetInstance()->UnregisterCommandHandler(this);
    unregisterAttributeAccessOverride(this);
}

CHIP_ERROR Instance::Init()
{
    ChipLogProgress(Zcl, "Location Location: INIT");

    // Check if the cluster has been selected in zap
    VerifyOrReturnError(
        emberAfContainsServer(mEndpointId, chip::app::Clusters::ServiceArea::Id), CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(Zcl, "Location: The cluster with Id %lu was not enabled in zap.", long(chip::app::Clusters::ServiceArea::Id)));

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
    ChipLogError(Zcl, "Location Location: Reading");

    switch (aPath.mAttributeId)
    {

    case ServiceArea::Attributes::SupportedLocations::Id: {
        ChipLogProgress(Zcl, "Location Location: Read SupportedLocations");
        ReturnErrorOnFailure(ReadSupportedLocations(aEncoder));
        break;
    }

    case ServiceArea::Attributes::SupportedMaps::Id: {
        ChipLogProgress(Zcl, "Location Location: Read SupportedMaps");
        ReturnErrorOnFailure(ReadSupportedMaps(aEncoder));
        break;
    } 

    case ServiceArea::Attributes::SelectedLocations::Id: {
        ChipLogProgress(Zcl, "Location Location: Read SelectedLocations");
        ReturnErrorOnFailure(ReadSelectedLocations(aEncoder));
        break;
    } 

    case ServiceArea::Attributes::CurrentLocation::Id: {
        ChipLogProgress(Zcl, "Location Location: Read CurrentLocation");
        ReturnErrorOnFailure(aEncoder.Encode(GetCurrentLocation()));
        break;     
    }

    case ServiceArea::Attributes::EstimatedEndTime::Id: {
        ChipLogProgress(Zcl, "Location Location: Read EstimatedEndTime");
        ReturnErrorOnFailure(aEncoder.Encode(GetEstimatedEndTime()));
        break;
    }

    case ServiceArea::Attributes::Progress::Id: {
        ChipLogProgress(Zcl, "Location Location: Read Progress");
        ReturnErrorOnFailure(ReadProgress(aEncoder));
        break;
    }

    case ServiceArea::Attributes::FeatureMap::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mFeature.Raw()));
        break;
    }
  
    default:
        ChipLogProgress(Zcl, "Location Location: Read unsupported attribute %u", aPath.mAttributeId);
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

    case Commands::SkipCurrent::Id:

        CommandHandlerInterface::HandleCommand<Commands::SkipCurrent::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleSkipCurrentCmd(ctx); });
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
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (auto & entry : mSupportedLocations)
        {
            ReturnErrorOnFailure(encoder.Encode(entry.second));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR Instance::ReadSupportedMaps(chip::app::AttributeValueEncoder & aEncoder) 
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (auto & entry : mSupportedMaps)
        {
            ReturnErrorOnFailure(encoder.Encode(entry.second));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR Instance::ReadSelectedLocations(chip::app::AttributeValueEncoder & aEncoder) 
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (uint32_t & entry : mSelectedLocations)
        {
            ReturnErrorOnFailure(encoder.Encode(entry));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR Instance::ReadProgress(chip::app::AttributeValueEncoder & aEncoder) 
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR
    {
        // give the device a chance to make sure values are up-to-date
        mDelegate->HandleVolatileProgressList();

        for (ProgressPairType & entry : mProgressList)
        {
            ReturnErrorOnFailure(encoder.Encode(entry.second));
        }
        return CHIP_NO_ERROR;
    });
}


//*************************************************************************
// command handlers

void Instance::HandleSelectLocationsCmd(HandlerContext & ctx, const Commands::SelectLocations::DecodableType & req)
{
    ChipLogDetail(Zcl, "HandleSelectLocationsCmd");
    SelectLocationsStatus locationStatus = SelectLocationsStatus::kSuccess;
    std::string           locationStatusText; 

    Status cmdStatus = Status::Success;

    // On receipt of this command the device SHALL respond with a SelectLocationsResponse command.
    Commands::SelectLocationsResponse::Type response;

    std::vector<uint32_t> newSelectLocations;  // save extracted locations from command parameter
    size_t numberOfLocations = 0;
    bool   matchesCurrentSelectedLocations = false;

    // get number of Selected Locations in command parameter
    VerifyOrExit((req.newLocations.IsNull() || (CHIP_NO_ERROR == req.newLocations.Value().ComputeSize(&numberOfLocations))), 
                                    locationStatus = SelectLocationsStatus::kInvalidSet;
                                    locationStatusText = "Select Locations command - newLocations parameter failed decoding size";
                                    ChipLogError(Zcl, "%s", locationStatusText.c_str())  );

    // If this field is an empty list, or if the device determines that it can't operate at all locations from the list,
    // the SelectLocationsResponse command's Status field SHALL indicate InvalidSet.
    // If this field is null, that indicates that the device is to operate without being constrained to any specific location(s).
    VerifyOrExit((req.newLocations.IsNull() || ((numberOfLocations != 0) && (numberOfLocations <= kMaxNumSelectedLocations))),
                                locationStatus = SelectLocationsStatus::kInvalidSet;
                                locationStatusText = "Select Locations command - invalid number of locations " + numberOfLocations;
                                ChipLogError(Zcl, "%s", locationStatusText.c_str())   );

    // decode the values of Selected Locations in command parameter
    {
        newSelectLocations.reserve(numberOfLocations);

        if (!req.newLocations.IsNull())
        {
            auto locationIter = req.newLocations.Value().begin();
            while (locationIter.Next()) 
            {
                newSelectLocations.push_back(locationIter.GetValue());  // temporary save of the decoded location values
            }

            VerifyOrExit(CHIP_NO_ERROR == locationIter.GetStatus(), 
                                    locationStatus = SelectLocationsStatus::kInvalidSet;
                                    locationStatusText = "SelectLocations command - newLocations parameter failed decoding value";
                                    ChipLogError(Zcl, "%s", locationStatusText.c_str())  );
        }
    }

    // If the NewLocations field is the same as the value of the SelectedLocations attribute
    // the SelectLocationsResponse command SHALL have the Status field set to Success and
    // the StatusText field MAY be supplied with a human readable string or include an empty string.
    if (numberOfLocations == mSelectedLocations.size())
    {
        matchesCurrentSelectedLocations = true; // might be true
        size_t locationIndex = 0;
        for (auto entry : mSelectedLocations)
        {
            if (entry != newSelectLocations[locationIndex++])
            {
                matchesCurrentSelectedLocations = false; // NOT true after all
                break;
            }
        }
    }

    // if new selected locations match current selected locations, no action or further validation is needed, command is successful
    VerifyOrExit(!matchesCurrentSelectedLocations, cmdStatus = Status::Success);

    // validate command's Selected Locations values against cluster requirements
    VerifyOrExit(AreSetSelectLocationsParamsValid(newSelectLocations, locationStatus, locationStatusText), cmdStatus = Status::Failure);

    // If the current state of the device doesn't allow for the locations to be selected,
    // the SelectLocationsResponse command SHALL have the Status field set to InvalidInMode.
    // if the Status field is set to InvalidInMode, the StatusText field SHOULD indicate why the request is not allowed, 
    // given the current mode of the device, which may involve other clusters. 
    // (note - locationStatusText to be filled out by delegated function for if return value is false)
    VerifyOrExit(mDelegate->IsSetSelectedLocationAllowed(locationStatusText), 
                                                            locationStatus =SelectLocationsStatus::kInvalidInMode;  
                                                            cmdStatus = Status::Failure);


    // ask the device to handle SelectLocations Command
    // (note - locationStatusText to be filled out by delegated function for kInvalidInMode and InvalidSet)
    VerifyOrExit(mDelegate->HandleSetSelectLocations(newSelectLocations, locationStatus, locationStatusText), cmdStatus = Status::Failure);

    {
        // If the device successfully accepts the request, the server will attempt to operate at the location(s)
        // indicated by the entries of the NewLocation field, when requested to operate,
        // the SelectLocationsResponse command SHALL have the Status field set to Success,
        // and the SelectedLocations attribute SHALL be set to the value of the NewLocations field.
        mSelectedLocations.clear();

        for (size_t locationIndex = 0; locationIndex < numberOfLocations; ++locationIndex)
        {
            mSelectedLocations.push_back(newSelectLocations[locationIndex]);
        }

        NotifySelectedLocationsChanged();
    }


exit:
    response.status = locationStatus;
    if (locationStatusText.length() > 0)
    {
        response.statusText = chip::CharSpan::fromCharString( locationStatusText.c_str());
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, cmdStatus);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}


bool Instance::AreSetSelectLocationsParamsValid(const std::vector<uint32_t> & selectLocations, 
                                            SelectLocationsStatus & locationStatus, std::string & locationStatusText)
{
    bool ret_val = false;
    std::map<uint32_t, uint32_t> testMap;

    for (size_t entry = 0; entry < selectLocations.size(); ++entry)
    {
        // If at least one entry on the NewLocations field doesn't match the LocationID field of any entry of the SupportedLocations list,
        // the SelectLocationsResponse command's Status field SHALL indicate UnsupportedLocation.
        // If the Status field is set to UnsupportedLocation, the StatusText field SHALL be an empty string.
        VerifyOrExit(IsSupportedLocation(selectLocations[entry]), 
                            locationStatus = SelectLocationsStatus::kUnsupportedLocation;
                            ChipLogError(Zcl, "AreSetSelectLocationsParamsValid - unsupported location %u", selectLocations[entry]) );

        // If this field contains any duplicated entries,
        // the SelectLocationsResponse command's Status field SHALL indicate DuplicatedLocations.
        // If the Status field is set to DuplicatedLocations, the StatusText field SHALL be an empty string.
        VerifyOrExit(testMap.emplace(entry, entry).second, 
                        locationStatus = SelectLocationsStatus::kDuplicatedLocations;
                        ChipLogError(Zcl, "AreSetSelectLocationsParamsValid - duplicated location %u", selectLocations[entry]));
    }


    // all checks passed,
    ret_val = true;
    locationStatus = SelectLocationsStatus::kSuccess;
    // If the Status field is set to Success, the StatusText field is optional.


exit:
    return ret_val;
}


void Instance::HandleSkipCurrentCmd(HandlerContext & ctx)
{
    ChipLogDetail(Zcl, "Location Location: HandleSkipCurrent");

    SkipCurrentStatus skipStatus = SkipCurrentStatus::kSuccess;
    std::string       skipStatusText; 

    Status   cmdStatus = Status::Success;

    // On receipt of this command the device SHALL respond with a SkipCurrentLocationResponse command. 
    Commands::SkipCurrentResponse::Type response;

    // InvalidLocationList | The SelectedLocations attribute is null.
    // If the Status field is set to InvalidLocationList, the StatusText field SHALL be an empty string.
    VerifyOrExit((mSelectedLocations.size() == 0), 
                                    skipStatus  = SkipCurrentStatus::kInvalidLocationList;

                                    ChipLogError(Zcl, "Skip Current Location command - Selected Locations atttribute is null");
                                    cmdStatus = Status::Failure );

    // InvalidInMode  | The received request cannot be handled due to the current mode of the device. For example, the CurrentLocation attribute is null.
    // If the Status field is not set to Success, or InvalidLocationList, the StatusText field SHALL include a vendor defined error description.
    VerifyOrExit(!mCurrentLocation.IsNull(), 
                                    skipStatus  = SkipCurrentStatus::kInvalidInMode;
                                    skipStatusText = "SkipCurrent command - Current Location attribute is null";
                                    ChipLogError(Zcl, "%s", skipStatusText.c_str());
                                    cmdStatus = Status::Failure );


    // have the device attempt to skip     
    // If the Status field is not set to Success, or InvalidLocationList, the StatusText field SHALL include a vendor defined error description. 
    // InvalidInMode | The received request cannot be handled due to the current mode of the device.    
    // (skipStatusText to be filled out by delegated function on failure.)   
    VerifyOrExit(mDelegate->HandleSkipCurrentLocation(skipStatusText), skipStatus  = SkipCurrentStatus::kInvalidInMode; cmdStatus = Status::Failure);


exit:
    response.status = skipStatus;
    if (skipStatusText.length() > 0)
    {
        response.statusText = chip::CharSpan::fromCharString( skipStatusText.c_str());
    }

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

void  Instance::GetSupportedLocationIds(std::vector<uint32_t> & aSupportedLocationIds)
{
    aSupportedLocationIds.reserve(mSupportedLocations.size());

    for (SupportedLocationPairType & entry : mSupportedLocations)
    {
        aSupportedLocationIds.push_back(entry.second.locationId);
    }
}

bool Instance::GetSupportedLocationById(uint32_t aLocationId, const LocationStructureWrapper*& aSupportedLocation)
{
    bool ret_value = false;

    auto locationIter = mSupportedLocations.find(aLocationId);

    if (locationIter != mSupportedLocations.end())
    {
        aSupportedLocation = &locationIter->second;
        ret_value = true;
    }

    return ret_value;
}

bool Instance::IsSupportedLocation(uint32_t aLocationId)
{
    auto locationIter = mSupportedLocations.find(aLocationId);
    return (locationIter != mSupportedLocations.end());
}


bool Instance::IsValidSupportedLocation(const LocationStructureWrapper & aLocation)
{
    bool ret_value = false;

    // If the HomeLocationInfo field is null, the LandmarkTag field SHALL NOT be null.
    // If the LandmarkTag field is null, the HomeLocationInfo field SHALL NOT be null.
    VerifyOrExit((!aLocation.locationInfo.homeLocationInfo.IsNull()) || (!aLocation.locationInfo.landmarkTag.IsNull()),
                ChipLogError(Zcl,  "IsValidSupportedLocation %u - must have homeLocationInfo and/or LandmarkTag", aLocation.locationId));


    if (!aLocation.locationInfo.homeLocationInfo.IsNull())
    {
        // If HomeLocationInfo is not null, and its LocationName field is an empty string, at least one of the following SHALL NOT be null:
        // HomeLocationInfo's FloorNumber field, HomeLocationInfo's AreaType field, the LandmarkTag field

        // If all three of the following are null, HomeLocationInfo's LocationName field SHALL NOT be an empty string:
        // HomeLocationInfo's FloorNumber field HomeLocationInfo's AreaType field, the LandmarkTag field
        VerifyOrExit(  ((aLocation.locationInfo.homeLocationInfo.Value().locationName.size() != 0) ||
                        !aLocation.locationInfo.homeLocationInfo.Value().floorNumber.IsNull() || 
                        !aLocation.locationInfo.homeLocationInfo.Value().areaType.IsNull() ||
                        !aLocation.locationInfo.landmarkTag.IsNull()),
                    ChipLogError(Zcl,  "IsValidSupportedLocation %u - LocationName is empty string, Floornumber, AreaType, LandmarkTag are null", aLocation.locationId));
    }

    // If the LandmarkTag field is null, the PositionTag field SHALL be null.
    VerifyOrExit((!aLocation.locationInfo.landmarkTag.IsNull() || aLocation.locationInfo.positionTag.IsNull()),
                ChipLogError(Zcl,  "IsValidSupportedLocation %u - PositionTag with no LandmarkTag", aLocation.locationId));

    if (mSupportedMaps.size() == 0)
    {
        // If the SupportedMaps attribute is null, mapid SHALL be null.
        VerifyOrExit((aLocation.mapId.IsNull()),
                    ChipLogError(Zcl,  "IsValidSupportedLocation %u - map Id %u is not in empty supported map list ", aLocation.locationId, aLocation.mapId.Value()));
    }
    else
    {
        // If the SupportedMaps attribute is not null, mapId SHALL be the ID of an entry from the SupportedMaps attribute.
        VerifyOrExit((IsSupportedMap(aLocation.mapId.Value())),
                    ChipLogError(Zcl,  "IsValidSupportedLocation %u - map Id %u is not in supported map list ", aLocation.locationId, aLocation.mapId.Value()));
    }

    //success
    ret_value = true;


exit:
    return ret_value;
}


bool Instance::IsUniqueSupportedLocation(const LocationStructureWrapper & aLocation)
{
    bool ret_value = true;

    // If the SupportedMaps attribute is not null, each entry in this list SHALL have a unique value for the combination of the MapID and LocationInfo fields.
    // If the SupportedMaps attribute is null, each entry in this list SHALL have a unique value for the LocationInfo field.

    // validate that the location is unique with regard to the supported locations list
    for (auto & entry : mSupportedLocations)
    {
        // this function may be used for uniqueness checking of a location that is a member of supported locations, (for validating modifications)
        // so do not test it against itself.
        // skip location if locationId's match
        if (aLocation.locationId == entry.second.locationId)
        {
            continue;
        }

        // check for non-matching mapId
        // if mapId is not null, skip locations with null mapId's
        // if mapId is null, skip locations with non-null mapId's
        // if both location's mapId are not null, skip locations with non matching mapId's
        if ((aLocation.mapId.IsNull() != (entry.second.mapId.IsNull()))  ||

            (!aLocation.mapId.IsNull() && (!entry.second.mapId.IsNull()) && (aLocation.mapId.Value() != entry.second.mapId.Value())))
        {
            continue;
        }

        // check for null vs non-null HomeLocationInfo
        if (aLocation.locationInfo.homeLocationInfo.IsNull() != (entry.second.locationInfo.homeLocationInfo.IsNull()))
        {
             continue;
        }

        // if both locations have non-null HomeLocationInfo, check fields
        if (!aLocation.locationInfo.homeLocationInfo.IsNull() && (!entry.second.locationInfo.homeLocationInfo.IsNull()))
        {
            if (!aLocation.DoesNameMatch(entry.second.locationInfo.homeLocationInfo.Value().locationName))
            {
                continue;
            }


            // check for non-matching FloorNumber
            // if FloorNumber is not null, skip locations with null FloorNumber's
            // if FloorNumber is null, skip locations with non-null FloorNumber's
            // if both location's FloorNumber are not null, skip locations with non matching FloorNumber's
            if ((aLocation.locationInfo.homeLocationInfo.Value().floorNumber.IsNull() != entry.second.locationInfo.homeLocationInfo.Value().floorNumber.IsNull()) ||

                ((!aLocation.locationInfo.homeLocationInfo.Value().floorNumber.IsNull() && !entry.second.locationInfo.homeLocationInfo.Value().floorNumber.IsNull()) 
                    && (aLocation.locationInfo.homeLocationInfo.Value().floorNumber.Value() != entry.second.locationInfo.homeLocationInfo.Value().floorNumber.Value()))  )
            {
                continue;
            }


            // check for non-matching AreaType
            // if AreaType is not null, skip locations with null AreaType's
            // if AreaType is null, skip locations with non-null AreaType's
            // if both location's AreaType are not null, skip locations with non matching AreaType's
            if ((aLocation.locationInfo.homeLocationInfo.Value().areaType.IsNull() != entry.second.locationInfo.homeLocationInfo.Value().areaType.IsNull()) ||

                ((!aLocation.locationInfo.homeLocationInfo.Value().areaType.IsNull() && !entry.second.locationInfo.homeLocationInfo.Value().areaType.IsNull()) 
                    && (aLocation.locationInfo.homeLocationInfo.Value().areaType.Value() != entry.second.locationInfo.homeLocationInfo.Value().areaType.Value()))  )
            {
                continue;
            }
        }

        // check for non-matching landmarkTag
        // if landmarkTag is not null, skip locations with null landmarkTag's
        // if landmarkTag is null, skip locations with non-null landmarkTag's
        // if both location's landmarkTag are not null, skip locations with non matching landmarkTag's
        if ((aLocation.locationInfo.landmarkTag.IsNull() != entry.second.locationInfo.landmarkTag.IsNull()) ||

            ((!aLocation.locationInfo.landmarkTag.IsNull() && !entry.second.locationInfo.landmarkTag.IsNull()) 
                && (aLocation.locationInfo.landmarkTag.Value() != entry.second.locationInfo.landmarkTag.Value()))  )
        {
            continue;
        }

        // check for non-matching positionTag
        // if positionTag is not null, skip locations with null positionTag's
        // if positionTag is null, skip locations with non-null positionTag's
        // if both location's positionTag are not null, skip locations with non matching positionTag's
        if ((aLocation.locationInfo.positionTag.IsNull() != entry.second.locationInfo.positionTag.IsNull())  ||

            ((!aLocation.locationInfo.positionTag.IsNull() && !entry.second.locationInfo.positionTag.IsNull()  
                && (aLocation.locationInfo.positionTag.Value() != entry.second.locationInfo.positionTag.Value())))  )
        {
            continue;
        }

        // check for non-matching surfaceTag
        // if surfaceTag is not null, skip locations with null surfaceTag's
        // if surfaceTag is null, skip locations with non-null surfaceTag's
        // if both location's surfaceTag are not null, skip locations with non matching surfaceTag's
        if ((aLocation.locationInfo.surfaceTag.IsNull() != entry.second.locationInfo.surfaceTag.IsNull())  ||

            ((!aLocation.locationInfo.surfaceTag.IsNull() && !entry.second.locationInfo.surfaceTag.IsNull()  
                && (aLocation.locationInfo.surfaceTag.Value() != entry.second.locationInfo.surfaceTag.Value())))  )
        {
            continue;
        }


        // failure - not unique - no differences found between test location and this supported location entry
        ret_value = false;
        break;
    }


    return ret_value;
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

    // create location object for validation
    LocationStructureWrapper aNewLocation(  aLocationId, aMapId,
                                            aLocationName, aFloorNumber, aAreaType, 
                                            aLandmarkTag, aPositionTag, aSurfaceTag);

    // check max# of list entries
    VerifyOrExit((kMaxNumSupportedLocations > mSupportedLocations.size()),
                ChipLogError(Zcl,  "AddSupportedLocation %u - to many entries", aLocationId));


    // verify cluster requirements concerning valid fields and field relationships
    VerifyOrExit(IsValidSupportedLocation(aNewLocation), 
                ChipLogError(Zcl,  "AddSupportedLocation %u - not a valid location object", aNewLocation.locationId));

    // must not match existing location description
    VerifyOrExit(IsUniqueSupportedLocation(aNewLocation),
                ChipLogError(Zcl,  "AddSupportedLocation %u - not a unique location object", aNewLocation.locationId));


    {
        // validated - add to list
        auto result = mSupportedLocations.emplace( SupportedLocationPairType(aLocationId, aNewLocation));

        // Each entry in Supported Locations SHALL have a unique value for the ID field.
        // (successful insertion in map)
        VerifyOrExit((result.second),
            ChipLogError(Zcl,  "AddSupportedLocation - non-unique location Id %u", aLocationId));
    }

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

    // find existing supported location to modify
    auto locationIter = mSupportedLocations.find(aLocationId);
    
    // create location object for validation
    LocationStructureWrapper aNewLocation(  aLocationId, aMapId,
                                            aLocationName, aFloorNumber, aAreaType, 
                                            aLandmarkTag, aPositionTag, aSurfaceTag);

    VerifyOrExit((locationIter != mSupportedLocations.end()),
                ChipLogError(Zcl,  "ModifySupportedLocation %u - not a supported locationId", aNewLocation.locationId));

    // verify cluster requirements concerning valid fields and field relationships
    VerifyOrExit(IsValidSupportedLocation(aNewLocation), 
                ChipLogError(Zcl,  "ModifySupportedLocation %u - not a valid location object", aNewLocation.locationId));

    // updated location description must not match another existing location description
    VerifyOrExit(IsUniqueSupportedLocation(aNewLocation),
                ChipLogError(Zcl,  "ModifySupportedLocation %u - not a unique location object", aNewLocation.locationId));


    // success! replace the supported location with the modified location 
    locationIter->second = aNewLocation;
    ret_value = true;
    NotifySupportedLocationsChanged();

    // update other attributes as required by the supported location change
    HandleSupportedLocationModified(aLocationId); 


exit:
    return ret_value; 
}


void Instance::HandleSupportedLocationModified(uint32_t aLocationId)
{
    // If any entry in the SupportedLocations attribute list is updated
    // all references to it (items matching the LocationID field's value) are removed as follows:
    if ((!mCurrentLocation.IsNull()) &&
        (IsSupportedLocation(mCurrentLocation.Value())))
    {
        // The CurrentLocation attribute SHALL be set to null if matching the updated supported location.
        mCurrentLocation.SetNull();
        NotifyCurrentLocationChanged();
    }

    // The SelectedLocations attribute list SHALL be updated to remove any matching entry.
    if (IsSelectedLocation(aLocationId))
    {
        if (DeleteSelectedLocation(aLocationId))
        {
            std::string statusText;

            if (!mDelegate->IsSetSelectedLocationAllowed(statusText))
            {
                ChipLogError(Zcl,  "HandleSupportedLocationModified - modification of SupportedLocation %u resulted in disallowed SelectedLocations change - %s",
                                            aLocationId, statusText.c_str());
            }
           
            // If the SelectedLocations attribute list becomes empty, the server SHALL stop operating.
            // If selected locations is changed and the device is operating, the device may need to update it's operations.
            mDelegate->HandleSelectedLocationsChanged();
            NotifySelectedLocationsChanged();
        }
    }
    
    // The Progress attribute list SHALL be updated to remove any matching entry.
    if (DeleteProgressElement(aLocationId))
    {
        NotifyProgressChanged();
    }
}

bool Instance::PruneSupportedLocations()
{
    bool supportedLocationsChanged = false;
    bool currentLocationChanged    = false;
    bool selectedLocationsChanged  = false;
    bool progressChanged           = false;

    // carefully iterate through SupportedLocations while possibly deleting some of them
    auto locationIter = mSupportedLocations.begin();

    while (locationIter != mSupportedLocations.end())
    {
        // remove supported location if mapId is no longer valid
        if ((!locationIter->second.mapId.IsNull()) &&
            (!IsSupportedMap(locationIter->second.mapId.Value())))
        {
            supportedLocationsChanged = true; // deleted at least one supported location
            uint32_t aLocationId = locationIter->second.locationId; // grab the location id before deleting it

            // delete the entry, moves iterator to next in list
            locationIter = mSupportedLocations.erase(locationIter);
            
            // The CurrentLocation attribute SHALL be set to null if deleted from the supported locations
            if ((!mCurrentLocation.IsNull()) &&
                (IsSupportedLocation(mCurrentLocation.Value())))
            {
                mCurrentLocation.SetNull();
                currentLocationChanged = true;
            }

            // remove deleted location from selected locations
            if (DeleteSelectedLocation(aLocationId))
            {
                selectedLocationsChanged = true;
            }
            
            // remove location from  progress list
            if (DeleteProgressElement(aLocationId))
            {
                progressChanged = true;
                NotifyProgressChanged();
            }
        }
        else
        {
            // location is still good, not deleted, move on to next one
            ++locationIter;
        }

    } // end of while()


    // MATTER notifications for changed attributes
    if (supportedLocationsChanged)
    {
        NotifySupportedLocationsChanged();
    }

    if (currentLocationChanged)
    {
        NotifyCurrentLocationChanged();
    }

    if (selectedLocationsChanged)
    {
         std::string statusText;
        if (!mDelegate->IsSetSelectedLocationAllowed(statusText))
        {
            ChipLogError(Zcl,  "PruneSupportedLocations - deletion of SupportedLocation resulted in disallowed SelectedLocations change - %s",
                                        statusText.c_str());
        }

        // If the SelectedLocations attribute list becomes empty, the server SHALL stop operating.
        // If selected locations is changed and the device is operating, the device may need to update it's operations.
        mDelegate->HandleSelectedLocationsChanged();
        NotifySelectedLocationsChanged();
    }

    if (progressChanged)
    {
        NotifyProgressChanged();
    }

    return supportedLocationsChanged;
}

bool Instance::DeleteSupportedLocation(uint32_t aLocationId)
{
    return (mSupportedLocations.erase(aLocationId) > 0);
}

bool Instance::ClearSupportedLocationsList()
{
    if (mSupportedLocations.size() != 0)
    {
        mSupportedLocations.clear();
        NotifySupportedLocationsChanged();

        // When Supported Locations change, the following attributes SHALL be set to null: SelectedLocations, CurrentLocation, and Progress.
        if (ClearSelectedLocationsList())
        {
            std::string statusText;
            if (!mDelegate->IsSetSelectedLocationAllowed(statusText))
            {
                ChipLogError(Zcl,  "ClearSupportedLocationsList - deletion of SupportedLocation resulted in disallowed SelectedLocations change - %s",
                                             statusText.c_str());
            }

            // If the SelectedLocations attribute list becomes empty, the server SHALL stop operating.
            // If selected locations is changed and the device is operating, the device may need to update it's operations.
            mDelegate->HandleSelectedLocationsChanged();
            NotifySelectedLocationsChanged();
        }

        if (!GetCurrentLocation().IsNull())
        {
            SetCurrentLocation(DataModel::Nullable<uint32_t>());
            NotifyCurrentLocationChanged();
        }

        if (ClearProgressList())
        {
            NotifyProgressChanged();
        }

        return true;
    }

    return false;
}


//*************************************************************************
// Supported Maps manipulators

void  Instance::GetSupportedMapIds(std::vector<uint8_t> & aSupportedMapIds)
{
    aSupportedMapIds.reserve(mSupportedMaps.size());

    for (auto & entry : mSupportedMaps)
    {
        aSupportedMapIds.push_back(entry.second.mapId);
    }
}

bool Instance::GetSupportedMapNameById(uint8_t aMapId, CharSpan & aMapName)
{
    bool ret_value = false;

    auto mapIter = mSupportedMaps.find(aMapId);

    if (mapIter != mSupportedMaps.end())
    {
        aMapName = mapIter->second.name;
        ret_value = true;
    }

    return ret_value;
}

bool Instance::IsSupportedMap(uint8_t aMapId)
{
    auto mapIter = mSupportedMaps.find(aMapId);
    return (mapIter != mSupportedMaps.end());
}

bool Instance::AddSupportedMap(uint8_t aMapId, const CharSpan & aMapName)
{
    bool ret_value = false;

    // check max# of list entries
    VerifyOrExit((mSupportedMaps.size() < kMaxNumSupportedMaps), 
                ChipLogError(Zcl,  "AddSupportedMap %u - maximum number of entries", aMapId));

    //  Map name SHALL include readable text that describes the mapname (cannot be empty string)
    VerifyOrExit((aMapName.size() != 0),
                ChipLogError(Zcl,  "AddSupportedMap %u - Name must not be empty string", aMapId));

    {
         // Each entry in this list SHALL have a unique value for the Name field.
        for (auto & entry : mSupportedMaps)
        {
            // the name cannot be the same as another map
            VerifyOrExit(((aMapId != entry.second.mapId) || (!entry.second.DoesNameMatch(aMapName))),
                    ChipLogError(Zcl,  "AddSupportedMap %u - map already exists with mapId %u and name '%s'", 
                                aMapId, entry.second.mapId, entry.second.name_c_str()));
        }
    }

    {
        //  Each entry in this list SHALL have a unique value for the MapID field.
        auto result = mSupportedMaps.emplace(SupportedMapPairType(aMapId, SupportedMapType(aMapId, aMapName)));

        VerifyOrExit(result.second, ChipLogError(Zcl,  "AddSupportedMap - non-unique Id %u", aMapId));
    }


    // map successfully added
    ret_value = true;
    NotifySupportedMapsChanged();
    // note - no need to modifiy other lists when a map is added.


exit:
    return ret_value;
}


/**
 * @brief rename an existing map in the supported maps list
 * @param aMapId id of the map
 * @param aMapName new name of the map (cannot be empty string)
 * @return true if the new name passed validation checks and was successfully modified
 * @note if the specified map is not a member of the supported maps list, returns false with no action taken.
 * @note caller is responsible for change notification
 * 
 */ 
bool Instance::RenameSupportedMap(uint8_t aMapId, const CharSpan & aMapName)
{
    bool ret_value = false;

    // check that entry exists
    auto mapIter = mSupportedMaps.find(aMapId);

    VerifyOrExit((mapIter != mSupportedMaps.end()), 
                ChipLogError(Zcl,  "RenameSupportedMap Id %u - map does not exist", aMapId));

    //  Map name SHALL include readable text that describes the mapname (cannot be empty string)
    VerifyOrExit((aMapName.size() != 0),
                ChipLogError(Zcl,  "RenameSupportedMap %u - Name must not be empty string", aMapId));

    {
        // Each entry in this list SHALL have a unique value for the Name field.
        for (auto & entry : mSupportedMaps)
        {
            // don't compare with it's own entry
            if (entry.second.mapId == aMapId)
            {
                continue;
            }

           
            // (making use of MapStructureWrapper::DoesNameMatch())
            VerifyOrExit(((aMapId != entry.second.mapId) || (!entry.second.DoesNameMatch(aMapName))),
                    ChipLogError(Zcl,  "AddSupportedMap %u - map already exists with mapId %u and name '%s'", 
                                aMapId, entry.second.mapId, entry.second.name_c_str()));
        }
    }


    // successfully rename map
    mapIter->second.Set(aMapId, aMapName);   // making use of MapStructureWrapper::Set()
    ret_value = true;
    NotifySupportedMapsChanged();
    // note - no need to modifiy other lists when a map is renamed


exit:
    return ret_value;
}
    

bool Instance::DeleteSupportedMap(uint8_t aMapId)
{
    bool ret_value = false;

    if (mSupportedMaps.erase(aMapId) > 0)
    {
        ret_value = true;
        mSupportedMaps.erase(aMapId);
        NotifySupportedMapsChanged();

        // If any entry in the SupportedMaps attribute list is deleted, the entries in the SupportedLocations 
        // attribute list with the MapID field matching the ID of the deleted map SHALL be deleted.
        // (as necessary update SupportedLocations, SelectedLocations, CurrentLocation, Progress)
        PruneSupportedLocations();
    }

    return ret_value;
}

bool Instance::ClearSupportedMapsList()
{
    bool ret_value = false;

    if (mSupportedMaps.size() != 0)
    {
        ret_value = true;
        mSupportedMaps.clear();
        NotifySupportedMapsChanged();

        // clear lists of any locations that depended on mapId's
        // (as necessary update SupportedLocations, SelectedLocations, CurrentLocation, Progress)
        PruneSupportedLocations();
    }

    return ret_value;
}


//*************************************************************************
// Selected Locations manipulators

void Instance::GetSelectedLocations(std::vector<uint32_t> & aSelectedLocations)
{
    aSelectedLocations.clear();
    aSelectedLocations.reserve(mSelectedLocations.size());

    for (auto & entry : mSelectedLocations)
    {
        aSelectedLocations.push_back(entry);
    }
}

bool Instance::IsSelectedLocation(uint32_t aLocationId)
{
    bool ret_value = false;

    for (uint32_t & entry : mSelectedLocations)
    {
        if (entry == aLocationId)
        {
            ret_value = true;
        }
    }

    return ret_value;
}

bool Instance::AddSelectedLocation(uint32_t & aSelectedLocation)
{
    bool ret_value;
    std::string locationStatusText;

    // check max# of list entries
    VerifyOrExit((mSelectedLocations.size() < kMaxNumSelectedLocations), 
                    ChipLogError(Zcl,  "AddSelectedLocation %u - maximum number of entries", aSelectedLocation));

    // each item in this list SHALL match the LocationID field of an entry on the SupportedLocations attribute's list
    VerifyOrExit(IsSupportedLocation(aSelectedLocation), 
                    ChipLogError(Zcl, "AddSelectedLocation - unsupported location %u", aSelectedLocation));

    // each entry in this list SHALL have a unique value
    VerifyOrExit(!IsSelectedLocation(aSelectedLocation), 
                    ChipLogError(Zcl, "AddSelectedLocation %u - duplicated location", aSelectedLocation));

    // Does device mode allow modification of selected locations?
    VerifyOrExit(mDelegate->IsSetSelectedLocationAllowed(locationStatusText), 
                    ChipLogError(Zcl, "AddSelectedLocation %u - %s", aSelectedLocation, locationStatusText.c_str()));


    // update selected location list with the new value
    ret_value = true;
    mSelectedLocations.push_back(aSelectedLocation);


exit:
    return ret_value;
}

bool Instance::PruneSelectedLocations()
{
    bool ret_value = false;

    // carefully iterate through SelectedLocations while possibly deleting some of them
    auto locationIter = mSelectedLocations.begin();

    while (locationIter != mSelectedLocations.end())
    {
        if (IsSupportedLocation(*locationIter))
        {
            // delete location, move to next
            ret_value = true;
            locationIter = mSelectedLocations.erase(locationIter);
        }
        else
        {
            ++ locationIter; //next location
        }
    }

    return ret_value;
}

bool Instance::DeleteSelectedLocation(uint32_t aLocationId)
{
    bool ret_value = false;

    auto locationIter = mSelectedLocations.begin();

    while (locationIter != mSelectedLocations.end())
    {
        if (*locationIter == aLocationId)
        {
            // delete location
            ret_value = true;
            locationIter = mSelectedLocations.erase(locationIter);
            break;
        }
    }

    return ret_value;
}

bool Instance::ClearSelectedLocationsList()
{
    bool ret_value = false;

    if (mSelectedLocations.size() != 0)
    {
        ret_value = true;
        mSelectedLocations.clear();

    }

    return ret_value;
}


//*************************************************************************
// Current Location manipulators

DataModel::Nullable<uint32_t> Instance::GetCurrentLocation()
{
    // give the device a chance to make sure value is up-to-date
    mDelegate->HandleVolatileCurrentLocation(mCurrentLocation); 
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
    // give the device a chance to make sure value is up-to-date
    mDelegate->HandleVolatileEstimatedEndTime(mEstimatedEndTime);
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

void Instance::GetProgressElementIds(std::vector<uint32_t> & aProgressIds)
{
    aProgressIds.clear();
    aProgressIds.reserve(mProgressList.size());

    for (auto & entry : mProgressList)
    {
        aProgressIds.push_back(entry.first);
    }
}

bool Instance::GetProgressElementById(uint32_t aLocationId, const Structs::ProgressStruct::Type*& aProgressElement)
{
    bool ret_value = false;

    auto progressIter = mProgressList.find(aLocationId);

    if (progressIter != mProgressList.end())
    {
        aProgressElement = &progressIter->second;
        ret_value = true;
    }

    return ret_value;
}

bool Instance::IsProgressElement(uint32_t aLocationId)
{
    bool ret_value = false;

    auto progressIter = mProgressList.find(aLocationId);

    if (progressIter != mProgressList.end())
    {
        ret_value = true;
    }

    return ret_value;
}

bool Instance::AddPendingProgressElement(uint32_t aLocationId)
{
    bool ret_value = false;
    ProgressType inactiveProgress = { aLocationId, OperationalStatusEnum::kPending};

    // check max# of list entries
    VerifyOrExit((mProgressList.size() < kMaxNumSupportedProgress),  // Note: progress elements must uniquely map to supported locations, so max size is the same
                ChipLogError(Zcl,  "AddPendingProgressElement - maximum number of entries"));

    // For each entry in this list, the LocationID field SHALL match an entry on the SupportedLocations attribute's list.
    VerifyOrExit(IsSupportedLocation(aLocationId),
                ChipLogError(Zcl,  "AddPendingProgressElement - not a supported location %u", aLocationId));

    {
        // add progress element to list
        auto result = mProgressList.emplace(ProgressPairType(aLocationId, inactiveProgress));

        // Each entry in this list SHALL have a unique value for the LocationID field.
        // emplace failed, Id already exists
        VerifyOrExit(result.second,
                    ChipLogError(Zcl,  "AddPendingProgressElement - progress element already exists for location %u", aLocationId));
    }

    // success
    ret_value = true;


exit:
    return ret_value;
}

bool Instance::SetProgressStatus(uint32_t aLocationId, OperationalStatusEnum opStatus)
{
    bool ret_value = false;
    OperationalStatusEnum oldOpStatus;

    auto entry = mProgressList.find(aLocationId);

    VerifyOrExit((entry != mProgressList.end()),
                ChipLogError(Zcl,  "SetProgressStatus - progress element does not existsfor location %u", aLocationId));


    VerifyOrExit((opStatus < OperationalStatusEnum::kUnknownEnumValue),
                ChipLogError(Zcl,  "SetProgressStatus - unknown opStatus value %u", to_underlying(opStatus)));
    

    // set the progress status
    oldOpStatus = OperationalStatusEnum(entry->second.status);
    entry->second.status = opStatus;
    ret_value = true;

    // TotalOperationalTime SHALL be null if the Status field is not set to Completed or Skipped.
    if ((opStatus != OperationalStatusEnum::kCompleted) &&
        (opStatus != OperationalStatusEnum::kSkipped))
    {
        entry->second.totalOperationalTime.Value().SetNull();
    }

    if (oldOpStatus != OperationalStatusEnum(entry->second.status))
    {
        NotifyProgressChanged();
    }


exit:
    return ret_value;
}

bool Instance::SetProgressTotalOperationalTime(uint32_t aLocationId, const DataModel::Nullable<uint32_t> & aTotalOperationalTime)
{
    bool ret_value = false;
    auto entry = mProgressList.find(aLocationId);

    VerifyOrExit((entry != mProgressList.end()),
            ChipLogError(Zcl,  "SetProgressTotalOperationalTime - no progress element for location %u", aLocationId));

    // This attribute SHALL be null if the Status field is not set to Completed or Skipped
    VerifyOrExit((aTotalOperationalTime.IsNull() || (entry->second.status == OperationalStatusEnum::kCompleted) || (entry->second.status == OperationalStatusEnum::kSkipped)),
            ChipLogError(Zcl,  "SetProgressTotalOperationalTime - location %u opStatus value %u - can be non-null only if opStatus is Completed or Skipped",
                                aLocationId, to_underlying(entry->second.status)));


    // success
    entry->second.totalOperationalTime.SetValue(aTotalOperationalTime);
    ret_value = true;


exit:
    return ret_value;
}

bool Instance::SetProgressEstimatedTime(uint32_t aLocationId, const DataModel::Nullable<uint32_t> & aEstimatedTime)
{
    bool ret_value = false;

    auto entry = mProgressList.find(aLocationId);

    VerifyOrExit((entry != mProgressList.end()),
            ChipLogError(Zcl,  "SetProgressTotalOperationalTime - no progress element for location %u", aLocationId));

    entry->second.estimatedTime.SetValue(aEstimatedTime);
    ret_value = true;

exit:
    return ret_value;
}

bool Instance::PruneProgressElements()
{
    bool ret_value = false;

    // carefully iterate through ProgressList elements while possibly deleting some of them
    auto progressIter = mProgressList.begin();

    while (progressIter != mProgressList.end())
    {
        if (IsSupportedLocation(progressIter->first))
        {
            // delete location, move to next
            ret_value = true;
            progressIter = mProgressList.erase(progressIter);
        }
        else
        {
            ++progressIter; //next location
        }
    }

    return ret_value;
}

bool Instance::DeleteProgressElement(uint32_t aLocationId)
{
    return (mProgressList.erase(aLocationId) > 0);
}

bool Instance::ClearProgressList()
{
    if (mProgressList.size() != 0)
    {
        mProgressList.clear();
        return true;
    }

    return false;
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

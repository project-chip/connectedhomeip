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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <rvc-service-area-delegate.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ServiceArea;

CHIP_ERROR RvcServiceAreaDelegate::Init()
{
    // hardcoded fill of SUPPORTED MAPS for prototyping
    uint8_t supportedMapId_XX  = 3;
    uint8_t supportedMapId_YY  = 245;
    
    GetInstance()->AddSupportedMap(supportedMapId_XX, CharSpan::fromCharString("My Map XX"));
    GetInstance()->AddSupportedMap(supportedMapId_YY, CharSpan::fromCharString("My Map YY"));


    // hardcoded fill of SUPPORTED LOCATIONS for prototyping
    uint32_t supportedLocationId_A  = 7;
    uint32_t supportedLocationId_B  = 1234567;
    uint32_t supportedLocationId_C  = 10050;
    uint32_t supportedLocationId_D  = 0x88888888;


    // Location A has name, floor number, uses map XX
    GetInstance()->AddSupportedLocation(  supportedLocationId_A, 
                                              DataModel::Nullable<uint_fast8_t>(supportedMapId_XX),
                                              CharSpan::fromCharString("My Location A"),
                                              DataModel::Nullable<int16_t>(4),
                                              DataModel::Nullable<ServiceArea::AreaTypeTag>(),
                                              DataModel::Nullable<ServiceArea::LandmarkTag>(),
                                              DataModel::Nullable<ServiceArea::PositionTag>(),
                                              DataModel::Nullable<ServiceArea::FloorSurfaceTag>() );


    // Location B has name, uses map XX
    GetInstance()->AddSupportedLocation(  supportedLocationId_B, 
                                              DataModel::Nullable<uint_fast8_t>(supportedMapId_XX),
                                              CharSpan::fromCharString("My Location B"),
                                              DataModel::Nullable<int16_t>(),
                                              DataModel::Nullable<ServiceArea::AreaTypeTag>(),
                                              DataModel::Nullable<ServiceArea::LandmarkTag>(),
                                              DataModel::Nullable<ServiceArea::PositionTag>(),
                                              DataModel::Nullable<ServiceArea::FloorSurfaceTag>() );




    // Location C has full SemData, no name, Map YY
    GetInstance()->AddSupportedLocation(  supportedLocationId_C, 
                                              DataModel::Nullable<uint_fast8_t>(supportedMapId_YY),
                                              CharSpan(),
                                              DataModel::Nullable<int16_t>(-1),
                                              DataModel::Nullable<ServiceArea::AreaTypeTag>(ServiceArea::AreaTypeTag::kPlayRoom),
                                              DataModel::Nullable<ServiceArea::LandmarkTag>(ServiceArea::LandmarkTag::kBackDoor),
                                              DataModel::Nullable<ServiceArea::PositionTag>(ServiceArea::PositionTag::kNextTo),
                                              DataModel::Nullable<ServiceArea::FloorSurfaceTag>(ServiceArea::FloorSurfaceTag::kConcrete) );

    // Location D has null values for all HomeLocationStruct fields, Map YY
    GetInstance()->AddSupportedLocation(   supportedLocationId_D, 
                                              DataModel::Nullable<uint_fast8_t>(supportedMapId_YY),
                                              CharSpan::fromCharString("My Location D"),
                                              DataModel::Nullable<int16_t>(),
                                              DataModel::Nullable<ServiceArea::AreaTypeTag>(),
                                              DataModel::Nullable<ServiceArea::LandmarkTag>(ServiceArea::LandmarkTag::kCouch),
                                              DataModel::Nullable<ServiceArea::PositionTag>(ServiceArea::PositionTag::kNextTo),
                                              DataModel::Nullable<ServiceArea::FloorSurfaceTag>(ServiceArea::FloorSurfaceTag::kHardwood) );


    GetInstance()->SetCurrentLocation(supportedLocationId_C);

    return CHIP_NO_ERROR;
}


//*************************************************************************
// command support

bool RvcServiceAreaDelegate::IsSetSelectedLocationsAllowed(char* statusText)
{
    return true; // TODO IMPLEMENT
};

bool RvcServiceAreaDelegate::IsValidSelectLocationsSet(const Commands::SelectLocations::DecodableType & req,
                                          SelectLocationsStatus & locationStatus, char* statusText, bool & useStatusText)
{
    bool ret_value = false;

    ret_value = true; // TODO IMPLEMENT

    return ret_value;
};

bool RvcServiceAreaDelegate::HandleSkipCurrentLocation(char* skipStatusText)
{
    bool ret_value = false;

    ret_value = true; // TODO IMPLEMENT

    return ret_value;
};


//*************************************************************************
// Supported Locations accessors

bool RvcServiceAreaDelegate::IsSupportedLocationsChangeAllowed()
{
    return true; // TODO
}

uint32_t RvcServiceAreaDelegate::GetNumberOfSupportedLocations() 
{
    return mSupportedLocations.size();
}

bool RvcServiceAreaDelegate::GetSupportedLocationByIndex(uint32_t listIndex, LocationStructureWrapper & aSupportedLocation) 
{
    bool ret_value = false;

    if (listIndex < mSupportedLocations.size())
    {
        aSupportedLocation = mSupportedLocations[listIndex];
        ret_value = true;
    }

    return ret_value;
};

bool RvcServiceAreaDelegate::GetSupportedLocationById(uint32_t aLocationId, uint32_t & listIndex, LocationStructureWrapper & aSupportedLocation)
{
    // We do not need to reimplement this method as it's already done by the SDK.
    // We are reimplementing this method, still using linear search, but with some optimization on the SDK implementation
    // since we have direct access to the list.
    bool ret_value = false;

    listIndex = 0;
    uint32_t listSize = mSupportedLocations.size();

    while (listIndex < listSize)
    {
        if (mSupportedLocations[listIndex].locationID == aLocationId)
        {
            aSupportedLocation = mSupportedLocations[listIndex];
            ret_value = true;
            break;
        }

        ++listIndex;
    }

    return ret_value;
};

bool RvcServiceAreaDelegate::AddSupportedLocation(const LocationStructureWrapper & newLocation, uint32_t & listIndex)
{
    bool ret_value = false;

    // The server instance (caller) is responsible for ensuring that there are no duplicate location IDs, list size not exceeded, etc.

    // Double-check list size to ensure there no memory issues.
    if (mSupportedLocations.size() < kMaxNumSupportedLocations)
    {
        // not sorting list, number of locations normally expected to be small, max 255
        mSupportedLocations.push_back(newLocation);
        listIndex = mSupportedMaps.size() - 1; // new element is last in list
        ret_value = true;
    }
    else
    {
        ChipLogError(Zcl,  "AddSupportedLocation %u - supported locations list is already at maximum size %u",
                     newLocation.locationID, (uint32_t)kMaxNumSupportedLocations);
    }

    return ret_value;
}

bool RvcServiceAreaDelegate::ModifySupportedLocation(uint32_t listIndex, const LocationStructureWrapper & modifiedLocation)
{
    bool ret_value = false;

    // The server instance (caller) is responsible for ensuring that there are no duplicate location IDs, list size not exceeded, etc.

    // Double-check that locationID's match.
    VerifyOrExit((modifiedLocation.locationID == mSupportedLocations[listIndex].locationID), 
                 ChipLogError(Zcl,  "ModifySupportedLocation - new locationID %u does not match existing locationID %u", 
                              modifiedLocation.locationID, mSupportedLocations[listIndex].locationID));   

    // checks passed, update the attribute
    mSupportedLocations[listIndex] = modifiedLocation;
    ret_value = true;

exit:

    return ret_value;
}

bool RvcServiceAreaDelegate::ClearSupportedLocations()
{
    bool ret_value = false;

    if (mSupportedLocations.size() > 0)
    {
        mSupportedLocations.clear();
        ret_value = true;
    }

    return ret_value;
}


//*************************************************************************
// Supported Maps accessors

bool RvcServiceAreaDelegate::IsSupportedMapChangeAllowed()
{
    return true; // TODO
}

uint32_t RvcServiceAreaDelegate::GetNumberOfSupportedMaps() 
{
    return mSupportedMaps.size();
}

bool RvcServiceAreaDelegate::GetSupportedMapByIndex(uint32_t listIndex, MapStructureWrapper & aSupportedMap) 
{
    bool ret_value = false;

    if (listIndex < mSupportedMaps.size())
    {
        aSupportedMap = mSupportedMaps[listIndex];
        ret_value = true;
    }

    return ret_value;
};

bool RvcServiceAreaDelegate::GetSupportedMapById(uint8_t aMapId, uint32_t & listIndex, MapStructureWrapper & aSupportedMap)
{
    // We do not need to reimplement this method as it's already done by the SDK.
    // We are reimplementing this method, still using linear search, but with some optimization on the SDK implementation
    // since we have direct access to the list.
    bool ret_value = false;

    listIndex = 0;
    uint32_t listSize = mSupportedMaps.size();

    while (listIndex < listSize)
    {
        if (mSupportedMaps[listIndex].mapID == aMapId)
        {
            aSupportedMap = mSupportedMaps[listIndex];
            ret_value = true;
            break;
        }

        ++listIndex;
    }

    return ret_value;
};

bool RvcServiceAreaDelegate::AddSupportedMap(const MapStructureWrapper & newMap, uint32_t & listIndex)
{
    bool ret_value = false;

    // The server instance (caller) is responsible for ensuring that there are no duplicate location IDs, list size not exceeded, etc.

    // Double-check list size to ensure there no memory issues.
    if (mSupportedMaps.size() < kMaxNumSupportedMaps)
    {
        // not sorting list, number of locations normally expected to be small, max 255
        mSupportedMaps.push_back(newMap);
        listIndex = mSupportedMaps.size() - 1; // new element is last in list
    }
    else
    {
        ChipLogError(Zcl,  "AddSupportedMap %u - supported maps list is already at maximum size %u",
                     newMap.mapID, (uint32_t)kMaxNumSupportedMaps);
    }

    return ret_value;
}

bool RvcServiceAreaDelegate::ModifySupportedMap(uint32_t listIndex, const MapStructureWrapper & modifiedMap)
{
    bool ret_value = false;

    // The server instance (caller) is responsible for ensuring that there are no duplicate location IDs, list size not exceeded, etc.

    // Double-check that mapID's match.
    VerifyOrExit((modifiedMap.mapID == mSupportedMaps[listIndex].mapID),
                 ChipLogError(Zcl,  "ModifySupportedMap - mapID's do not match, new mapID %u, existing mapID %u",
                              modifiedMap.mapID, mSupportedMaps[listIndex].mapID));   

    // save modified map
    mSupportedMaps[listIndex] = modifiedMap;
    ret_value = true;

exit:
    return ret_value;
}

bool RvcServiceAreaDelegate::ClearSupportedMaps()
{
    bool ret_value = false;

    if (mSupportedMaps.size() > 0)
    {
        mSupportedMaps.clear();
        ret_value = true;
    }

    return ret_value;
}


//*************************************************************************
// Selected Locations accessors

uint32_t RvcServiceAreaDelegate::GetNumberOfSelectedLocations()
{
    return mSelectedLocations.size();
}

bool RvcServiceAreaDelegate::GetSelectedLocationByIndex(uint32_t listIndex, uint32_t & aSelectedLocation)
{
    bool ret_value = false;

    if (listIndex < mSelectedLocations.size())
    {
        aSelectedLocation = mSelectedLocations[listIndex];
        ret_value = true;
    }

    return ret_value;
};

bool RvcServiceAreaDelegate::AddSelectedLocation(uint32_t aLocationId, uint32_t & listIndex)
{
    bool ret_value = false;

    // The server instance (caller) is responsible for ensuring that there are no duplicate location IDs, list size not exceeded, etc.

    // Double-check list size to ensure there no memory issues.
    if (mSelectedLocations.size() < kMaxNumSelectedLocations)
    {
        // not sorting list, number of locations normally expected to be small, max 255
        mSelectedLocations.push_back(aLocationId);
        listIndex = mSelectedLocations.size() - 1; // new element is last in list
    }
    else
    {
        ChipLogError(Zcl,  "AddSelectedLocation %u - selected locations list is already at maximum size %u",
                     aLocationId, (uint32_t)kMaxNumSelectedLocations);
    }

    return ret_value;
}

bool RvcServiceAreaDelegate::ClearSelectedLocations()
{
    bool ret_value = false;

    if (mSelectedLocations.size() > 0)
    {
        mSelectedLocations.clear();
        ret_value = true;
    }

    return ret_value;
}


//*************************************************************************
// Progress List accessors

uint32_t RvcServiceAreaDelegate::GetNumberOfProgressElements() 
{
    return mProgressList.size();
}

bool RvcServiceAreaDelegate::GetProgressElementByIndex(uint32_t listIndex,  Structs::ProgressStruct::Type & aProgressElement) 
{
    bool ret_value = false;

    if (listIndex < mProgressList.size())
    {
        aProgressElement = mProgressList[listIndex];
        ret_value = true;
    }

    return ret_value;
};

bool RvcServiceAreaDelegate::GetProgressElementById(uint32_t aLocationId, uint32_t & listIndex, Structs::ProgressStruct::Type & aProgressElement)
{
    // We do not need to reimplement this method as it's already done by the SDK.
    // We are reimplementing this method, still using linear search, but with some optimization on the SDK implementation
    // since we have direct access to the list.
    bool ret_value = false;

    listIndex = 0;
    uint32_t listSize = mProgressList.size();

    while (listIndex < listSize)
    {
        if (mProgressList[listIndex].locationID == aLocationId)
        {
            aProgressElement = mProgressList[listIndex];
            ret_value = true;
            break;
        }

        ++listIndex;
    }

    return ret_value;
};

bool RvcServiceAreaDelegate::AddProgressElement(const Structs::ProgressStruct::Type & newProgressElement, uint32_t & listIndex)
{
    bool ret_value = false;

    // The server instance (caller) is responsible for ensuring that there are no duplicate location IDs, list size not exceeded, etc.

    // Double-check list size to ensure there no memory issues.
    if (mProgressList.size() < kMaxNumProgressElements)
    {
        // not sorting list, number of locations normally expected to be small, max 255
        mProgressList.push_back(newProgressElement);
        listIndex = mProgressList.size() - 1; // new element is last in list
    }
    else
    {
        ChipLogError(Zcl,  "AddProgressElement %u -progress list is already at maximum size %u", 
                     newProgressElement.locationID, (uint32_t)kMaxNumProgressElements);
    }

    return ret_value;
}

bool RvcServiceAreaDelegate::ModifyProgressElement(uint32_t listIndex, const Structs::ProgressStruct::Type & modifiedProgressElement) 
{
    bool ret_value = false;

    // TODO

    return ret_value; 
}

bool RvcServiceAreaDelegate::ClearProgress()
{
    bool ret_value = false;

    if (mProgressList.size() > 0)
    {
        mProgressList.clear();
        ret_value = true;
    }

    return ret_value;
}

#include "service-area-delegate.h"
#include "service-area-server.h"

using namespace chip::app::Clusters::ServiceArea;


bool Delegate::GetSupportedLocationById(uint32_t aLocationId, uint32_t & listIndex, LocationStructureWrapper & aSupportedLocation)
{
    bool ret_value = false;

    listIndex = 0;

    // simple linear iteration to find the location with the desired locationID.
    while (GetSupportedLocationByIndex(listIndex, aSupportedLocation))
    {
        if (aSupportedLocation.locationID == aLocationId)
        {
            ret_value = true;
            break;
        }

        ++listIndex;
    }

    return ret_value;
};

/**
 * When we cannot guarantee that the restrictions imposed on the SupportedLocations, CurrentLocation and Progress attributes
 * can be upheld, these attributes are set to null.
 *
 * The user is free the redefine this method as their device may have more information on what has changed and may be able to
 * maintain the restrictions on these attributes by selectively editing them.
 */
void Delegate::HandleSupportedLocationsUpdated()  {
    mInstance->ClearSelectedLocations();
    mInstance->SetCurrentLocation(DataModel::Nullable<uint32_t>());
    mInstance->ClearProgress();
};

bool Delegate::GetSupportedMapById(uint8_t aMapId, uint32_t & listIndex, MapStructureWrapper & aSupportedMap)
{
    bool ret_value = false;

    listIndex = 0;

    while (GetSupportedMapByIndex(listIndex, aSupportedMap))
    {
        if (aSupportedMap.mapID == aMapId)
        {
            ret_value = true;
            break;
        }

        ++listIndex;
    }

    return ret_value;
};

bool Delegate::IsSelectedLocation(uint32_t aLocationId)
{
    bool ret_value = false;

    uint32_t listIndex = 0;
    uint32_t selectedLocation;

    while (GetSelectedLocationByIndex(listIndex, selectedLocation))
    {
        if (selectedLocation == aLocationId)
        {
            ret_value = true;
            break;
        }

        ++listIndex;
    }

    return ret_value;
};

bool Delegate::GetProgressElementById(uint32_t aLocationId, uint32_t & listIndex, Structs::ProgressStruct::Type & aProgressElement)
{
    bool ret_value = false;

    listIndex = 0;

    // simple linear iteration to find the progress element with the desired locationID.
    while (GetProgressElementByIndex(listIndex, aProgressElement))
    {
        if (aProgressElement.locationID == aLocationId)
        {
            ret_value = true;
            break;
        }

        ++listIndex;
    }

    return ret_value;
};

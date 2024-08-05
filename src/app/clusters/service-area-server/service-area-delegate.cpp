#include "service-area-delegate.h"
#include "service-area-server.h"

using namespace chip::app::Clusters::ServiceArea;

bool Delegate::GetSupportedLocationById(uint32_t aAreaId, uint32_t & listIndex, AreaStructureWrapper & aSupportedLocation)
{
    listIndex = 0;

    // simple linear iteration to find the location with the desired areaId.
    while (GetSupportedLocationByIndex(listIndex, aSupportedLocation))
    {
        if (aSupportedLocation.areaID == aAreaId)
        {
            return true;
        }

        ++listIndex;
    }

    return false;
}

void Delegate::HandleSupportedAreasUpdated()
{
    mInstance->ClearSelectedAreas();
    mInstance->SetCurrentArea(DataModel::NullNullable);
    mInstance->ClearProgress();
}

bool Delegate::GetSupportedMapById(uint8_t aMapId, uint32_t & listIndex, MapStructureWrapper & aSupportedMap)
{
    listIndex = 0;

    while (GetSupportedMapByIndex(listIndex, aSupportedMap))
    {
        if (aSupportedMap.mapID == aMapId)
        {
            return true;
        }

        ++listIndex;
    }

    return false;
}

bool Delegate::IsSelectedLocation(uint32_t aAreaId)
{
    uint32_t listIndex = 0;
    uint32_t selectedLocation;

    while (GetSelectedLocationByIndex(listIndex, selectedLocation))
    {
        if (selectedLocation == aAreaId)
        {
            return true;
        }

        ++listIndex;
    }

    return false;
}

bool Delegate::GetProgressElementById(uint32_t aAreaId, uint32_t & listIndex, Structs::ProgressStruct::Type & aProgressElement)
{
    listIndex = 0;

    // simple linear iteration to find the progress element with the desired areaID.
    while (GetProgressElementByIndex(listIndex, aProgressElement))
    {
        if (aProgressElement.areaID == aAreaId)
        {
            return true;
        }

        ++listIndex;
    }

    return false;
}

bool Delegate::IsProgressElement(uint32_t aAreaId)
{
    uint32_t index;
    Structs::ProgressStruct::Type progressElement;

    return GetProgressElementById(aAreaId, index, progressElement);
}

#include "service-area-delegate.h"
#include "service-area-server.h"

using namespace chip::app::Clusters::ServiceArea;

bool Delegate::GetSupportedAreaById(uint32_t aAreaId, uint32_t & listIndex, AreaStructureWrapper & aSupportedArea)
{
    listIndex = 0;

    // simple linear iteration to find the area with the desired areaId.
    while (GetSupportedAreaByIndex(listIndex, aSupportedArea))
    {
        if (aSupportedArea.areaID == aAreaId)
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

bool Delegate::GetSupportedMapById(uint32_t aMapId, uint32_t & listIndex, MapStructureWrapper & aSupportedMap)
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

bool Delegate::IsSelectedArea(uint32_t aAreaId)
{
    uint32_t listIndex = 0;
    uint32_t selectedArea;

    while (GetSelectedAreaByIndex(listIndex, selectedArea))
    {
        if (selectedArea == aAreaId)
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

// todo: Should we add default implementations for the accessor methods of the non-mandatory attributes?
//  This is so that devices that do not support these attributes are not forced to provide an implementation.

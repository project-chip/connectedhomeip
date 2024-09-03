#include "service-area-storage-delegate.h"

using namespace chip::app::Clusters::ServiceArea;

bool StorageDelegate::GetSupportedAreaById(uint32_t aAreaId, uint32_t & listIndex, AreaStructureWrapper & aSupportedArea)
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

bool StorageDelegate::IsSupportedArea(uint32_t aAreaId)
{
    uint32_t ignoredIndex;
    AreaStructureWrapper ignoredArea;

    return GetSupportedAreaById(aAreaId, ignoredIndex, ignoredArea);
}

bool StorageDelegate::RemoveSupportedAreaRaw(uint32_t areaId)
{
    return false;
}

bool StorageDelegate::GetSupportedMapById(uint32_t aMapId, uint32_t & listIndex, MapStructureWrapper & aSupportedMap)
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

bool StorageDelegate::IsSupportedMap(uint32_t aMapId)
{
    uint32_t ignoredIndex;
    MapStructureWrapper ignoredMap;

    return GetSupportedMapById(aMapId, ignoredIndex, ignoredMap);
}

bool StorageDelegate::RemoveSupportedMapRaw(uint32_t mapId)
{
    return false;
}

bool StorageDelegate::IsSelectedArea(uint32_t aAreaId)
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

bool StorageDelegate::GetProgressElementById(uint32_t aAreaId, uint32_t & listIndex,
                                             Structs::ProgressStruct::Type & aProgressElement)
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

bool StorageDelegate::IsProgressElement(uint32_t aAreaId)
{
    uint32_t index;
    Structs::ProgressStruct::Type progressElement;

    return GetProgressElementById(aAreaId, index, progressElement);
}

// todo: Should we add default implementations for the accessor methods of the optional attributes?
//  This is so that devices that do not support these attributes are not forced to provide an implementation.

/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "LoggingServiceAreaStorageDelegate.h"
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app::Clusters::ServiceArea {

//*************************************************************************
// Supported Areas accessors

uint32_t LoggingServiceAreaStorageDelegate::GetNumberOfSupportedAreas()
{
    return static_cast<uint32_t>(mSupportedAreas.size());
}

bool LoggingServiceAreaStorageDelegate::GetSupportedAreaByIndex(uint32_t listIndex, AreaStructureWrapper & supportedArea)
{
    if (listIndex < mSupportedAreas.size())
    {
        supportedArea = mSupportedAreas[listIndex];
        return true;
    }

    return false;
}

bool LoggingServiceAreaStorageDelegate::GetSupportedAreaById(uint32_t aAreaId, uint32_t & listIndex,
                                                             AreaStructureWrapper & supportedArea)
{
    listIndex = 0;

    while (listIndex < mSupportedAreas.size())
    {
        if (mSupportedAreas[listIndex].areaID == aAreaId)
        {
            supportedArea = mSupportedAreas[listIndex];
            return true;
        }

        ++listIndex;
    }

    return false;
}

bool LoggingServiceAreaStorageDelegate::AddSupportedAreaRaw(const AreaStructureWrapper & newArea, uint32_t & listIndex)
{
    // The server instance (caller) is responsible for ensuring that there are no duplicate area IDs, list size not exceeded,
    // etc.
    if (mSupportedAreas.size() < kMaxNumSupportedAreas)
    {
        mSupportedAreas.push_back(newArea);
        listIndex = static_cast<uint32_t>(mSupportedAreas.size()) - 1; // new element is last in list
        return true;
    }

    ChipLogError(Zcl, "AddSupportedAreaRaw %u - supported areas list is already at maximum size %u",
                 static_cast<unsigned>(newArea.areaID), static_cast<unsigned>(kMaxNumSupportedAreas));

    return false;
}

bool LoggingServiceAreaStorageDelegate::ModifySupportedAreaRaw(uint32_t listIndex, const AreaStructureWrapper & modifiedArea)
{
    // The server instance (caller) is responsible for ensuring that there are no duplicate area IDs, list size not exceeded,
    // etc.
    if (modifiedArea.areaID != mSupportedAreas[listIndex].areaID)
    {
        ChipLogError(Zcl, "ModifySupportedAreaRaw - areaID's do not match, new areaID %u, existing areaID %u",
                     static_cast<unsigned>(modifiedArea.areaID), static_cast<unsigned>(mSupportedAreas[listIndex].areaID));
        return false;
    }

    mSupportedAreas[listIndex] = modifiedArea;
    return true;
}

bool LoggingServiceAreaStorageDelegate::ClearSupportedAreasRaw()
{
    if (!mSupportedAreas.empty())
    {
        mSupportedAreas.clear();
        return true;
    }

    return false;
}

bool LoggingServiceAreaStorageDelegate::RemoveSupportedAreaRaw(uint32_t areaId)
{
    for (auto it = mSupportedAreas.begin(); it != mSupportedAreas.end(); ++it)
    {
        if (it->areaID == areaId)
        {
            mSupportedAreas.erase(it);
            return true;
        }
    }

    return false;
}

//*************************************************************************
// Supported Maps accessors

uint32_t LoggingServiceAreaStorageDelegate::GetNumberOfSupportedMaps()
{
    return static_cast<uint32_t>(mSupportedMaps.size());
}

bool LoggingServiceAreaStorageDelegate::GetSupportedMapByIndex(uint32_t listIndex, MapStructureWrapper & supportedMap)
{
    if (listIndex < mSupportedMaps.size())
    {
        supportedMap = mSupportedMaps[listIndex];
        return true;
    }

    return false;
}

bool LoggingServiceAreaStorageDelegate::GetSupportedMapById(uint32_t aMapId, uint32_t & listIndex,
                                                            MapStructureWrapper & supportedMap)
{
    listIndex = 0;

    while (listIndex < mSupportedMaps.size())
    {
        if (mSupportedMaps[listIndex].mapID == aMapId)
        {
            supportedMap = mSupportedMaps[listIndex];
            return true;
        }

        ++listIndex;
    }

    return false;
}

bool LoggingServiceAreaStorageDelegate::AddSupportedMapRaw(const MapStructureWrapper & newMap, uint32_t & listIndex)
{
    // The server instance (caller) is responsible for ensuring that there are no duplicate map IDs, list size not exceeded,
    // etc.
    if (mSupportedMaps.size() < kMaxNumSupportedMaps)
    {
        mSupportedMaps.push_back(newMap);
        listIndex = static_cast<uint32_t>(mSupportedMaps.size()) - 1; // new element is last in list
        return true;
    }

    ChipLogError(Zcl, "AddSupportedMapRaw %u - supported maps list is already at maximum size %u",
                 static_cast<unsigned>(newMap.mapID), static_cast<unsigned>(kMaxNumSupportedMaps));

    return false;
}

bool LoggingServiceAreaStorageDelegate::ModifySupportedMapRaw(uint32_t listIndex, const MapStructureWrapper & modifiedMap)
{
    // The server instance (caller) is responsible for ensuring that there are no duplicate map IDs, list size not exceeded,
    // etc.
    if (modifiedMap.mapID != mSupportedMaps[listIndex].mapID)
    {
        ChipLogError(Zcl, "ModifySupportedMapRaw - mapID's do not match, new mapID %u, existing mapID %u",
                     static_cast<unsigned>(modifiedMap.mapID), static_cast<unsigned>(mSupportedMaps[listIndex].mapID));
        return false;
    }

    mSupportedMaps[listIndex] = modifiedMap;
    return true;
}

bool LoggingServiceAreaStorageDelegate::ClearSupportedMapsRaw()
{
    if (!mSupportedMaps.empty())
    {
        mSupportedMaps.clear();
        return true;
    }

    return false;
}

bool LoggingServiceAreaStorageDelegate::RemoveSupportedMapRaw(uint32_t mapId)
{
    for (auto it = mSupportedMaps.begin(); it != mSupportedMaps.end(); ++it)
    {
        if (it->mapID == mapId)
        {
            mSupportedMaps.erase(it);
            return true;
        }
    }

    return false;
}

//*************************************************************************
// Selected Areas accessors

uint32_t LoggingServiceAreaStorageDelegate::GetNumberOfSelectedAreas()
{
    return static_cast<uint32_t>(mSelectedAreas.size());
}

bool LoggingServiceAreaStorageDelegate::GetSelectedAreaByIndex(uint32_t listIndex, uint32_t & selectedArea)
{
    if (listIndex < mSelectedAreas.size())
    {
        selectedArea = mSelectedAreas[listIndex];
        return true;
    }

    return false;
}

bool LoggingServiceAreaStorageDelegate::AddSelectedAreaRaw(uint32_t aAreaId, uint32_t & listIndex)
{
    // The server instance (caller) is responsible for ensuring that there are no duplicate area IDs, list size not exceeded,
    // etc.
    if (mSelectedAreas.size() < kMaxNumSelectedAreas)
    {
        mSelectedAreas.push_back(aAreaId);
        listIndex = static_cast<uint32_t>(mSelectedAreas.size()) - 1; // new element is last in list
        return true;
    }

    ChipLogError(Zcl, "AddSelectedAreaRaw %u - selected areas list is already at maximum size %u", static_cast<unsigned>(aAreaId),
                 static_cast<unsigned>(kMaxNumSelectedAreas));

    return false;
}

bool LoggingServiceAreaStorageDelegate::ClearSelectedAreasRaw()
{
    if (!mSelectedAreas.empty())
    {
        mSelectedAreas.clear();
        return true;
    }

    return false;
}

bool LoggingServiceAreaStorageDelegate::RemoveSelectedAreasRaw(uint32_t areaId)
{
    for (auto it = mSelectedAreas.begin(); it != mSelectedAreas.end(); ++it)
    {
        if (*it == areaId)
        {
            mSelectedAreas.erase(it);
            return true;
        }
    }

    return false;
}

//*************************************************************************
// Progress List accessors

uint32_t LoggingServiceAreaStorageDelegate::GetNumberOfProgressElements()
{
    return static_cast<uint32_t>(mProgressList.size());
}

bool LoggingServiceAreaStorageDelegate::GetProgressElementByIndex(uint32_t listIndex,
                                                                  Structs::ProgressStruct::Type & aProgressElement)
{
    if (listIndex < mProgressList.size())
    {
        aProgressElement = mProgressList[listIndex];
        return true;
    }

    return false;
}

bool LoggingServiceAreaStorageDelegate::GetProgressElementById(uint32_t aAreaId, uint32_t & listIndex,
                                                               Structs::ProgressStruct::Type & aProgressElement)
{
    listIndex = 0;

    while (listIndex < mProgressList.size())
    {
        if (mProgressList[listIndex].areaID == aAreaId)
        {
            aProgressElement = mProgressList[listIndex];
            return true;
        }

        ++listIndex;
    }

    return false;
}

bool LoggingServiceAreaStorageDelegate::AddProgressElementRaw(const Structs::ProgressStruct::Type & newProgressElement,
                                                              uint32_t & listIndex)
{
    // The server instance (caller) is responsible for ensuring that there are no duplicate area IDs, list size not exceeded,
    // etc.
    if (mProgressList.size() < kMaxNumProgressElements)
    {
        mProgressList.push_back(newProgressElement);
        listIndex = static_cast<uint32_t>(mProgressList.size()) - 1; // new element is last in list
        return true;
    }

    ChipLogError(Zcl, "AddProgressElementRaw %u - progress list is already at maximum size %u",
                 static_cast<unsigned>(newProgressElement.areaID), static_cast<unsigned>(kMaxNumProgressElements));

    return false;
}

bool LoggingServiceAreaStorageDelegate::ModifyProgressElementRaw(uint32_t listIndex,
                                                                 const Structs::ProgressStruct::Type & modifiedProgressElement)
{
    if (modifiedProgressElement.areaID != mProgressList[listIndex].areaID)
    {
        ChipLogError(Zcl, "ModifyProgressElementRaw - areaID's do not match, new areaID %u, existing areaID %u",
                     static_cast<unsigned>(modifiedProgressElement.areaID), static_cast<unsigned>(mProgressList[listIndex].areaID));
        return false;
    }

    mProgressList[listIndex] = modifiedProgressElement;
    return true;
}

bool LoggingServiceAreaStorageDelegate::ClearProgressRaw()
{
    if (!mProgressList.empty())
    {
        mProgressList.clear();
        return true;
    }

    return false;
}

bool LoggingServiceAreaStorageDelegate::RemoveProgressElementRaw(uint32_t areaId)
{
    for (auto it = mProgressList.begin(); it != mProgressList.end(); ++it)
    {
        if (it->areaID == areaId)
        {
            mProgressList.erase(it);
            return true;
        }
    }

    return false;
}

} // namespace chip::app::Clusters::ServiceArea

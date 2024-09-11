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
#include <rvc-service-area-storage-delegate.h>
#include <vector>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ServiceArea;

//*************************************************************************
// Supported Areas accessors

uint32_t RvcServiceAreaStorageDelegate::GetNumberOfSupportedAreas()
{
    return static_cast<uint32_t>(mSupportedAreas.size());
}

bool RvcServiceAreaStorageDelegate::GetSupportedAreaByIndex(uint32_t listIndex, AreaStructureWrapper & supportedArea)
{
    if (listIndex < mSupportedAreas.size())
    {
        supportedArea = mSupportedAreas[listIndex];
        return true;
    }

    return false;
};

bool RvcServiceAreaStorageDelegate::GetSupportedAreaById(uint32_t aAreaID, uint32_t & listIndex,
                                                         AreaStructureWrapper & supportedArea)
{
    // We do not need to reimplement this method as it's already done by the SDK.
    // We are reimplementing this method, still using linear search, but with some optimization on the SDK implementation
    // since we have direct access to the list.
    listIndex = 0;

    while (listIndex < mSupportedAreas.size())
    {
        if (mSupportedAreas[listIndex].areaID == aAreaID)
        {
            supportedArea = mSupportedAreas[listIndex];
            return true;
        }

        ++listIndex;
    }

    return false;
};

bool RvcServiceAreaStorageDelegate::AddSupportedAreaRaw(const AreaStructureWrapper & newArea, uint32_t & listIndex)
{
    // The server instance (caller) is responsible for ensuring that there are no duplicate area IDs, list size not exceeded,
    // etc.

    // Double-check list size to ensure there no memory issues.
    if (mSupportedAreas.size() < kMaxNumSupportedAreas)
    {
        // not sorting list, number of areas normally expected to be small, max 255
        mSupportedAreas.push_back(newArea);
        listIndex = static_cast<uint32_t>(mSupportedMaps.size()) - 1; // new element is last in list
        return true;
    }

    ChipLogError(Zcl, "AddSupportedAreaRaw %u - supported areas list is already at maximum size %u", newArea.areaID,
                 static_cast<uint32_t>(kMaxNumSupportedAreas));

    return false;
}

bool RvcServiceAreaStorageDelegate::ModifySupportedAreaRaw(uint32_t listIndex, const AreaStructureWrapper & modifiedArea)
{
    // The server instance (caller) is responsible for ensuring that there are no duplicate area IDs, list size not exceeded,
    // etc.

    // Double-check that areaID's match.
    if (modifiedArea.areaID != mSupportedAreas[listIndex].areaID)
    {
        ChipLogError(Zcl, "ModifySupportedAreaRaw - areaID's do not match, new areaID %u, existing areaID %u", modifiedArea.areaID,
                     mSupportedAreas[listIndex].areaID);
        return false;
    }

    // checks passed, update the attribute
    mSupportedAreas[listIndex] = modifiedArea;
    return true;
}

bool RvcServiceAreaStorageDelegate::ClearSupportedAreasRaw()
{
    if (!mSupportedAreas.empty())
    {
        mSupportedAreas.clear();
        return true;
    }

    return false;
}

bool RvcServiceAreaStorageDelegate::RemoveSupportedAreaRaw(uint32_t areaId)
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

uint32_t RvcServiceAreaStorageDelegate::GetNumberOfSupportedMaps()
{
    return static_cast<uint32_t>(mSupportedMaps.size());
}

bool RvcServiceAreaStorageDelegate::GetSupportedMapByIndex(uint32_t listIndex, MapStructureWrapper & aSupportedMap)
{
    if (listIndex < mSupportedMaps.size())
    {
        aSupportedMap = mSupportedMaps[listIndex];
        return true;
    }

    return false;
};

bool RvcServiceAreaStorageDelegate::GetSupportedMapById(uint32_t aMapId, uint32_t & listIndex, MapStructureWrapper & aSupportedMap)
{
    // We do not need to reimplement this method as it's already done by the SDK.
    // We are reimplementing this method, still using linear search, but with some optimization on the SDK implementation
    // since we have direct access to the list.
    listIndex = 0;

    while (listIndex < mSupportedMaps.size())
    {
        if (mSupportedMaps[listIndex].mapID == aMapId)
        {
            aSupportedMap = mSupportedMaps[listIndex];
            return true;
        }

        ++listIndex;
    }

    return false;
};

bool RvcServiceAreaStorageDelegate::AddSupportedMapRaw(const MapStructureWrapper & newMap, uint32_t & listIndex)
{
    // The server instance (caller) is responsible for ensuring that there are no duplicate area IDs, list size not exceeded,
    // etc.

    // Double-check list size to ensure there no memory issues.
    if (mSupportedMaps.size() < kMaxNumSupportedMaps)
    {
        // not sorting list, number of areas normally expected to be small, max 255
        mSupportedMaps.push_back(newMap);
        listIndex = static_cast<uint32_t>(mSupportedMaps.size()) - 1; // new element is last in list
        return true;
    }
    ChipLogError(Zcl, "AddSupportedMapRaw %u - supported maps list is already at maximum size %u", newMap.mapID,
                 static_cast<uint32_t>(kMaxNumSupportedMaps));

    return false;
}

bool RvcServiceAreaStorageDelegate::ModifySupportedMapRaw(uint32_t listIndex, const MapStructureWrapper & modifiedMap)
{
    // The server instance (caller) is responsible for ensuring that there are no duplicate area IDs, list size not exceeded,
    // etc.

    // Double-check that mapID's match.
    if (modifiedMap.mapID != mSupportedMaps[listIndex].mapID)
    {
        ChipLogError(Zcl, "ModifySupportedMapRaw - mapID's do not match, new mapID %u, existing mapID %u", modifiedMap.mapID,
                     mSupportedMaps[listIndex].mapID);
        return false;
    }

    // save modified map
    mSupportedMaps[listIndex] = modifiedMap;
    return true;
}

bool RvcServiceAreaStorageDelegate::ClearSupportedMapsRaw()
{
    if (!mSupportedMaps.empty())
    {
        mSupportedMaps.clear();
        return true;
    }

    return false;
}

bool RvcServiceAreaStorageDelegate::RemoveSupportedMapRaw(uint32_t mapId)
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
// Selected areas accessors

uint32_t RvcServiceAreaStorageDelegate::GetNumberOfSelectedAreas()
{
    return static_cast<uint32_t>(mSelectedAreas.size());
}

bool RvcServiceAreaStorageDelegate::GetSelectedAreaByIndex(uint32_t listIndex, uint32_t & selectedArea)
{
    if (listIndex < mSelectedAreas.size())
    {
        selectedArea = mSelectedAreas[listIndex];
        return true;
    }

    return false;
};

bool RvcServiceAreaStorageDelegate::AddSelectedAreaRaw(uint32_t aAreaID, uint32_t & listIndex)
{
    // The server instance (caller) is responsible for ensuring that there are no duplicate area IDs, list size not exceeded,
    // etc.

    // Double-check list size to ensure there no memory issues.
    if (mSelectedAreas.size() < kMaxNumSelectedAreas)
    {
        // not sorting list, number of areas normally expected to be small, max 255
        mSelectedAreas.push_back(aAreaID);
        listIndex = static_cast<uint32_t>(mSelectedAreas.size()) - 1; // new element is last in list
        return true;
    }
    ChipLogError(Zcl, "AddSelectedAreaRaw %u - selected areas list is already at maximum size %u", aAreaID,
                 static_cast<uint32_t>(kMaxNumSelectedAreas));

    return false;
}

bool RvcServiceAreaStorageDelegate::ClearSelectedAreasRaw()
{
    if (!mSelectedAreas.empty())
    {
        mSelectedAreas.clear();
        return true;
    }

    return false;
}

bool RvcServiceAreaStorageDelegate::RemoveSelectedAreasRaw(uint32_t areaId)
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

uint32_t RvcServiceAreaStorageDelegate::GetNumberOfProgressElements()
{
    return static_cast<uint32_t>(mProgressList.size());
}

bool RvcServiceAreaStorageDelegate::GetProgressElementByIndex(uint32_t listIndex, Structs::ProgressStruct::Type & aProgressElement)
{
    if (listIndex < mProgressList.size())
    {
        aProgressElement = mProgressList[listIndex];
        return true;
    }

    return false;
};

bool RvcServiceAreaStorageDelegate::GetProgressElementById(uint32_t aAreaID, uint32_t & listIndex,
                                                           Structs::ProgressStruct::Type & aProgressElement)
{
    // We do not need to reimplement this method as it's already done by the SDK.
    // We are reimplementing this method, still using linear search, but with some optimization on the SDK implementation
    // since we have direct access to the list.
    listIndex = 0;

    while (listIndex < mProgressList.size())
    {
        if (mProgressList[listIndex].areaID == aAreaID)
        {
            aProgressElement = mProgressList[listIndex];
            return true;
        }

        ++listIndex;
    }

    return false;
};

bool RvcServiceAreaStorageDelegate::AddProgressElementRaw(const Structs::ProgressStruct::Type & newProgressElement,
                                                          uint32_t & listIndex)
{
    // The server instance (caller) is responsible for ensuring that there are no duplicate area IDs, list size not exceeded,
    // etc.

    // Double-check list size to ensure there no memory issues.
    if (mProgressList.size() < kMaxNumProgressElements)
    {
        // not sorting list, number of areas normally expected to be small, max 255
        mProgressList.push_back(newProgressElement);
        listIndex = static_cast<uint32_t>(mProgressList.size()) - 1; // new element is last in list
        return true;
    }
    ChipLogError(Zcl, "AddProgressElementRaw %u -progress list is already at maximum size %u", newProgressElement.areaID,
                 static_cast<uint32_t>(kMaxNumProgressElements));

    return false;
}

bool RvcServiceAreaStorageDelegate::ModifyProgressElementRaw(uint32_t listIndex,
                                                             const Structs::ProgressStruct::Type & modifiedProgressElement)
{
    if (modifiedProgressElement.areaID != mProgressList[listIndex].areaID)
    {
        ChipLogError(Zcl, "ModifyProgressElementRaw - areaID's do not match, new areaID %u, existing areaID %u",
                     modifiedProgressElement.areaID, mProgressList[listIndex].areaID);
        return false;
    }

    mProgressList[listIndex] = modifiedProgressElement;
    return true;
}

bool RvcServiceAreaStorageDelegate::ClearProgressRaw()
{
    if (!mProgressList.empty())
    {
        mProgressList.clear();
        return true;
    }

    return false;
}

bool RvcServiceAreaStorageDelegate::RemoveProgressElementRaw(uint32_t areaId)
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

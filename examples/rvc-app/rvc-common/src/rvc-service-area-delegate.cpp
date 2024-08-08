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
#include <vector>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ServiceArea;

CHIP_ERROR RvcServiceAreaDelegate::Init()
{
    // hardcoded fill of SUPPORTED MAPS for prototyping
    uint32_t supportedMapId_XX = 3;
    uint32_t supportedMapId_YY = 245;

    GetInstance()->AddSupportedMap(supportedMapId_XX, "My Map XX"_span);
    GetInstance()->AddSupportedMap(supportedMapId_YY, "My Map YY"_span);

    // hardcoded fill of SUPPORTED AREAS for prototyping
    uint32_t supportedAreaID_A = 7;
    uint32_t supportedAreaID_B = 1234567;
    uint32_t supportedAreaID_C = 10050;
    uint32_t supportedAreaID_D = 0x88888888;

    // Area A has name, floor number, uses map XX
    auto areaA =
        AreaStructureWrapper{}
            .SetAreaId(supportedAreaID_A)
            .SetMapId(supportedMapId_XX)
            .SetLocationInfo("My Location A"_span, DataModel::Nullable<int16_t>(4), DataModel::Nullable<Globals::AreaTypeTag>());

    // Area B has name, uses map XX
    auto areaB = AreaStructureWrapper{}
                     .SetMapId(supportedAreaID_B)
                     .SetMapId(supportedMapId_XX)
                     .SetLocationInfo("My Location B"_span, DataModel::NullNullable, DataModel::NullNullable);

    // Area C has full SemData, no name, Map YY
    auto areaC = AreaStructureWrapper{}
                     .SetAreaId(supportedAreaID_C)
                     .SetMapId(supportedMapId_YY)
                     .SetLocationInfo(""_span, -1, Globals::AreaTypeTag::kPlayRoom)
                     .SetLandmarkInfo(Globals::LandmarkTag::kBackDoor, Globals::RelativePositionTag::kNextTo);

    // Area D has null values for all landmark fields, Map YY
    auto areaD = AreaStructureWrapper{}
                     .SetAreaId(supportedAreaID_D)
                     .SetMapId(supportedMapId_YY)
                     .SetLocationInfo("My Location D"_span, DataModel::NullNullable, DataModel::NullNullable)
                     .SetLandmarkInfo(Globals::LandmarkTag::kCouch, Globals::RelativePositionTag::kNextTo);

    GetInstance()->AddSupportedArea(areaA);
    GetInstance()->AddSupportedArea(areaB);
    GetInstance()->AddSupportedArea(areaC);
    GetInstance()->AddSupportedArea(areaD);
    GetInstance()->SetCurrentArea(supportedAreaID_C);

    return CHIP_NO_ERROR;
}

//*************************************************************************
// command support

bool RvcServiceAreaDelegate::IsSetSelectedAreasAllowed(MutableCharSpan statusText)
{
    // TODO IMPLEMENT
    return true;
};

bool RvcServiceAreaDelegate::IsValidSelectAreasSet(const Commands::SelectAreas::DecodableType & req, SelectAreasStatus & areaStatus,
                                                   MutableCharSpan statusText)
{
    // TODO IMPLEMENT
    return true;
};

bool RvcServiceAreaDelegate::HandleSkipCurrentArea(uint32_t skippedArea, MutableCharSpan skipStatusText)
{
    // TODO IMPLEMENT
    return true;
};

//*************************************************************************
// Supported Areas accessors

bool RvcServiceAreaDelegate::IsSupportedAreasChangeAllowed()
{
    // TODO IMPLEMENT
    return true;
}

uint32_t RvcServiceAreaDelegate::GetNumberOfSupportedAreas()
{
    return static_cast<uint32_t>(mSupportedAreas.size());
}

bool RvcServiceAreaDelegate::GetSupportedAreaByIndex(uint32_t listIndex, AreaStructureWrapper & supportedArea)
{
    if (listIndex < mSupportedAreas.size())
    {
        supportedArea = mSupportedAreas[listIndex];
        return true;
    }

    return false;
};

bool RvcServiceAreaDelegate::GetSupportedAreaById(uint32_t aAreaID, uint32_t & listIndex, AreaStructureWrapper & supportedArea)
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

bool RvcServiceAreaDelegate::AddSupportedArea(const AreaStructureWrapper & newArea, uint32_t & listIndex)
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

    ChipLogError(Zcl, "AddSupportedArea %u - supported areas list is already at maximum size %u", newArea.areaID,
                 static_cast<uint32_t>(kMaxNumSupportedAreas));

    return false;
}

bool RvcServiceAreaDelegate::ModifySupportedArea(uint32_t listIndex, const AreaStructureWrapper & modifiedArea)
{
    // The server instance (caller) is responsible for ensuring that there are no duplicate area IDs, list size not exceeded,
    // etc.

    // Double-check that areaID's match.
    if (modifiedArea.areaID != mSupportedAreas[listIndex].areaID)
    {
        ChipLogError(Zcl, "ModifySupportedArea - areaID's do not match, new areaID %u, existing areaID %u", modifiedArea.areaID,
                     mSupportedAreas[listIndex].areaID);
        return false;
    }

    // checks passed, update the attribute
    mSupportedAreas[listIndex] = modifiedArea;
    return true;
}

bool RvcServiceAreaDelegate::ClearSupportedAreas()
{
    if (!mSupportedAreas.empty())
    {
        mSupportedAreas.clear();
        return true;
    }

    return false;
}

//*************************************************************************
// Supported Maps accessors

bool RvcServiceAreaDelegate::IsSupportedMapChangeAllowed()
{
    // TODO IMPLEMENT
    return true;
}

uint32_t RvcServiceAreaDelegate::GetNumberOfSupportedMaps()
{
    return static_cast<uint32_t>(mSupportedMaps.size());
}

bool RvcServiceAreaDelegate::GetSupportedMapByIndex(uint32_t listIndex, MapStructureWrapper & aSupportedMap)
{
    if (listIndex < mSupportedMaps.size())
    {
        aSupportedMap = mSupportedMaps[listIndex];
        return true;
    }

    return false;
};

bool RvcServiceAreaDelegate::GetSupportedMapById(uint32_t aMapId, uint32_t & listIndex, MapStructureWrapper & aSupportedMap)
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

bool RvcServiceAreaDelegate::AddSupportedMap(const MapStructureWrapper & newMap, uint32_t & listIndex)
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
    ChipLogError(Zcl, "AddSupportedMap %u - supported maps list is already at maximum size %u", newMap.mapID,
                 static_cast<uint32_t>(kMaxNumSupportedMaps));

    return false;
}

bool RvcServiceAreaDelegate::ModifySupportedMap(uint32_t listIndex, const MapStructureWrapper & modifiedMap)
{
    // The server instance (caller) is responsible for ensuring that there are no duplicate area IDs, list size not exceeded,
    // etc.

    // Double-check that mapID's match.
    if (modifiedMap.mapID != mSupportedMaps[listIndex].mapID)
    {
        ChipLogError(Zcl, "ModifySupportedMap - mapID's do not match, new mapID %u, existing mapID %u", modifiedMap.mapID,
                     mSupportedMaps[listIndex].mapID);
        return false;
    }

    // save modified map
    mSupportedMaps[listIndex] = modifiedMap;
    return true;
}

bool RvcServiceAreaDelegate::ClearSupportedMaps()
{
    if (!mSupportedMaps.empty())
    {
        mSupportedMaps.clear();
        return true;
    }

    return false;
}

//*************************************************************************
// Selected areas accessors

uint32_t RvcServiceAreaDelegate::GetNumberOfSelectedAreas()
{
    return static_cast<uint32_t>(mSelectedAreas.size());
}

bool RvcServiceAreaDelegate::GetSelectedAreaByIndex(uint32_t listIndex, uint32_t & selectedArea)
{
    if (listIndex < mSelectedAreas.size())
    {
        selectedArea = mSelectedAreas[listIndex];
        return true;
    }

    return false;
};

bool RvcServiceAreaDelegate::AddSelectedArea(uint32_t aAreaID, uint32_t & listIndex)
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
    ChipLogError(Zcl, "AddSelectedArea %u - selected areas list is already at maximum size %u", aAreaID,
                 static_cast<uint32_t>(kMaxNumSelectedAreas));

    return false;
}

bool RvcServiceAreaDelegate::ClearSelectedAreas()
{
    if (!mSelectedAreas.empty())
    {
        mSelectedAreas.clear();
        return true;
    }

    return false;
}

//*************************************************************************
// Progress List accessors

uint32_t RvcServiceAreaDelegate::GetNumberOfProgressElements()
{
    return static_cast<uint32_t>(mProgressList.size());
}

bool RvcServiceAreaDelegate::GetProgressElementByIndex(uint32_t listIndex, Structs::ProgressStruct::Type & aProgressElement)
{
    if (listIndex < mProgressList.size())
    {
        aProgressElement = mProgressList[listIndex];
        return true;
    }

    return false;
};

bool RvcServiceAreaDelegate::GetProgressElementById(uint32_t aAreaID, uint32_t & listIndex,
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

bool RvcServiceAreaDelegate::AddProgressElement(const Structs::ProgressStruct::Type & newProgressElement, uint32_t & listIndex)
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
    ChipLogError(Zcl, "AddProgressElement %u -progress list is already at maximum size %u", newProgressElement.areaID,
                 static_cast<uint32_t>(kMaxNumProgressElements));

    return false;
}

bool RvcServiceAreaDelegate::ModifyProgressElement(uint32_t listIndex,
                                                   const Structs::ProgressStruct::Type & modifiedProgressElement)
{
    // TODO IMPLEMENT
    return false;
}

bool RvcServiceAreaDelegate::ClearProgress()
{
    if (!mProgressList.empty())
    {
        mProgressList.clear();
        return true;
    }

    return false;
}

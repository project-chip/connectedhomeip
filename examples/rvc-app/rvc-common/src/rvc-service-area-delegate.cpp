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

void RvcServiceAreaDelegate::SetMapTopology()
{
    GetInstance()->ClearSupportedMaps();

    GetInstance()->AddSupportedMap(supportedMapId_XX, "My Map XX"_span);
    GetInstance()->AddSupportedMap(supportedMapId_YY, "My Map YY"_span);

    // Area A has name, floor number, uses map XX
    auto areaA =
        AreaStructureWrapper{}
            .SetAreaId(supportedAreaID_A)
            .SetMapId(supportedMapId_XX)
            .SetLocationInfo("My Location A"_span, DataModel::Nullable<int16_t>(4), DataModel::Nullable<Globals::AreaTypeTag>());

    // Area B has name, uses map XX
    auto areaB = AreaStructureWrapper{}
                     .SetAreaId(supportedAreaID_B)
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
}

void RvcServiceAreaDelegate::SetNoMapTopology()
{
    GetInstance()->ClearSupportedMaps();

    // Area A has name, floor number.
    auto areaA =
        AreaStructureWrapper{}
            .SetAreaId(supportedAreaID_A)
            .SetLocationInfo("My Location A"_span, DataModel::Nullable<int16_t>(4), DataModel::Nullable<Globals::AreaTypeTag>());

    // Area B has name.
    auto areaB = AreaStructureWrapper{}
                     .SetAreaId(supportedAreaID_B)
                     .SetLocationInfo("My Location B"_span, DataModel::NullNullable, DataModel::NullNullable);

    // Area C has full SemData, no name.
    auto areaC = AreaStructureWrapper{}
                     .SetAreaId(supportedAreaID_C)
                     .SetLocationInfo(""_span, -1, Globals::AreaTypeTag::kPlayRoom)
                     .SetLandmarkInfo(Globals::LandmarkTag::kBackDoor, Globals::RelativePositionTag::kNextTo);

    // Area D has null values for all landmark fields.
    auto areaD = AreaStructureWrapper{}
                     .SetAreaId(supportedAreaID_D)
                     .SetLocationInfo("My Location D"_span, DataModel::NullNullable, DataModel::NullNullable)
                     .SetLandmarkInfo(Globals::LandmarkTag::kCouch, Globals::RelativePositionTag::kNextTo);

    GetInstance()->AddSupportedArea(areaA);
    GetInstance()->AddSupportedArea(areaB);
    GetInstance()->AddSupportedArea(areaC);
    GetInstance()->AddSupportedArea(areaD);
}

CHIP_ERROR RvcServiceAreaDelegate::Init()
{
    SetMapTopology();

    GetInstance()->SetCurrentArea(supportedAreaID_C);

    return CHIP_NO_ERROR;
}

//*************************************************************************
// command support

bool RvcServiceAreaDelegate::IsSetSelectedAreasAllowed(MutableCharSpan & statusText)
{
    return (mIsSetSelectedAreasAllowedDeviceInstance->*mIsSetSelectedAreasAllowedCallback)(statusText);
};

bool RvcServiceAreaDelegate::IsValidSelectAreasSet(const Span<const uint32_t> & selectedAreas, SelectAreasStatus & areaStatus,
                                                   MutableCharSpan & statusText)
{
    if (selectedAreas.empty())
    {
        return true;
    }

    // If there is 1 or 0 supported maps, any combination of areas is valid.
    if (!GetInstance()->HasFeature(Feature::kMaps) || GetInstance()->GetNumberOfSupportedMaps() <= 1)
    {
        return true;
    }

    // Check that all the requested areas are in the same map.
    {
        AreaStructureWrapper tempArea;
        uint32_t ignoredIndex;
        if (!GetInstance()->GetSupportedAreaById(selectedAreas[0], ignoredIndex, tempArea))
        {
            areaStatus = SelectAreasStatus::kUnsupportedArea;
            CopyCharSpanToMutableCharSpanWithTruncation("unable to find selected area in supported areas"_span, statusText);
            return false;
        }

        auto mapId = tempArea.mapID.Value(); // It is safe to call `.Value()` as we confirmed that there are at least 2 maps.

        for (const auto & areaId : selectedAreas.SubSpan(1))
        {
            if (!GetInstance()->GetSupportedAreaById(areaId, ignoredIndex, tempArea))
            {
                areaStatus = SelectAreasStatus::kUnsupportedArea;
                CopyCharSpanToMutableCharSpanWithTruncation("unable to find selected area in supported areas"_span, statusText);
                return false;
            }

            if (tempArea.mapID.Value() != mapId)
            {
                areaStatus = SelectAreasStatus::kInvalidSet;
                CopyCharSpanToMutableCharSpanWithTruncation("all selected areas must be in the same map"_span, statusText);
                return false;
            }
        }
    }

    return true;
};

bool RvcServiceAreaDelegate::HandleSkipArea(uint32_t skippedArea, MutableCharSpan & skipStatusText)
{
    return (mHandleSkipAreaDeviceInstance->*mHandleSkipAreaCallback)(skippedArea, skipStatusText);
};

bool RvcServiceAreaDelegate::IsSupportedAreasChangeAllowed()
{
    return (mIsSupportedAreasChangeAllowedDeviceInstance->*mIsSupportedAreasChangeAllowedCallback)();
}

bool RvcServiceAreaDelegate::IsSupportedMapChangeAllowed()
{
    return (mIsSupportedMapChangeAllowedDeviceInstance->*mIsSupportedMapChangeAllowedCallback)();
}

void RvcServiceAreaDelegate::SetAttributesAtCleanStart()
{
    if (GetInstance()->GetNumberOfSupportedAreas() == 0)
    {
        return;
    }

    if (GetInstance()->GetNumberOfSelectedAreas() == 0)
    {
        AreaStructureWrapper firstArea;
        GetInstance()->GetSupportedAreaByIndex(0, firstArea);

        GetInstance()->SetCurrentArea(firstArea.areaID);

        if (GetInstance()->HasFeature(Feature::kProgressReporting))
        {
            GetInstance()->AddPendingProgressElement(firstArea.areaID);
            GetInstance()->SetProgressStatus(firstArea.areaID, OperationalStatusEnum::kOperating);
        }
    }
    else
    {
        uint32_t areaId;
        GetInstance()->GetSelectedAreaByIndex(0, areaId);

        GetInstance()->SetCurrentArea(areaId);

        if (GetInstance()->HasFeature(Feature::kProgressReporting))
        {
            GetInstance()->AddPendingProgressElement(areaId);
            GetInstance()->SetProgressStatus(areaId, OperationalStatusEnum::kOperating);

            uint32_t i = 1;
            while (GetInstance()->GetSelectedAreaByIndex(i, areaId))
            {
                GetInstance()->AddPendingProgressElement(areaId);
                i++;
            }
        }
    }
}

void RvcServiceAreaDelegate::GoToNextArea(OperationalStatusEnum currentAreaOpState, bool & finished)
{
    AreaStructureWrapper currentArea;
    auto currentAreaIdN = GetInstance()->GetCurrentArea();

    if (currentAreaIdN.IsNull())
    {
        ChipLogError(Zcl, "GoToNextArea: Cannot go to the next area when the current area is null.");
        return;
    }

    if (currentAreaOpState != OperationalStatusEnum::kCompleted && currentAreaOpState != OperationalStatusEnum::kSkipped)
    {
        ChipLogError(Zcl, "GoToNextArea: currentAreaOpState must be either completed or skipped.");
        return;
    }

    auto currentAreaId = currentAreaIdN.Value();
    uint32_t currentAreaIndex;
    GetInstance()->GetSupportedAreaById(currentAreaId, currentAreaIndex, currentArea);
    auto currentAreaMapId = currentArea.mapID;
    finished              = true;

    if (GetInstance()->HasFeature(Feature::kProgressReporting))
    {
        GetInstance()->SetProgressStatus(currentAreaId, currentAreaOpState);
    }

    if (GetInstance()->GetNumberOfSelectedAreas() == 0)
    {
        AreaStructureWrapper nextArea;
        uint32_t nextIndex = currentAreaIndex + 1;
        while (GetInstance()->GetSupportedAreaByIndex(nextIndex, nextArea))
        {
            if (!currentAreaMapId.IsNull() && nextArea.mapID == currentAreaMapId.Value())
            {
                GetInstance()->SetCurrentArea(nextArea.areaID);

                if (GetInstance()->HasFeature(Feature::kProgressReporting))
                {
                    GetInstance()->SetProgressStatus(nextArea.areaID, OperationalStatusEnum::kOperating);
                }

                finished = false;
                return;
            }

            ++nextIndex;
        }
    }
    else
    {
        uint32_t selectedAreaId;
        uint32_t selectedAreaIndex = 0;
        while (GetInstance()->GetSelectedAreaByIndex(selectedAreaIndex, selectedAreaId))
        {
            if (selectedAreaId == currentAreaId)
            {
                break;
            }
            ++selectedAreaIndex;
        }

        uint32_t nextSelectedAreaId;
        uint32_t nextSelectedAreaIndex = selectedAreaIndex + 1;
        if (GetInstance()->GetSelectedAreaByIndex(nextSelectedAreaIndex, nextSelectedAreaId))
        {
            GetInstance()->SetCurrentArea(nextSelectedAreaId);

            if (GetInstance()->HasFeature(Feature::kProgressReporting))
            {
                GetInstance()->SetProgressStatus(nextSelectedAreaId, OperationalStatusEnum::kOperating);
            }

            finished = false;
            return;
        }
    }
}

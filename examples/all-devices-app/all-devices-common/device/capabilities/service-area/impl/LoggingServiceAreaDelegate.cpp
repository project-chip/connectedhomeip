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

#include "LoggingServiceAreaDelegate.h"
#include <RvcSimulationLogic.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app::Clusters::ServiceArea {

using namespace chip::app::all_devices::rvc_simulation;
using namespace chip::app::all_devices::rvc_simulation::Topology;

namespace {

bool IsOperating(OperationalState::OperationalStateCluster * cluster)
{
    return cluster != nullptr &&
        cluster->GetCurrentOperationalState() == to_underlying(OperationalState::OperationalStateEnum::kRunning);
}

} // namespace

CHIP_ERROR LoggingServiceAreaDelegate::Init()
{
    SetMapTopology();
    if (mCluster != nullptr)
    {
        mCluster->SetCurrentArea(DefaultCurrentAreaId());
    }
    return CHIP_NO_ERROR;
}

void LoggingServiceAreaDelegate::SetMapTopology()
{
    VerifyOrReturn(mCluster != nullptr);
    ApplyDefaultMapTopology(*mCluster);
}

bool LoggingServiceAreaDelegate::IsSetSelectedAreasAllowed(MutableCharSpan & statusText)
{
    if (IsOperating(mOperationalStateCluster))
    {
        CopyCharSpanToMutableCharSpanWithTruncation("cannot set the Selected Areas while the device is running"_span, statusText);
        return false;
    }
    return true;
}

bool LoggingServiceAreaDelegate::IsValidSelectAreasSet(const Span<const uint32_t> & selectedAreas,
                                                       SelectAreasStatus & locationStatus, MutableCharSpan & statusText)
{
    if (selectedAreas.empty())
    {
        return true;
    }

    VerifyOrReturnError(mCluster != nullptr, false);

    if (!mCluster->HasFeature(Feature::kMaps) || mCluster->GetNumberOfSupportedMaps() <= 1)
    {
        return true;
    }

    AreaStructureWrapper tempArea;
    uint32_t ignoredIndex;
    if (!mCluster->GetSupportedAreaById(selectedAreas[0], ignoredIndex, tempArea))
    {
        locationStatus = SelectAreasStatus::kUnsupportedArea;
        CopyCharSpanToMutableCharSpanWithTruncation("unable to find selected area in supported areas"_span, statusText);
        return false;
    }

    auto mapId = tempArea.mapID.Value();

    for (const auto & areaId : selectedAreas.SubSpan(1))
    {
        if (!mCluster->GetSupportedAreaById(areaId, ignoredIndex, tempArea))
        {
            locationStatus = SelectAreasStatus::kUnsupportedArea;
            CopyCharSpanToMutableCharSpanWithTruncation("unable to find selected area in supported areas"_span, statusText);
            return false;
        }

        if (tempArea.mapID.Value() != mapId)
        {
            locationStatus = SelectAreasStatus::kInvalidSet;
            CopyCharSpanToMutableCharSpanWithTruncation("all selected areas must be in the same map"_span, statusText);
            return false;
        }
    }

    return true;
}

bool LoggingServiceAreaDelegate::HandleSkipArea(uint32_t skippedArea, MutableCharSpan & skipStatusText)
{
    VerifyOrReturnError(mCluster != nullptr, false);

    if (mCluster->GetCurrentArea() != skippedArea)
    {
        CopyCharSpanToMutableCharSpanWithTruncation("the skipped area does not match the current area"_span, skipStatusText);
        return false;
    }

    if (!IsOperating(mOperationalStateCluster))
    {
        CopyCharSpanToMutableCharSpanWithTruncation("skip area is only accepted when the device is running"_span, skipStatusText);
        return false;
    }

    bool finished = false;
    GoToNextArea(OperationalStatusEnum::kSkipped, finished);

    if (finished && mActivityCompleteHandler)
    {
        mActivityCompleteHandler();
    }

    return true;
}

bool LoggingServiceAreaDelegate::IsSupportedAreasChangeAllowed()
{
    return !IsOperating(mOperationalStateCluster);
}

bool LoggingServiceAreaDelegate::IsSupportedMapChangeAllowed()
{
    return !IsOperating(mOperationalStateCluster);
}

void LoggingServiceAreaDelegate::SetAttributesAtCleanStart()
{
    VerifyOrReturn(mCluster != nullptr);

    if (mCluster->GetNumberOfSupportedAreas() == 0)
    {
        return;
    }

    if (mCluster->GetNumberOfSelectedAreas() == 0)
    {
        AreaStructureWrapper firstArea;
        mCluster->GetSupportedAreaByIndex(0, firstArea);

        mCluster->SetCurrentArea(firstArea.areaID);

        if (mCluster->HasFeature(Feature::kProgressReporting))
        {
            mCluster->AddPendingProgressElement(firstArea.areaID);
            mCluster->SetProgressStatus(firstArea.areaID, OperationalStatusEnum::kOperating);
        }
    }
    else
    {
        uint32_t areaId;
        mCluster->GetSelectedAreaByIndex(0, areaId);

        mCluster->SetCurrentArea(areaId);

        if (mCluster->HasFeature(Feature::kProgressReporting))
        {
            mCluster->AddPendingProgressElement(areaId);
            mCluster->SetProgressStatus(areaId, OperationalStatusEnum::kOperating);

            uint32_t i = 1;
            while (mCluster->GetSelectedAreaByIndex(i, areaId))
            {
                mCluster->AddPendingProgressElement(areaId);
                i++;
            }
        }
    }
}

void LoggingServiceAreaDelegate::GoToNextArea(OperationalStatusEnum currentAreaOpState, bool & finished)
{
    finished = true;
    VerifyOrReturn(mCluster != nullptr);

    auto currentAreaIdN = mCluster->GetCurrentArea();

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
    AreaStructureWrapper currentArea;
    uint32_t currentAreaIndex;
    if (!mCluster->GetSupportedAreaById(currentAreaId, currentAreaIndex, currentArea))
    {
        ChipLogError(Zcl, "GoToNextArea: current area %u not found in supported areas.", currentAreaId);
        return;
    }
    auto currentAreaMapId = currentArea.mapID;

    if (mCluster->HasFeature(Feature::kProgressReporting))
    {
        mCluster->SetProgressStatus(currentAreaId, currentAreaOpState);
    }

    if (mCluster->GetNumberOfSelectedAreas() == 0)
    {
        AreaStructureWrapper nextArea;
        uint32_t nextIndex = currentAreaIndex + 1;
        while (mCluster->GetSupportedAreaByIndex(nextIndex, nextArea))
        {
            if (!currentAreaMapId.IsNull() && nextArea.mapID == currentAreaMapId.Value())
            {
                mCluster->SetCurrentArea(nextArea.areaID);

                if (mCluster->HasFeature(Feature::kProgressReporting))
                {
                    mCluster->SetProgressStatus(nextArea.areaID, OperationalStatusEnum::kOperating);
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
        while (mCluster->GetSelectedAreaByIndex(selectedAreaIndex, selectedAreaId))
        {
            if (selectedAreaId == currentAreaId)
            {
                break;
            }
            ++selectedAreaIndex;
        }

        uint32_t nextSelectedAreaId;
        uint32_t nextSelectedAreaIndex = selectedAreaIndex + 1;
        if (mCluster->GetSelectedAreaByIndex(nextSelectedAreaIndex, nextSelectedAreaId))
        {
            mCluster->SetCurrentArea(nextSelectedAreaId);

            if (mCluster->HasFeature(Feature::kProgressReporting))
            {
                mCluster->SetProgressStatus(nextSelectedAreaId, OperationalStatusEnum::kOperating);
            }

            finished = false;
        }
    }
}

void LoggingServiceAreaDelegate::UpdateProgressOnExit()
{
    VerifyOrReturn(mCluster != nullptr);
    VerifyOrReturn(mCluster->HasFeature(Feature::kProgressReporting));

    uint32_t i = 0;
    Structs::ProgressStruct::Type progressElement;
    while (mCluster->GetProgressElementByIndex(i, progressElement))
    {
        if (progressElement.status == OperationalStatusEnum::kOperating ||
            progressElement.status == OperationalStatusEnum::kPending)
        {
            mCluster->SetProgressStatus(progressElement.areaID, OperationalStatusEnum::kSkipped);
        }
        i++;
    }
}

} // namespace chip::app::Clusters::ServiceArea

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

#pragma once

#include <app/clusters/service-area-server/service-area-server.h>
#include <app/util/config.h>
#include <cstring>
#include <utility>

namespace chip {
namespace app {
namespace Clusters {

class RvcDevice;

typedef bool (RvcDevice::*IsSetSelectedAreasAllowedCallback)(MutableCharSpan & statusText);
typedef bool (RvcDevice::*HandleSkipCurrentAreaCallback)(uint32_t skippedArea, MutableCharSpan & skipStatusText);
typedef bool (RvcDevice::*IsChangeAllowedSimpleCallback)();

namespace ServiceArea {

class RvcServiceAreaDelegate : public Delegate
{
private:
    // containers for array attributes.
    std::vector<ServiceArea::AreaStructureWrapper> mSupportedAreas;
    std::vector<ServiceArea::MapStructureWrapper> mSupportedMaps;
    std::vector<uint32_t> mSelectedAreas;
    std::vector<ServiceArea::Structs::ProgressStruct::Type> mProgressList;

    RvcDevice * mIsSetSelectedAreasAllowedDeviceInstance;
    IsSetSelectedAreasAllowedCallback mIsSetSelectedAreasAllowedCallback;
    RvcDevice * mHandleSkipCurrentAreaDeviceInstance;
    HandleSkipCurrentAreaCallback mHandleSkipCurrentAreaCallback;
    RvcDevice * mIsSupportedAreasChangeAllowedDeviceInstance;
    IsChangeAllowedSimpleCallback mIsSupportedAreasChangeAllowedCallback;
    RvcDevice * mIsSupportedMapChangeAllowedDeviceInstance;
    IsChangeAllowedSimpleCallback mIsSupportedMapChangeAllowedCallback;

    // hardcoded values for SUPPORTED MAPS.
    const uint32_t supportedMapId_XX = 3;
    const uint32_t supportedMapId_YY = 245;

    // hardcoded values for SUPPORTED AREAS.
    const uint32_t supportedAreaID_A = 7;
    const uint32_t supportedAreaID_B = 1234567;
    const uint32_t supportedAreaID_C = 10050;
    const uint32_t supportedAreaID_D = 0x88888888;

    /**
     * Set the SupportedMaps and SupportedAreas where the SupportedMaps is not null.
     */
    void SetMapTopology();

    /**
     * Set the SupportedMaps and SupportedAreas where the SupportedMaps is null.
     */
    void SetNoMapTopology();

public:
    CHIP_ERROR Init() override;

    // command support
    bool IsSetSelectedAreasAllowed(MutableCharSpan & statusText) override;

    bool IsValidSelectAreasSet(const ServiceArea::Commands::SelectAreas::DecodableType & req,
                               ServiceArea::SelectAreasStatus & areaStatus, MutableCharSpan & statusText) override;

    bool HandleSkipCurrentArea(uint32_t skippedArea, MutableCharSpan & skipStatusText) override;

    //*************************************************************************
    // Supported Areas accessors

    bool IsSupportedAreasChangeAllowed() override;

    uint32_t GetNumberOfSupportedAreas() override;

    bool GetSupportedAreaByIndex(uint32_t listIndex, AreaStructureWrapper & supportedArea) override;

    bool GetSupportedAreaById(uint32_t aAreaId, uint32_t & listIndex, AreaStructureWrapper & supportedArea) override;

    bool AddSupportedArea(const AreaStructureWrapper & newArea, uint32_t & listIndex) override;

    bool ModifySupportedArea(uint32_t listIndex, const AreaStructureWrapper & modifiedArea) override;

    bool ClearSupportedAreas() override;

    //*************************************************************************
    // Supported Maps accessors

    bool IsSupportedMapChangeAllowed() override;

    uint32_t GetNumberOfSupportedMaps() override;

    bool GetSupportedMapByIndex(uint32_t listIndex, ServiceArea::MapStructureWrapper & supportedMap) override;

    bool GetSupportedMapById(uint32_t aMapId, uint32_t & listIndex, ServiceArea::MapStructureWrapper & supportedMap) override;

    bool AddSupportedMap(const ServiceArea::MapStructureWrapper & newMap, uint32_t & listIndex) override;

    bool ModifySupportedMap(uint32_t listIndex, const ServiceArea::MapStructureWrapper & newMap) override;

    bool ClearSupportedMaps() override;

    //*************************************************************************
    // Selected Areas accessors

    uint32_t GetNumberOfSelectedAreas() override;

    bool GetSelectedAreaByIndex(uint32_t listIndex, uint32_t & selectedArea) override;

    bool AddSelectedArea(uint32_t aAreaId, uint32_t & listIndex) override;

    bool ClearSelectedAreas() override;

    //*************************************************************************
    // Progress accessors

    uint32_t GetNumberOfProgressElements() override;

    bool GetProgressElementByIndex(uint32_t listIndex, ServiceArea::Structs::ProgressStruct::Type & aProgressElement) override;

    bool GetProgressElementById(uint32_t aAreaId, uint32_t & listIndex,
                                ServiceArea::Structs::ProgressStruct::Type & aProgressElement) override;

    bool AddProgressElement(const ServiceArea::Structs::ProgressStruct::Type & newProgressElement, uint32_t & listIndex) override;

    bool ModifyProgressElement(uint32_t listIndex,
                               const ServiceArea::Structs::ProgressStruct::Type & modifiedProgressElement) override;

    bool ClearProgress() override;

    //*************************************************************************
    // RVC device callback setters

    void SetIsSetSelectedAreasAllowedCallback(IsSetSelectedAreasAllowedCallback callback, RvcDevice * instance)
    {
        mIsSetSelectedAreasAllowedCallback       = callback;
        mIsSetSelectedAreasAllowedDeviceInstance = instance;
    }

    void SetHandleSkipCurrentAreaCallback(HandleSkipCurrentAreaCallback callback, RvcDevice * instance)
    {
        mHandleSkipCurrentAreaCallback       = callback;
        mHandleSkipCurrentAreaDeviceInstance = instance;
    }

    void SetIsSupportedAreasChangeAllowedCallback(IsChangeAllowedSimpleCallback callback, RvcDevice * instance)
    {
        mIsSupportedAreasChangeAllowedCallback       = callback;
        mIsSupportedAreasChangeAllowedDeviceInstance = instance;
    }

    void SetIsSupportedMapChangeAllowedCallback(IsChangeAllowedSimpleCallback callback, RvcDevice * instance)
    {
        mIsSupportedMapChangeAllowedCallback       = callback;
        mIsSupportedMapChangeAllowedDeviceInstance = instance;
    }

    //*************************************************************************
    // Helper methods for setting service area attributes.

    /**
     * Sets the service area attributes at the start of a clean.
     * This includes the current area an progress attributes.
     */
    void SetAttributesAtCleanStart();

    /**
     * Go to the next area in the list of selected areas.
     * @param currentAreaOpState The operational state to be set in the Status field of the Progress attribute for the current area.
     * This can only be Completed or Skipped.
     * @param finished true if there are no more areas to clean an we should end the clean.
     */
    void GoToNextArea(OperationalStatusEnum currentAreaOpState, bool & finished);
};

} // namespace ServiceArea

} // namespace Clusters
} // namespace app
} // namespace chip

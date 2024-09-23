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
#include <vector>

namespace chip {
namespace app {
namespace Clusters {

class RvcDevice;

typedef bool (RvcDevice::*IsSetSelectedAreasAllowedCallback)(MutableCharSpan & statusText);
typedef bool (RvcDevice::*HandleSkipAreaCallback)(uint32_t skippedArea, MutableCharSpan & skipStatusText);
typedef bool (RvcDevice::*IsChangeAllowedSimpleCallback)();

namespace ServiceArea {

class RvcServiceAreaDelegate : public Delegate
{
private:
    RvcDevice * mIsSetSelectedAreasAllowedDeviceInstance;
    IsSetSelectedAreasAllowedCallback mIsSetSelectedAreasAllowedCallback;
    RvcDevice * mHandleSkipAreaDeviceInstance;
    HandleSkipAreaCallback mHandleSkipAreaCallback;
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

public:
    /**
     * Set the SupportedMaps and SupportedAreas where the SupportedMaps is not null.
     */
    void SetMapTopology();

    /**
     * Set the SupportedMaps and SupportedAreas where the SupportedMaps is null.
     */
    void SetNoMapTopology();

    CHIP_ERROR Init() override;

    // command support
    bool IsSetSelectedAreasAllowed(MutableCharSpan & statusText) override;

    bool IsValidSelectAreasSet(const Span<const uint32_t> & selectedAreas, ServiceArea::SelectAreasStatus & areaStatus,
                               MutableCharSpan & statusText) override;

    bool HandleSkipArea(uint32_t skippedArea, MutableCharSpan & skipStatusText) override;

    bool IsSupportedAreasChangeAllowed() override;

    bool IsSupportedMapChangeAllowed() override;

    //*************************************************************************
    // RVC device callback setters

    void SetIsSetSelectedAreasAllowedCallback(IsSetSelectedAreasAllowedCallback callback, RvcDevice * instance)
    {
        mIsSetSelectedAreasAllowedCallback       = callback;
        mIsSetSelectedAreasAllowedDeviceInstance = instance;
    }

    void SetHandleSkipAreaCallback(HandleSkipAreaCallback callback, RvcDevice * instance)
    {
        mHandleSkipAreaCallback       = callback;
        mHandleSkipAreaDeviceInstance = instance;
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

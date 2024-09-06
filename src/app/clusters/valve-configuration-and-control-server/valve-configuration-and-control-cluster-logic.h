/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
/**
 * @file Cross-platform API to handle cluster-specific logic for the valve configuration and control cluster on a single endpoint.
 */

#pragma once

#include "valve-configuration-and-control-delegate.h"
#include "valve-configuration-and-control-matter-context.h"
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ValveConfigurationAndControl {

struct ClusterConformance
{
    inline bool HasFeature(Feature feature) const { return featureMap & to_underlying(feature); }
    uint32_t featureMap;
    bool supportsDefaultOpenLevel;
    bool supportsValveFault;
    bool supportsLevelStep;
    bool Valid() const
    {
        bool supportsLvl = HasFeature(Feature::kLevel);
        if (supportsDefaultOpenLevel & !supportsLvl)
        {
            ChipLogError(Zcl,
                         "Invalid Valve configuration and control conformance - DefaultOpenLevel is not supported without LVL");
            return false;
        }
        if (supportsLevelStep & !supportsLvl)
        {
            ChipLogError(Zcl, "Invalid Valve configuration and control conformance - LevelStep is not supported without LVL");
            return false;
        }
        return true;
    }
};

struct ClusterState
{
    DataModel::Nullable<ElapsedS> openDuration        = DataModel::NullNullable;
    DataModel::Nullable<ElapsedS> defaultOpenDuration = DataModel::NullNullable;
    DataModel::Nullable<EpochUs> autoCloseTime        = DataModel::NullNullable;
    DataModel::Nullable<ElapsedS> remainingDuration   = DataModel::NullNullable;
    DataModel::Nullable<ValveStateEnum> currentState  = DataModel::NullNullable;
    DataModel::Nullable<ValveStateEnum> targetState   = DataModel::NullNullable;
    DataModel::Nullable<Percent> currentLevel         = DataModel::NullNullable;
    DataModel::Nullable<Percent> targetLevel          = DataModel::NullNullable;
    Percent defaultOpenLevel                          = 100u;
    BitMask<ValveFaultBitmap> valveFault              = 0u;
    uint8_t levelStep                                 = 1u;
};

class ClusterLogic
{
public:
    // Instantiates a ClusterLogic class. The caller maintains ownership of the driver and the context, but provides them for use by
    // the ClusterLogic class.
    ClusterLogic(Delegate & clusterDriver, MatterContext & matterContext) :
        mClusterDriver(clusterDriver), mMatterContext(matterContext)
    {
        // TODO: remove these once the fields are used properly
        (void) mClusterDriver;
        (void) mMatterContext;
    }

    // Validates the conformance and performs initialization.
    // Returns CHIP_ERROR_INCORRECT_STATE if the cluster has already been initialized.
    // Returns CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR if the conformance is incorrect.
    CHIP_ERROR Init(const ClusterConformance & conformance, const ClusterState & initialState = ClusterState());
    // CHIP_ERROR HandleOpen();
    // CHIP_ERROR HandleClose();

    // All Get functions:
    // Return CHIP_ERROR_INVALID_STATE if the class has not been initialized.
    // Return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if the attribute is not supported by the conformance.
    // Otherwise return CHIP_NO_ERROR and set the input parameter value to the current cluster state value
    CHIP_ERROR GetOpenDuration(DataModel::Nullable<ElapsedS> & openDuration);
    CHIP_ERROR GetDefaultOpenDuration(DataModel::Nullable<ElapsedS> & defaultOpenDuration);
    CHIP_ERROR GetAutoCloseTime(DataModel::Nullable<EpochUs> & autoCloseTime);
    CHIP_ERROR GetRemainingDuration(DataModel::Nullable<ElapsedS> & remainingDuration);
    CHIP_ERROR GetCurrentState(DataModel::Nullable<ValveStateEnum> & currentState);
    CHIP_ERROR GetTargetState(DataModel::Nullable<ValveStateEnum> & targetState);
    CHIP_ERROR GetCurrentLevel(DataModel::Nullable<Percent> & currentLevel);
    CHIP_ERROR GetTargetLevel(DataModel::Nullable<Percent> & targetLevel);
    CHIP_ERROR GetDefaultOpenLevel(Percent & defaultOpenLevel);
    CHIP_ERROR GetValveFault(BitMask<ValveFaultBitmap> & valveFault);
    CHIP_ERROR GetLevelStep(uint8_t & levelStep);

private:
    bool mInitialized = false;

    Delegate & mClusterDriver;
    MatterContext & mMatterContext;

    ClusterConformance mConformance;
    ClusterState mState;
};

} // namespace ValveConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip

/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <string>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvSettingsUserLevelManagement {

constexpr int16_t kPanMinMinValue  = -180;
constexpr int16_t kPanMinMaxValue  = 0;
constexpr int16_t kPanMaxMinValue  = 1;
constexpr int16_t kPanMaxMaxValue  = 180;
constexpr int16_t kTiltMinMinValue = -180;
constexpr int16_t kTiltMinMaxValue = 0;
constexpr int16_t kTiltMaxMinValue = 1;
constexpr int16_t kTiltMaxMaxValue = 180;
constexpr uint8_t kZoomMinValue    = 1;
constexpr uint8_t kZoomMaxMinValue = 2;
constexpr uint8_t kZoomMaxMaxValue = 100;

// Spec defined defaults for Pan, Tilt, and Zoom
constexpr int16_t kDefaultPan  = 0;
constexpr int16_t kDefaultTilt = 0;
constexpr uint8_t kDefaultZoom = 1;

constexpr size_t kMptzPositionStructMaxSerializedSize =
    TLV::EstimateStructOverhead(sizeof(int16_t), sizeof(int16_t), sizeof(uint8_t));

class PhysicalPTZCallback
{
public:
    PhysicalPTZCallback()                                                               = default;
    virtual ~PhysicalPTZCallback()                                                      = default;
    virtual void OnPhysicalMovementComplete(Protocols::InteractionModel::Status status) = 0;
};

struct MPTZPresetHelper
{
private:
    uint8_t mPresetID;
    std::string mName;
    CameraAvSettingsUserLevelManagement::Structs::MPTZStruct::Type mMptzPosition;

public:
    virtual ~MPTZPresetHelper() = default;
    MPTZPresetHelper() {}
    MPTZPresetHelper(uint8_t aPreset, CharSpan aName, CameraAvSettingsUserLevelManagement::Structs::MPTZStruct::Type aPosition)
    {
        SetPresetID(aPreset);
        SetName(aName);
        SetMptzPosition(aPosition);
    }

    // Accessors and Mutators
    //
    std::string GetName() const { return mName; }
    void SetName(chip::CharSpan aName)
    {
        mName = std::string(aName.begin(), aName.end());
        ChipLogProgress(Zcl, "My preset name being set %s", mName.c_str());
    }

    uint8_t GetPresetID() const { return mPresetID; }
    void SetPresetID(uint8_t aPreset) { mPresetID = aPreset; }

    CameraAvSettingsUserLevelManagement::Structs::MPTZStruct::Type GetMptzPosition() const { return mMptzPosition; }
    void SetMptzPosition(CameraAvSettingsUserLevelManagement::Structs::MPTZStruct::Type aPosition) { mMptzPosition = aPosition; }
};

} // namespace CameraAvSettingsUserLevelManagement
} // namespace Clusters
} // namespace app
} // namespace chip

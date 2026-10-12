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
#include <lib/support/CodeUtils.h>
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

// 1 control byte + end-of-array marker
constexpr size_t kArrayTlvOverhead = 2;

// Max size for an MPTZStruct
constexpr size_t kMptzPositionStructMaxSerializedSize =
    TLV::EstimateStructOverhead(sizeof(int16_t), sizeof(int16_t), sizeof(uint8_t));

// Maximum byte length of Name in MPTZSavePreset and MPTZPresetStruct.
constexpr size_t kMaxMPTZPresetNameLength = 32;

// Max size for an MPTZPresetStruct
constexpr size_t kMaxMPTZPresetStructSerializedSize =
    TLV::EstimateStructOverhead(sizeof(uint8_t), kMaxMPTZPresetNameLength, kMptzPositionStructMaxSerializedSize);

// Spec maximum number of entries in MPTZPresets (ZAP length="255").
constexpr size_t kSpecMaxMPTZPresets = 255;

// Max TLV size for a full MPTZPresets array at the spec maximum entry count.
constexpr size_t kMaxMPTZPresetsSerializedSize = kArrayTlvOverhead + (kSpecMaxMPTZPresets * kMaxMPTZPresetStructSerializedSize);

// Max size for a TLV encoded array of DPTZStruct
constexpr size_t kViewportStructMaxSerializedSize =
    TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t));
constexpr size_t kMaxDPTZStructSerializedSize = TLV::EstimateStructOverhead(sizeof(uint16_t), kViewportStructMaxSerializedSize);
constexpr size_t kMaxDPTZStreamsSerializedSize =
    kArrayTlvOverhead + (CHIP_CONFIG_MAX_NUM_CAMERA_VIDEO_STREAMS * kMaxDPTZStructSerializedSize);

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
    /**
     * @brief Destroy the owned preset data; allow destruction through a base helper pointer.
     */
    virtual ~MPTZPresetHelper() = default;
    /**
     * @brief Create a preset with an empty name; the caller must assign its ID before use.
     */
    MPTZPresetHelper() {}
    /**
     * @brief Construct a preset with an owned name; an overlength name leaves the name empty.
     * @param aPreset The preset ID, whose range must be validated by the caller.
     * @param aName The UTF-8 name, limited to 32 bytes.
     * @param aPosition The saved position, preserving the presence of optional axes.
     */
    MPTZPresetHelper(uint8_t aPreset, CharSpan aName, CameraAvSettingsUserLevelManagement::Structs::MPTZStruct::Type aPosition)
    {
        SetPresetID(aPreset);
        SetName(aName);
        SetMptzPosition(aPosition);
    }

    // Accessors and Mutators
    //
    /**
     * @brief Return an owned copy of the preset name that remains valid after the helper changes.
     * @return The complete stored name, including an empty name.
     */
    std::string GetName() const { return mName; }
    /**
     * @brief Copy a name of at most 32 bytes; an overlength name leaves the current value unchanged.
     * The void signature is retained for source compatibility. Callers accepting external input must validate the
     * length and report any error before calling this setter.
     * @param aName The UTF-8 name, whose limit is measured in bytes.
     */
    void SetName(chip::CharSpan aName)
    {
        VerifyOrReturn(aName.size() <= kMaxMPTZPresetNameLength,
                       ChipLogError(Zcl, "CameraAVSettingsUserLevelManagement: Preset name exceeds the maximum byte length"));
        mName = std::string(aName.begin(), aName.end());
        ChipLogProgress(Zcl, "My preset name being set %s", mName.c_str());
    }

    /**
     * @brief Return the ID assigned by the parameterized constructor or SetPresetID.
     * @return The assigned preset ID. The caller must initialize the ID before reading it.
     */
    uint8_t GetPresetID() const { return mPresetID; }
    /**
     * @brief Assign an ID without range validation.
     * @param aPreset The ID, whose cluster-specific range must be validated by the caller.
     */
    void SetPresetID(uint8_t aPreset) { mPresetID = aPreset; }

    /**
     * @brief Return the saved position, preserving absent optional pan, tilt, and zoom fields.
     * @return A copy of the saved position.
     */
    CameraAvSettingsUserLevelManagement::Structs::MPTZStruct::Type GetMptzPosition() const { return mMptzPosition; }
    /**
     * @brief Copy a saved position without validating axis ranges.
     * @param aPosition The saved position; the caller validates supported axes and their values.
     */
    void SetMptzPosition(CameraAvSettingsUserLevelManagement::Structs::MPTZStruct::Type aPosition) { mMptzPosition = aPosition; }
};

} // namespace CameraAvSettingsUserLevelManagement
} // namespace Clusters
} // namespace app
} // namespace chip

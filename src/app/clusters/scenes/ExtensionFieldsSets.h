/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        https://urldefense.com/v3/__http://www.apache.org/licenses/LICENSE-2.0__;!!N30Cs7Jr!UgbMbEQ59BIK-1Xslc7QXYm0lQBh92qA3ElecRe1CF_9YhXxbwPOZa6j4plru7B7kCJ7bKQgHxgQrket3-Dnk268sIdA7Qb8$
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <app/util/config.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLV.h>

namespace chip {
namespace scenes {

typedef struct fieldSets_s
{
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
    bool onOff;
#endif

#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
    uint8_t currentLevel;
    uint16_t currentFrequency;
#endif

#ifdef ZCL_USING_MODE_SELECT_CLUSTER_SERVER
    uint8_t currentMode;
#endif

#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
    uint8_t currentSaturation;
    uint16_t currentX;
    uint16_t currentY;
    uint16_t colorTemperatureMireds;
    uint16_t enhancedCurrentHue;
    uint8_t enhancedColorMode;
    uint8_t colorLoopActive;
    uint8_t colorLoopDirection;
    uint16_t colorLoopTime;
#endif

#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
    uint16_t occupiedCoolingSetpoint;
    uint16_t occupiedHeatingSetpoint;
    uint8_t systemMode;
#endif

#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
    uint8_t lockState;
#endif

#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
    uint8_t currentPositionLiftPercentage;
    uint8_t currentPositionTiltPercentage;
    uint8_t targetPositionLiftPercent100ths;
    uint8_t targetPositionTiltPercent100ths;
#endif
} fieldSets_t;

class ExtensionFieldsSets
{
public:
    static constexpr size_t kExentesionFieldsSetsSize = sizeof(fieldSets_t);
    static constexpr TLV::Tag TagEnabledFielsSets() { return TLV::ContextTag(1); }
    fieldSets_t enabledFieldSets;
    bool empty = false;

    ExtensionFieldsSets();
    virtual ~ExtensionFieldsSets() = default;

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const;
    CHIP_ERROR Deserialize(TLV::TLVReader & reader);

    void clear();

    bool operator==(const ExtensionFieldsSets & other)
    {
        return (!memcmp(&this->enabledFieldSets, &other.enabledFieldSets, kExentesionFieldsSetsSize));
    }

    void operator=(const ExtensionFieldsSets & other)
    {
        memcpy(&this->enabledFieldSets, &other.enabledFieldSets, kExentesionFieldsSetsSize);
    }
};
} // namespace scenes
} // namespace chip

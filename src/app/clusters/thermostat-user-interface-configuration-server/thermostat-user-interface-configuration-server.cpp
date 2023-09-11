/**
 *
 * SPDX-FileCopyrightText: 2021-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <app/util/af.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/ConcreteAttributePath.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip::app;
using namespace chip::app::Clusters::ThermostatUserInterfaceConfiguration;
using chip::Protocols::InteractionModel::Status;

namespace {
//
// Those types are defined as drafts in
// src/app/zap-templates/zcl/data-model/draft/types/thermostat-user-interface-configuration.xml
//
enum class TemperatureDisplayMode : uint8_t
{
    kCelsius    = 0x0,
    kFahrenheit = 0x1,
};

enum class KeypadLockout : uint8_t
{
    kNoLockout         = 0x0,
    kLevelOneLockout   = 0x1,
    kLevelTwoLockout   = 0x2,
    kLevelThreeLockout = 0x3,
    kLevelFourLockout  = 0x4,
    kLevelFiveLockout  = 0x5,
};

} // namespace

Status MatterThermostatUserInterfaceConfigurationClusterServerPreAttributeChangedCallback(
    const ConcreteAttributePath & attributePath, EmberAfAttributeType attributeType, uint16_t size, uint8_t * value)
{
    if (size != 0)
    {
        if (Attributes::TemperatureDisplayMode::Id == attributePath.mAttributeId)
        {
            auto mode = static_cast<TemperatureDisplayMode>(chip::Encoding::Get8(value));
            if ((TemperatureDisplayMode::kCelsius != mode) && (TemperatureDisplayMode::kFahrenheit != mode))
            {
                return Status::Failure;
            }
        }
        else if (Attributes::KeypadLockout::Id == attributePath.mAttributeId)
        {
            auto lockout = static_cast<KeypadLockout>(chip::Encoding::Get8(value));
            if ((KeypadLockout::kNoLockout != lockout) && (KeypadLockout::kLevelOneLockout != lockout) &&
                (KeypadLockout::kLevelTwoLockout != lockout) && (KeypadLockout::kLevelThreeLockout != lockout) &&
                (KeypadLockout::kLevelFourLockout != lockout) && (KeypadLockout::kLevelFiveLockout != lockout))
            {
                return Status::Failure;
            }
        }
        else if (Attributes::ScheduleProgrammingVisibility::Id == attributePath.mAttributeId)
        {
            auto prog = chip::Encoding::Get8(value);
            if (prog > 1)
            {
                return Status::Failure;
            }
        }
    }

    return Status::Success;
}

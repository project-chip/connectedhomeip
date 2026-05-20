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

#pragma once

#include <app/ConcreteAttributePath.h>
#include <app/util/attribute-storage.h>
#include <lib/core/Optional.h>
#include <protocols/interaction_model/Constants.h>

#include "Temperature.h"

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class Setpoint
{
public:
    virtual ~Setpoint() = default;

    virtual bool HasTemperature() const                                  = 0;
    virtual temperature Temperature() const                              = 0;
    virtual chip::app::Clusters::Thermostat::SystemModeEnum Mode() const = 0;
    virtual chip::AttributeId AttributeId() const                        = 0;
};

class BaseSetpoint : public Setpoint
{
public:
    BaseSetpoint(chip::AttributeId attributeId) : mAttributeId(attributeId) {}
    chip::AttributeId AttributeId() const override { return mAttributeId; }
    chip::app::Clusters::Thermostat::SystemModeEnum Mode() const override;

protected:
    chip::AttributeId mAttributeId;
};

class AbsoluteSetpoint : public BaseSetpoint
{
public:
    AbsoluteSetpoint(chip::AttributeId attributeId, temperature value) : BaseSetpoint(attributeId) { mTemperature = value; }

    AbsoluteSetpoint(const AbsoluteSetpoint & other) : BaseSetpoint(other.mAttributeId) { mTemperature = other.mTemperature; };

    bool HasTemperature() const override { return true; }
    temperature Temperature() const override { return mTemperature; }
    void SetTemperature(temperature temp) { mTemperature = temp; }

private:
    temperature mTemperature;
};

class OptionalSetpoint : public BaseSetpoint
{
public:
    OptionalSetpoint(chip::AttributeId attributeId) : BaseSetpoint(attributeId) {}
    OptionalSetpoint(chip::AttributeId attributeId, Optional<temperature> value) : BaseSetpoint(attributeId)
    {
        mTemperature = value;
    }

    OptionalSetpoint(const OptionalSetpoint & other) : BaseSetpoint(other.mAttributeId) { mTemperature = other.mTemperature; };

    bool HasTemperature() const override { return mTemperature.HasValue(); }
    temperature Temperature() const override { return mTemperature.Value(); }
    void SetTemperature(temperature temp) { mTemperature.SetValue(temp); }

private:
    Optional<temperature> mTemperature;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

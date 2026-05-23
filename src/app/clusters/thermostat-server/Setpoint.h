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

/*
 * The Setpoint class represents a setpoint value and its associated attributes.
 */
class Setpoint
{
public:
    virtual ~Setpoint() = default;

    /*
     * Return true if the setpoint has a temperature value, false otherwise.
     */
    virtual bool HasTemperature() const = 0;

    /*
     * Return the temperature value.
     */
    virtual temperature Temperature() const = 0;

    /*
     * Return the mode of the setpoint.
     */
    virtual SystemModeEnum Mode() const = 0;

    /*
     * Return the attribute id associated with the setpoint.
     */
    virtual chip::AttributeId AttributeId() const = 0;
};

/*
 * Base class for all setpoints, containing the attribute id
 */
class BaseSetpoint : public Setpoint
{
public:
    BaseSetpoint(chip::AttributeId attributeId) : mAttributeId(attributeId) {}
    chip::AttributeId AttributeId() const override { return mAttributeId; }
    SystemModeEnum Mode() const override;

protected:
    chip::AttributeId mAttributeId;
};

/*
 * The AbsoluteSetpoint class represents an absolute setpoint value.
 */
class AbsoluteSetpoint : public BaseSetpoint
{
public:
    AbsoluteSetpoint(chip::AttributeId attributeId, temperature value) : BaseSetpoint(attributeId) { mTemperature = value; }

    AbsoluteSetpoint(const AbsoluteSetpoint & other) : BaseSetpoint(other.mAttributeId) { mTemperature = other.mTemperature; };

    bool HasTemperature() const override { return true; }
    temperature Temperature() const override { return mTemperature; }
    bool SetTemperature(temperature temp); // returns true if the temperature was changed

private:
    temperature mTemperature;
};

/*
 * The OptionalSetpoint class represents an optional setpoint value. It is used for setpoints which can be optionally overridden by
 * the user
 */
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
    bool SetTemperature(temperature temp); // returns true if the temperature was changed

private:
    Optional<temperature> mTemperature;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

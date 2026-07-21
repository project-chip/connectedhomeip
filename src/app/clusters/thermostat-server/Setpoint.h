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

#include <app-common/zap-generated/cluster-objects.h>
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
    Setpoint(chip::AttributeId attributeId) : mAttributeId(attributeId) {}
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
     * Set the temperature value.
     * Returns true if the temperature was changed, false otherwise.
     */
    virtual bool SetTemperature(temperature temp) = 0;

    /*
     * Return the mode of the setpoint.
     */
    SystemModeEnum Mode() const;

    /*
     * Return the attribute id associated with the setpoint.
     */
    chip::AttributeId AttributeId() const { return mAttributeId; }

    bool operator==(const Setpoint & other) const
    {
        return mAttributeId == other.mAttributeId && HasTemperature() == other.HasTemperature() &&
            (!HasTemperature() || Temperature() == other.Temperature());
    }
    bool operator!=(const Setpoint & other) const { return !(*this == other); }

protected:
    chip::AttributeId mAttributeId;
};

/*
 * The AbsoluteSetpoint class represents an absolute setpoint value.
 */
class AbsoluteSetpoint : public Setpoint
{
public:
    AbsoluteSetpoint(chip::AttributeId attributeId, temperature value) : Setpoint(attributeId), mTemperature(value) {}
    AbsoluteSetpoint(const AbsoluteSetpoint & other) : Setpoint(other.mAttributeId), mTemperature(other.mTemperature) {}
    AbsoluteSetpoint & operator=(const AbsoluteSetpoint & other) = default;

    bool HasTemperature() const override { return true; }
    temperature Temperature() const override { return mTemperature; }

    /*
     * Set the temperature value.
     * Returns true if the temperature was changed, false otherwise.
     */
    bool SetTemperature(temperature temp) override;

private:
    temperature mTemperature;
};

/*
 * The OptionalSetpoint class represents an optional setpoint value. It is used for setpoints which can be optionally overridden by
 * the user
 */
class OptionalSetpoint : public Setpoint
{
public:
    OptionalSetpoint(chip::AttributeId attributeId, const AbsoluteSetpoint & absoluteSetpoint) :
        Setpoint(attributeId), mAbsoluteSetpoint(absoluteSetpoint)
    {}

    OptionalSetpoint(const OptionalSetpoint & other, const AbsoluteSetpoint & absoluteSetpoint) :
        Setpoint(other.mAttributeId), mAbsoluteSetpoint(absoluteSetpoint), mTemperature(other.mTemperature)
    {}

    OptionalSetpoint(const OptionalSetpoint & other) = default;
    OptionalSetpoint & operator=(const OptionalSetpoint & other)
    {
        if (this != &other)
        {
            Setpoint::operator=(other);
            mTemperature = other.mTemperature;
        }
        return *this;
    }

    bool HasTemperature() const override { return mTemperature.HasValue(); }
    temperature Temperature() const override;

    /*
     * Set the temperature value.
     * Returns true if the temperature was changed, false otherwise.
     */
    bool SetTemperature(temperature temp) override;

    /*
     * Clear the temperature override; this will cause Temperature() to return the absolute setpoint's temperature
     * Returns true if the temperature was cleared, false otherwise.
     */
    bool ClearTemperature();

private:
    const AbsoluteSetpoint & mAbsoluteSetpoint;
    Optional<temperature> mTemperature;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

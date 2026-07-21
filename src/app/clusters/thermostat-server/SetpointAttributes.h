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

#include <stdint.h>
#include <type_traits>
#include <utility>

#include <app/ConcreteAttributePath.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <protocols/interaction_model/Constants.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

/*
 * The SetpointAttributes class is used to keep track of which setpoints have been changed or fixed.
 *
 * Note: this relies heavily on the fact that all the setpoint attributes have IDs below 32, and can thus be used
 * to bit-shift into a flag map. This is NOT a generic class for tracking arbitrary attribute changes.
 */
class SetpointAttributes
{

public:
    /*
     * Set the flag for the given attribute.
     */
    SetpointAttributes & Set(chip::AttributeId attribute);

    /*
     * Set the flags for the given attributes.
     */
    SetpointAttributes & Set(const SetpointAttributes & other);

    /*
     * Check if the flag for the given attribute is set.
     */
    bool Has(chip::AttributeId attribute) const;

    /*
     * Check if any of the given attributes are set.
     */
    template <typename... Args>
    bool HasAny(Args... args) const
    {
        return (mValue & Or(args...)) != 0;
    }

    /*
     * Clear the flag for the given attribute.
     */
    SetpointAttributes & Clear(chip::AttributeId attribute);

    /*
     * Clear all the flags.
     */
    SetpointAttributes & ClearAll();

    bool Empty() const { return mValue == 0; }

    /*
     * Return the raw value of the setpoint attributes.
     */
    uint32_t Raw() const { return mValue; }

    /*
     * Return the first attribute that was set on this map.
     *
     * Note: this is temporary, to work around a limitation of the ember framework
     * that relies on a chain of callbacks when one attribute forces the change of
     * another. This code will be removed in the code-driven cluster revision of Thermostat
     */
    chip::AttributeId FirstDirtyAttribute() const { return mFirstDirtyAttribute; }

    /*
     * Clear the attribute that is returned by FirstDirtyAttribute().
     */
    void ClearFirstDirtyAttribute() { mFirstDirtyAttribute = chip::kInvalidAttributeId; }

private:
    template <typename... Args>
    static constexpr uint32_t Or(chip::AttributeId attribute, Args... args)
    {
        VerifyOrDieWithoutLogging(attribute < 32);
        return (1U << attribute) | Or(args...);
    }
    static constexpr uint32_t Or() { return 0; }

    uint32_t mValue = 0;

    chip::AttributeId mFirstDirtyAttribute = chip::kInvalidAttributeId;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

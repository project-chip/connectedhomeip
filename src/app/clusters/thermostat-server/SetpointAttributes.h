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
#include <protocols/interaction_model/Constants.h>

using namespace chip::app::Clusters::Thermostat::Attributes;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class SetpointAttributes {

    public:
    SetpointAttributes & Set(chip::AttributeId attribute)
    {
        mValue |= static_cast<uint32_t>(1 << attribute);
        return *this;
    }

    SetpointAttributes & Set(const SetpointAttributes & other)
    {
        mValue |= other.mValue;
        return *this;
    }

    bool Has(chip::AttributeId attribute) const
    {
        return (mValue & static_cast<uint32_t>(1 << attribute)) != 0;
    }

    template<typename... Args>
    bool HasAny(Args ... args) const
    {
        return (mValue & Or(std::forward<chip::AttributeId>(args)...)) != 0;
    }

    SetpointAttributes & Clear(chip::AttributeId attribute)
    {
        mValue &= ~static_cast<uint32_t>(1 << attribute);
        return *this;
    }

    SetpointAttributes & ClearAll()
    {
        mValue = 0;
        return *this;
    }

    uint32_t Raw() const { return mValue; }
    
    private:

    template<typename... Args>
    static constexpr uint32_t Or(chip::AttributeId attribute, Args ... args)
    {
        return static_cast<uint32_t>(1 << attribute) | Or(args...);
    }
    static constexpr uint32_t Or()
    {
        return 0;
    }

    uint32_t mValue = 0;
};

char const * SetpointAttributeName(chip::AttributeId id);

}
} // namespace Clusters
} // namespace app
} // namespace chip

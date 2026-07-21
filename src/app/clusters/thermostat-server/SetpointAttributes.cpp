/**
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

#include "SetpointAttributes.h"

using namespace chip::app::Clusters::Thermostat::Attributes;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

SetpointAttributes & SetpointAttributes::Set(chip::AttributeId attribute)
{
    VerifyOrDie(attribute < 32);
    mValue |= (1U << attribute);
    if (mFirstDirtyAttribute == chip::kInvalidAttributeId)
    {
        mFirstDirtyAttribute = attribute;
    }
    return *this;
}

SetpointAttributes & SetpointAttributes::Set(const SetpointAttributes & other)
{
    mValue |= other.mValue;
    if (mFirstDirtyAttribute == chip::kInvalidAttributeId)
    {
        mFirstDirtyAttribute = other.mFirstDirtyAttribute;
    }
    return *this;
}

bool SetpointAttributes::Has(chip::AttributeId attribute) const
{
    VerifyOrDie(attribute < 32);
    return (mValue & (1U << attribute)) != 0;
}

SetpointAttributes & SetpointAttributes::Clear(chip::AttributeId attribute)
{
    VerifyOrDie(attribute < 32);
    mValue &= ~(1U << attribute);
    if (mFirstDirtyAttribute == attribute)
    {
        mFirstDirtyAttribute = chip::kInvalidAttributeId;
    }
    return *this;
}

SetpointAttributes & SetpointAttributes::ClearAll()
{
    mValue               = 0;
    mFirstDirtyAttribute = chip::kInvalidAttributeId;
    return *this;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

/*
 * Copyright (c) 2026 Project CHIP Authors
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <app/clusters/dynamic-lighting-client/DynamicLightingClient.h>

#include <app/data-model/Decode.h>
#include <clusters/DynamicLighting/Attributes.h>
#include <lib/support/CodeUtils.h>

namespace chip::app::Clusters::DynamicLighting {

CHIP_ERROR DynamicLightingClient::HandleAttributeData(AttributeId attributeId, TLV::TLVReader & reader)
{
    VerifyOrReturnValue(mDelegate != nullptr, CHIP_NO_ERROR);

    switch (attributeId)
    {
    case Attributes::AvailableEffects::Id: {
        Attributes::AvailableEffects::TypeInfo::DecodableType availableEffects;
        ReturnErrorOnFailure(DataModel::Decode(reader, availableEffects));
        mDelegate->OnAvailableEffectsChanged(availableEffects);
        break;
    }
    case Attributes::CurrentEffectID::Id: {
        Attributes::CurrentEffectID::TypeInfo::DecodableType currentEffectId;
        ReturnErrorOnFailure(DataModel::Decode(reader, currentEffectId));
        mDelegate->OnCurrentEffectIDChanged(currentEffectId);
        break;
    }
    case Attributes::CurrentSpeed::Id: {
        Attributes::CurrentSpeed::TypeInfo::DecodableType currentSpeed;
        ReturnErrorOnFailure(DataModel::Decode(reader, currentSpeed));
        mDelegate->OnCurrentSpeedChanged(currentSpeed);
        break;
    }
    default:
        // Unknown or unhandled attribute; silently skip.
        break;
    }

    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters::DynamicLighting

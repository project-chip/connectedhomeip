/*
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

#include <access/SubjectDescriptor.h>
#include <devices/boolean-state-sensor/BooleanStateSensorAccessor.h>
#include <lib/support/CodeUtils.h>

using namespace chip::app::Clusters;

namespace chip::app {

BooleanStateSensorAccessor::BooleanStateSensorAccessor(BooleanStateSensorDevice * device) : mDevice(device)
{
    VerifyOrDie(mDevice != nullptr);
}

std::optional<CHIP_ERROR> BooleanStateSensorAccessor::HandleAction(CharSpan actionName, chip::TLV::TLVReader & arguments,
                                                                   const ConcreteDataAttributePath * path)
{
    if (!actionName.data_equal(kActionSetAttribute))
    {
        return std::nullopt;
    }

    if (path == nullptr)
    {
        return std::make_optional(CHIP_ERROR_INVALID_ARGUMENT);
    }

    if (path->mEndpointId != mDevice->GetEndpointId())
    {
        return std::nullopt; // Not for our endpoint
    }

    // arguments TLVReader is positioned at the value.
    // Reconstruct decoder.
    Access::SubjectDescriptor subjectDescriptor{ .authMode = chip::Access::AuthMode::kInternalDeviceAccess };
    AttributeValueDecoder decoder(arguments, subjectDescriptor);

    return SetAttribute(*path, decoder);
}

std::optional<CHIP_ERROR> BooleanStateSensorAccessor::SetAttribute(const ConcreteDataAttributePath & path,
                                                                   AttributeValueDecoder & decoder)
{
    if (path.mClusterId == BooleanState::Id)
    {
        switch (path.mAttributeId)
        {
        case BooleanState::Attributes::StateValue::Id: {
            bool stateValue;
            ReturnErrorOnFailure(decoder.Decode(stateValue));
            mDevice->BooleanState().SetStateValue(stateValue);
            return std::make_optional(CHIP_NO_ERROR);
        }
        default:
            break;
        }
    }

    return std::nullopt;
}

} // namespace chip::app

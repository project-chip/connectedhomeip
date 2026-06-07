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

#include <devices/boolean-state-sensor/BooleanStateSensorAccessor.h>

using namespace chip::app::Clusters;

namespace chip::app {

BooleanStateSensorAccessor::BooleanStateSensorAccessor(BooleanStateSensorDevice * device) : SingleEndpointDeviceAccessor(device) {}

CHIP_ERROR BooleanStateSensorAccessor::SetAttribute(const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder)
{
    auto * device = static_cast<BooleanStateSensorDevice *>(mDevice);
    if (device == nullptr)
    {
        return CHIP_ERROR_INTERNAL;
    }

    if (path.mClusterId == BooleanState::Id)
    {
        switch (path.mAttributeId)
        {
        case BooleanState::Attributes::StateValue::Id: {
            bool stateValue;
            ReturnErrorOnFailure(decoder.Decode(stateValue));
            device->BooleanState().SetStateValue(stateValue);
            return CHIP_NO_ERROR;
        }
        default:
            break;
        }
    }

    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace chip::app

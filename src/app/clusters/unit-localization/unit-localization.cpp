/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/attribute-metadata.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app::Clusters::UnitLocalization;

using chip::app::ConcreteAttributePath;
using Status = Protocols::InteractionModel::Status;

Status MatterUnitLocalizationClusterServerPreAttributeChangedCallback(const ConcreteAttributePath & attributePath,
                                                                      EmberAfAttributeType attributeType, uint16_t size,
                                                                      uint8_t * value)
{
    Protocols::InteractionModel::Status res;

    switch (attributePath.mAttributeId)
    {
    case Attributes::TemperatureUnit::Id:
        // value is based of an enum_8
        if ((*value != to_underlying(TempUnit::kFahrenheit)) && (*value != to_underlying(TempUnit::kCelsius)) &&
            (*value != to_underlying(TempUnit::kKelvin)))
        {
            res = Status::ConstraintError;
        }
        else
        {
            res = Status::Success;
        }
        break;
    default:
        res = Status::Success;
        break;
    }

    return res;
}

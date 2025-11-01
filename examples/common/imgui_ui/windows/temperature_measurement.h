/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "window.h"

#include <app/data-model/Nullable.h>
#include <app-common/zap-generated/cluster-enums.h>

#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>

#include <stdint.h>
#include <string>

namespace example {
namespace Ui {
namespace Windows {

class TemperatureMeasurement : public Window
{
public:
    TemperatureMeasurement(chip::EndpointId endpointId) : mEndpointId(endpointId), mTitle("Temperature measurement") {}

    void UpdateState() override;
    void Render() override;

private:
    const chip::EndpointId mEndpointId;
    const std::string mTitle;

    int16_t mTemperatureMeasurementValue;
    int16_t mTemperatureMeasurementMin;
    int16_t mTemperatureMeasurementMax;
    uint16_t mTemperatureMeasurementTolerance;

    chip::app::DataModel::Nullable<int16_t> mTargetTemperatureMeasurementValue;
    chip::app::DataModel::Nullable<int16_t> mTargetTemperatureMeasurementMin;
    chip::app::DataModel::Nullable<int16_t> mTargetTemperatureMeasurementMax;
    uint16_t mTargetTemperatureMeasurementTolerance;
};

} // namespace Windows
} // namespace Ui
} // namespace example

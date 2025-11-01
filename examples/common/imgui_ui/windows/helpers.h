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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app/data-model/Nullable.h>
#include <lib/core/Optional.h>
#include <lib/support/logging/CHIPLogging.h>

namespace example {
namespace Ui {
namespace Windows {

template <typename T>
void UpdateStateEnum(chip::EndpointId endpointId, T & targetValue, T & value,
                     chip::Protocols::InteractionModel::Status (*setter)(chip::EndpointId endpoint, T value),
                     chip::Protocols::InteractionModel::Status (*getter)(chip::EndpointId endpoint, T * value))
{
    if (targetValue != value)
    {
        setter(endpointId, targetValue);
    }

    auto status = getter(endpointId, &value);
    if (status != chip::Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ImGuiHelper::UpdateStateEnum: failed to get enum value: %d", chip::to_underlying(status));
    }

    targetValue = value;
}

template <typename T>
void UpdateStateReadOnly(chip::EndpointId endpointId, T & targetValue, T & value,
                         chip::Protocols::InteractionModel::Status (*getter)(chip::EndpointId endpoint, T * value))
{
    auto status = getter(endpointId, &value);
    if (status != chip::Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ImGuiHelper::UpdateStateReadOnly: failed to get read only value: %d",
                     chip::to_underlying(status));
    }

    targetValue = value;
}

template <typename T, typename V>
void UpdateStateNullable(chip::EndpointId endpointId, chip::app::DataModel::Nullable<T> & targetValue, T & value,
                         chip::Protocols::InteractionModel::Status (*setter)(chip::EndpointId endpoint, V value),
                         chip::Protocols::InteractionModel::Status (*getter)(chip::EndpointId endpoint,
                                                                             chip::app::DataModel::Nullable<V> & value))
{
    if (!targetValue.IsNull())
    {
        setter(endpointId, targetValue.Value());
        targetValue.SetNull();
    }

    chip::app::DataModel::Nullable<T> result{};
    auto status = getter(endpointId, result);
    if (status != chip::Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ImGuiHelper::UpdateStateNullable: failed to get writable value: %d",
                     chip::to_underlying(status));
    }

    if (!result.IsNull())
    {
        value = result.Value();
    }
}

template <typename T, typename V>
void UpdateStateNullable(chip::EndpointId endpointId, chip::app::DataModel::Nullable<T> & targetValue, T & value,
                         chip::Protocols::InteractionModel::Status (*getter)(chip::EndpointId endpoint,
                                                                             chip::app::DataModel::Nullable<V> & value))
{
    chip::app::DataModel::Nullable<T> result{};
    auto status = getter(endpointId, result);
    if (status != chip::Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ImGuiHelper::UpdateStateNullable: failed to get read only value: %d",
                     chip::to_underlying(status));
    }

    value = result.Value();
}


} // namespace Windows
} // namespace Ui
} // namespace example

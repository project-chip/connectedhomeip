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

#include <stdint.h>
#include <string>

#include <app/data-model/Nullable.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>

#include <app-common/zap-generated/cluster-enums.h>

namespace example {
namespace Ui {
namespace Windows {

class OccupancySensing : public Window
{
public:
    OccupancySensing(chip::EndpointId endpointId, const char * title) : mEndpointId(endpointId), mTitle(title) {}

    void UpdateState() override;
    void Render() override;

private:
    const chip::EndpointId mEndpointId;
    const std::string mTitle;

    using BitMask = chip::BitMask<chip::app::Clusters::OccupancySensing::OccupancyBitmap>;

    BitMask mOccupancy;
    chip::Optional<BitMask> mTargetOccupancy;
};

} // namespace Windows
} // namespace Ui
} // namespace example

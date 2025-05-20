/*
 *
 * Copyright (c) 2025 Project CHIP Authors
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/ConcreteAttributePath.h>
#include <cstdint>
#include <memory>
#include <protocols/interaction_model/Constants.h>

namespace chip {
namespace app {
namespace Clusters {

class MicrowaveOvenControlServer;

namespace MicrowaveOvenControl {

class MicrowaveOvenControlServer
{

public:
    MicrowaveOvenControlServer() = default;
    ~MicrowaveOvenControlServer() = default;

    MicrowaveOvenControlServer(const MicrowaveOvenControlServer &) = delete;
    MicrowaveOvenControlServer & operator=(const MicrowaveOvenControlServer &) = delete;
    MicrowaveOvenControlServer(MicrowaveOvenControlServer &&) = delete;
    MicrowaveOvenControlServer & operator=(MicrowaveOvenControlServer &&) = delete;

    CHIP_ERROR Init();

    CHIP_ERROR GetCookPower(uint16_t & value);
    CHIP_ERROR GetPowerSetting(uint8_t & value);
    uint16_t GetMinCookPowerConstant() const { return mMinCookPower; }
    uint16_t GetMaxCookPowerConstant() const { return mMaxCookPower; }
    uint16_t GetCookPowerStepConstant() const { return mCookPowerStep; }
    uint32_t GetFeatureMap(EndpointId endpoint);

    Protocols::InteractionModel::Status SetCookPower(uint16_t cookPower);
    Protocols::InteractionModel::Status SetPowerSetting(uint8_t powerSetting);

private:
    EndpointId mEndpointId = kInvalidEndpointId;

    uint16_t mMinCookPower  = 10;
    uint16_t mMaxCookPower  = 100;
    uint16_t mCookPowerStep = 10;
};

void Shutdown();

} // namespace MicrowaveOvenControl
} // namespace Clusters
} // namespace app
} // namespace chip

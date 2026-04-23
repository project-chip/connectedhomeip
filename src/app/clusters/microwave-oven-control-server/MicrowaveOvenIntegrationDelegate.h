/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include <protocols/interaction_model/StatusCode.h>

#include <cstdint>

namespace chip::app::Clusters::MicrowaveOvenControl {

class MicrowaveOvenIntegrationDelegate
{
public:
    MicrowaveOvenIntegrationDelegate()          = default;
    virtual ~MicrowaveOvenIntegrationDelegate() = default;

    virtual uint8_t GetCurrentOperationalState() const                                       = 0;
    virtual Protocols::InteractionModel::Status GetNormalOperatingMode(uint8_t & mode) const = 0;
    virtual Protocols::InteractionModel::Status IsSupportedMode(uint8_t mode) const          = 0;
};

} // namespace chip::app::Clusters::MicrowaveOvenControl

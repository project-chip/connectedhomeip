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

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Span.h>
#include <oob-accessors/OOBAccessor.h>
#include <optional>

namespace chip::app {

// Abstract simulation delegate for Doorbell actions handled out-of-band.
class DoorbellSimulationDelegate
{
public:
    virtual ~DoorbellSimulationDelegate() = default;

    virtual CHIP_ERROR HandleShortPress()                                = 0;
    virtual CHIP_ERROR HandleSetCurrentPosition(uint8_t currentPosition) = 0;
};

class DoorbellOOBAccessor : public OOBAccessor
{
public:
    DoorbellOOBAccessor(DoorbellSimulationDelegate & delegate, EndpointId endpointId) : mDelegate(delegate), mEndpointId(endpointId)
    {}

    std::optional<CHIP_ERROR> HandleAction(CharSpan action, ByteSpan tlvData) override;

private:
    std::optional<CHIP_ERROR> HandleShortPress(ByteSpan tlvData) const;
    std::optional<CHIP_ERROR> HandleSetCurrentPosition(ByteSpan tlvData) const;

    DoorbellSimulationDelegate & mDelegate;
    EndpointId mEndpointId;
};

} // namespace chip::app

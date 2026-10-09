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

#include <optional>

#include <app/ConcreteAttributePath.h>
#include <device/types/water-valve/WaterValve.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <lib/support/Span.h>
#include <oob-accessors/OOBAccessor.h>

namespace chip::app {

class WaterValveAccessor : public OOBAccessor
{
public:
    explicit WaterValveAccessor(WaterValve & device) : mDevice(device) {}
    ~WaterValveAccessor() override = default;

    std::optional<CHIP_ERROR> HandleAction(CharSpan actionName, ByteSpan tlvBuffer) override;

private:
    std::optional<CHIP_ERROR> SetAttribute(const ConcreteDataAttributePath & path, TLV::TLVReader & reader);

    WaterValve & mDevice;
};

} // namespace chip::app

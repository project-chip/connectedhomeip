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

#include <lib/support/ScopedMemoryBuffer.h>
#include <lib/support/logging/CHIPLogging.h>
#include <oob-accessors/OOBAccessorRegistry.h>
#include <oob-accessors/OOBDataSerializer.h>
#include <pigweed/rpc_services/AccessInterceptor.h>

namespace chip::app {

class PigweedAttributeAccessor : public chip::rpc::PigweedDebugAccessInterceptor
{
public:
    PigweedAttributeAccessor()           = default;
    ~PigweedAttributeAccessor() override = default;

    std::optional<::pw::Status> Write(const ConcreteDataAttributePath & path, const TLV::TLVReader & reader) override
    {
        auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
        if (std::holds_alternative<CHIP_ERROR>(buildResult))
        {
            CHIP_ERROR err = std::get<CHIP_ERROR>(buildResult);
            ChipLogError(Support, "Failed to build OOB SetAttribute request: %" CHIP_ERROR_FORMAT, err.Format());
            return ::pw::Status::Internal();
        }

        auto & buffer     = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);
        CHIP_ERROR result = OOBAccessorRegistry::Instance().HandleAction("SetAttribute"_span, buffer);
        if (result != CHIP_ERROR_NOT_FOUND)
        {
            if (result == CHIP_NO_ERROR)
            {
                return ::pw::OkStatus();
            }
            ChipLogError(Support, "OOB Accessor failed to write attribute: %" CHIP_ERROR_FORMAT, result.Format());
            return ::pw::Status::Internal();
        }

        return std::nullopt;
    }
};

} // namespace chip::app

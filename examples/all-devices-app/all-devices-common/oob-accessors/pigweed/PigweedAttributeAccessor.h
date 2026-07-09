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
#include <oob-accessors/OOBInfoAccessor.h>
#include <pigweed/rpc_services/AccessInterceptor.h>

namespace chip::app {

class PigweedAttributeAccessor : public chip::rpc::PigweedDebugAccessInterceptor, public chip::rpc::PigweedDebugInfoInterceptor
{
public:
    PigweedAttributeAccessor()           = default;
    ~PigweedAttributeAccessor() override = default;

    std::variant<::pw::Status, chip::ReadOnlyBuffer<ConcreteDataAttributePath>> GetSupportedWritePaths() override
    {
        auto actionResult =
            OOBAccessorRegistry::Instance().HandleAction(OOBInfoAccessor::kActionGetAllSupportedSetAttributes, ByteSpan());

        if (actionResult.first != CHIP_NO_ERROR)
        {
            ChipLogError(Support, "Failed to retrieve OOB supported write paths: %" CHIP_ERROR_FORMAT, actionResult.first.Format());
            return ::pw::Status::Internal();
        }

        auto & tlvResponse     = actionResult.second;
        auto deserializeResult = OOBDataSerializer::DeSerializePathsList(ByteSpan(tlvResponse.data(), tlvResponse.size()));

        if (std::holds_alternative<CHIP_ERROR>(deserializeResult))
        {
            CHIP_ERROR err = std::get<CHIP_ERROR>(deserializeResult);
            ChipLogError(Support, "Failed to deserialize OOB supported paths: %" CHIP_ERROR_FORMAT, err.Format());
            return ::pw::Status::Internal();
        }

        return std::move(std::get<ReadOnlyBuffer<ConcreteDataAttributePath>>(deserializeResult));
    }

    std::optional<::pw::Status> Write(const ConcreteDataAttributePath & path, const TLV::TLVReader & reader) override
    {
        auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
        if (std::holds_alternative<CHIP_ERROR>(buildResult))
        {
            CHIP_ERROR err = std::get<CHIP_ERROR>(buildResult);
            ChipLogError(Support, "Failed to build OOB SetAttribute request: %" CHIP_ERROR_FORMAT, err.Format());
            return ::pw::Status::Internal();
        }

        auto & buffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);
        auto result   = OOBAccessorRegistry::Instance().HandleAction(SetAttributeAccessor::kActionSetAttribute, buffer);
        if (result.first != CHIP_ERROR_NOT_FOUND)
        {
            if (result.first == CHIP_NO_ERROR)
            {
                return ::pw::OkStatus();
            }
            ChipLogError(Support, "OOB Accessor failed to write attribute: %" CHIP_ERROR_FORMAT, result.first.Format());
            return ::pw::Status::Internal();
        }

        return std::nullopt;
    }
};

} // namespace chip::app

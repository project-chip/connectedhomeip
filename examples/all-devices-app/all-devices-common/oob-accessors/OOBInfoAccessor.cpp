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

#include <oob-accessors/OOBAccessorRegistry.h>
#include <oob-accessors/OOBDataSerializer.h>
#include <oob-accessors/OOBInfoAccessor.h>

namespace chip::app {

std::optional<OOBAccessor::ActionResponse> OOBInfoAccessor::HandleAction(CharSpan actionName, ByteSpan tlvBuffer)
{
    if (actionName.data_equal(kActionGetAllSupportedSetAttributes))
    {
        return HandleGetAllSupportedSetAttributes(tlvBuffer);
    }
    return std::nullopt;
}

std::optional<OOBAccessor::ActionResponse> OOBInfoAccessor::HandleGetAllSupportedSetAttributes(ByteSpan tlvBuffer)
{
    ReadOnlyBufferBuilder<ConcreteDataAttributePath> builder;

    for (auto & accessor : OOBAccessorRegistry::Instance().GetAccessors())
    {
        if (!accessor.IsAccessorType(OOBAccessorType::kSetAttribute))
        {
            continue;
        }
        auto * setAttrAccessor = static_cast<SetAttributeAccessor *>(&accessor);
        CHIP_ERROR err         = builder.AppendElements(setAttrAccessor->GetSupportedPaths());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Support, "Failed to append paths to builder: %" CHIP_ERROR_FORMAT, err.Format());
            return ActionResponse(err, ReadOnlyBuffer<uint8_t>());
        }
    }

    auto pathsBuffer     = builder.TakeBuffer();
    auto serializeResult = OOBDataSerializer::SerializePathsList(pathsBuffer);
    if (std::holds_alternative<CHIP_ERROR>(serializeResult))
    {
        CHIP_ERROR err = std::get<CHIP_ERROR>(serializeResult);
        ChipLogError(Support, "Failed to serialize paths list: %" CHIP_ERROR_FORMAT, err.Format());
        return ActionResponse(err, ReadOnlyBuffer<uint8_t>());
    }

    return ActionResponse(CHIP_NO_ERROR, std::move(std::get<ReadOnlyBuffer<uint8_t>>(serializeResult)));
}

} // namespace chip::app

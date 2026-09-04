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

#include <limits>
#include <optional>
#include <type_traits>

#include <json/json.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <oob-accessors/OOBAccessorRegistry.h>

namespace chip::app::NamedPipe {

class CommandTranslator
{
public:
    virtual ~CommandTranslator() = default;

    /**
     * @brief Translates a JSON payload into TLV and executes the action via OOBAccessorRegistry.
     * @param endpointId Target endpoint ID extracted by dispatcher.
     * @param json Parsed JSON payload from named pipe.
     * @param registry Target registry to dispatch the translated action.
     */
    virtual CHIP_ERROR TranslateAndExecute(EndpointId endpointId, const Json::Value & json,
                                           OOBAccessorRegistry & registry) const = 0;

    static std::optional<bool> ExtractBool(const Json::Value & json, const char * key)
    {
        if (!json.isObject() || !json.isMember(key))
        {
            return std::nullopt;
        }
        if (json[key].isBool())
        {
            return json[key].asBool();
        }
        if (json[key].isIntegral())
        {
            return json[key].asInt64() != 0;
        }
        return std::nullopt;
    }

    template <typename T>
    static std::optional<T> ExtractUInt(const Json::Value & json, const char * key)
    {
        static_assert(std::is_unsigned_v<T>, "ExtractUInt requires an unsigned integer type");
        if (!json.isObject() || !json.isMember(key) || !json[key].isIntegral())
        {
            return std::nullopt;
        }
        if (json[key].isUInt64())
        {
            auto val = json[key].asUInt64();
            if (val > std::numeric_limits<T>::max())
            {
                return std::nullopt;
            }
            return static_cast<T>(val);
        }
        if (json[key].isInt64())
        {
            auto val = json[key].asInt64();
            if (val < 0 || static_cast<uint64_t>(val) > std::numeric_limits<T>::max())
            {
                return std::nullopt;
            }
            return static_cast<T>(val);
        }
        return std::nullopt;
    }

    static CHIP_ERROR DispatchAction(OOBAccessorRegistry & registry, CharSpan actionName, EndpointId endpointId)
    {
        uint8_t buffer[32];
        TLV::TLVWriter writer;
        writer.Init(buffer, sizeof(buffer));

        TLV::TLVType outerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(1), endpointId));
        ReturnErrorOnFailure(writer.EndContainer(outerType));
        ReturnErrorOnFailure(writer.Finalize());

        return registry.HandleAction(actionName, ByteSpan(buffer, writer.GetLengthWritten()));
    }

    template <typename T>
    static CHIP_ERROR DispatchAction(OOBAccessorRegistry & registry, CharSpan actionName, EndpointId endpointId, const T & value)
    {
        uint8_t buffer[64];
        TLV::TLVWriter writer;
        writer.Init(buffer, sizeof(buffer));

        TLV::TLVType outerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(1), endpointId));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(2), value));
        ReturnErrorOnFailure(writer.EndContainer(outerType));
        ReturnErrorOnFailure(writer.Finalize());

        return registry.HandleAction(actionName, ByteSpan(buffer, writer.GetLengthWritten()));
    }

    static CHIP_ERROR DispatchStringAction(OOBAccessorRegistry & registry, CharSpan actionName, EndpointId endpointId,
                                           CharSpan value)
    {
        uint8_t buffer[256];
        TLV::TLVWriter writer;
        writer.Init(buffer, sizeof(buffer));

        TLV::TLVType outerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(1), endpointId));
        ReturnErrorOnFailure(writer.PutString(TLV::ContextTag(2), value));
        ReturnErrorOnFailure(writer.EndContainer(outerType));
        ReturnErrorOnFailure(writer.Finalize());

        return registry.HandleAction(actionName, ByteSpan(buffer, writer.GetLengthWritten()));
    }
};

} // namespace chip::app::NamedPipe

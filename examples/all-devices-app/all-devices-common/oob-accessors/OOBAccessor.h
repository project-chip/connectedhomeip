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
#include <utility>

#include <app/ConcreteAttributePath.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <lib/support/IntrusiveList.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/ScopedMemoryBuffer.h>
#include <lib/support/Span.h>

namespace chip::app {

/**
 * @brief Identifies the specific accessor subclass type at runtime.
 *
 * This enum is used in place of Run-Time Type Information (RTTI) to allow
 * dynamic type identification and safe downcasting at runtime.
 */
enum class OOBAccessorType
{
    kGeneric,
    kSetAttribute
};

class OOBAccessor : public chip::IntrusiveListNodeBase<chip::IntrusiveMode::AutoUnlink>
{
public:
    virtual ~OOBAccessor() = default;

    virtual bool IsAccessorType(OOBAccessorType type) const { return type == OOBAccessorType::kGeneric; }

    using ActionResponse = std::pair<CHIP_ERROR, ReadOnlyBuffer<uint8_t>>;

    /**
     * @brief Handles a generic out-of-band action.
     *
     * @param actionName The name of the action to invoke.
     * @param tlvBuffer Buffer containing TLV data for the action request.
     * @return std::nullopt if the action is not handled/supported by this accessor.
     *         A non-null optional containing:
     *           - CHIP_ERROR:
     *               - CHIP_NO_ERROR on success.
     *               - Other CHIP_ERROR codes except CHIP_ERROR_NOT_FOUND on failure.
     *           - A ReadOnlyBuffer<uint8_t> containing response TLV data (empty if no data is returned).
     *
     * @note **Asynchronous Safety Warning:** The `tlvBuffer` parameter is a non-owning,
     *       temporary view whose underlying memory is only guaranteed to be valid during the
     *       synchronous execution of this function call.
     */
    virtual std::optional<ActionResponse> HandleAction(CharSpan actionName, ByteSpan tlvBuffer) = 0;
};

class SetAttributeAccessor : public OOBAccessor
{
public:
    static constexpr CharSpan kActionSetAttribute = "SetAttribute"_span;

    bool IsAccessorType(OOBAccessorType type) const override
    {
        return type == OOBAccessorType::kSetAttribute || OOBAccessor::IsAccessorType(type);
    }

    ~SetAttributeAccessor() override = default;

    /**
     * @brief Returns a list of concrete data attribute paths that this accessor supports writing to.
     *
     * @note **Asynchronous Safety Warning:** The returned Span is a non-owning, temporary view
     *       whose underlying memory is only guaranteed to be valid during the synchronous
     *       execution of this function call.
     */
    virtual Span<const ConcreteDataAttributePath> GetSupportedPaths() = 0;
};

} // namespace chip::app

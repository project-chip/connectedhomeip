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
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <lib/support/IntrusiveList.h>
#include <lib/support/ScopedMemoryBuffer.h>
#include <lib/support/Span.h>

namespace chip::app {

class OOBAccessor : public chip::IntrusiveListNodeBase<chip::IntrusiveMode::AutoUnlink>
{
public:
    virtual ~OOBAccessor() = default;

    /**
     * @brief Handles a generic out-of-band action.
     *
     * @param actionName The name of the action to invoke.
     * @param tlvBuffer Buffer containing TLV data for the action request.
     * @return std::nullopt if the action is not handled/supported by this accessor.
     *         A non-null optional containing:
     *           - CHIP_NO_ERROR on success.
     *           - Other CHIP_ERROR codes except CHIP_ERROR_NOT_FOUND on failure.
     *
     * @note **Asynchronous Safety Warning:** The `tlvBuffer` parameter is a non-owning,
     *       temporary view whose underlying memory is only guaranteed to be valid during the
     *       synchronous execution of this function call.
     */
    virtual std::optional<CHIP_ERROR> HandleAction(CharSpan actionName, ByteSpan tlvBuffer) = 0;
};

} // namespace chip::app

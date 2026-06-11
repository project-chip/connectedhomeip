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
#include <lib/support/Span.h>

namespace chip::app {

class OOBAccessor : public chip::IntrusiveListNodeBase<chip::IntrusiveMode::AutoUnlink>
{
public:
    virtual ~OOBAccessor() = default;

    static constexpr CharSpan kActionSetAttribute = chip::literals::operator""_span("SetAttribute", 12);

    /**
     * @brief Handles a generic out-of-band action.
     *
     * @param actionName The name of the action to invoke.
     * @param arguments A TLVReader positioned at the arguments for the action.
     * @param path Optional pointer to ConcreteDataAttributePath for attribute writes.
     * @return std::nullopt if the action is not handled/supported by this accessor.
     *         A non-null optional containing:
     *           - CHIP_NO_ERROR on success.
     *           - Other CHIP_ERROR codes on failure.
     */
    virtual std::optional<CHIP_ERROR> HandleAction(CharSpan actionName, chip::TLV::TLVReader & arguments,
                                                   const ConcreteDataAttributePath * path = nullptr) = 0;
};

} // namespace chip::app

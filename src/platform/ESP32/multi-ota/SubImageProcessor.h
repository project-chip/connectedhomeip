/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include "MultiOTAImageHeader.h"
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

namespace chip {

enum class DeviceState : uint8_t
{
    kUnknown,
    kReady,           // proceed — Dispatcher calls Init() then Write()
    kNotReady,        // skip — component unavailable; blocks softwareVersion confirmation
    kAlreadyUpToDate, // skip — component already at targetVersion; counts as verified
};

enum class AbortReason : uint8_t
{
    kUnknown,
    kError,
    kCancelled,
};

struct AbortContext
{
    AbortReason reason = AbortReason::kUnknown;
    CHIP_ERROR error = CHIP_NO_ERROR;
};

class SubImageProcessor
{
public:
    virtual ~SubImageProcessor() = default;

    virtual CHIP_ERROR Init(const SubImageHeader & entry) = 0;

    virtual bool IsInitialized() = 0;

    virtual CHIP_ERROR IsReadyForOTA(DeviceState & state) = 0;

    virtual CHIP_ERROR Write(ByteSpan & block) = 0;

    virtual void Abort(AbortContext & context) = 0;

    virtual CHIP_ERROR Apply() { return CHIP_NO_ERROR; }
};

} // namespace chip

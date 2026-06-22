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
    CHIP_ERROR error   = CHIP_NO_ERROR;
};

/**
 * @brief Handles one component's binary within a multi-image OTA bundle.
 *
 * The application implements one per registered image ID; the dispatcher routes that component's
 * bytes to it. All methods run on the Matter thread and must not block.
 */
class SubImageProcessor
{
public:
    virtual ~SubImageProcessor() = default;

    /**
     * @brief Prepare to receive this component's binary. Called once, before IsReadyForOTA().
     *
     * @param entry  Header for this component (image ID, version, length, SHA-256). Valid only for
     *               the duration of the call.
     * @retval CHIP_NO_ERROR  Ready to proceed.
     * @retval other          Aborts the session.
     */
    virtual CHIP_ERROR Init(const SubImageHeader & entry) = 0;

    /**
     * @brief Whether Init() has run (the processor is mid-transfer).
     * @return true if initialised; false otherwise.
     */
    virtual bool IsInitialized() = 0;

    /**
     * @brief Report readiness for this OTA cycle. .
     *
     * @param[out] state  kReady (stream via Write()), kAlreadyUpToDate or kNotReady (skip).
     * @retval CHIP_NO_ERROR  state was set.
     * @retval other          Aborts the session.
     */
    virtual CHIP_ERROR IsReadyForOTA(DeviceState & state) = 0;

    /**
     * @brief Consume one ordered chunk of the component's binary. Called only when kReady.
     *
     * @param block  Raw binary bytes (no header); valid only for the duration of the call.
     *               Chunks total exactly entry.length bytes across all calls.
     * @retval CHIP_NO_ERROR  Chunk accepted.
     * @retval other          Aborts the session.
     */
    virtual CHIP_ERROR Write(ByteSpan & block) = 0;

    /**
     * @brief Tear down an aborted session. Called on every Init()'d processor.
     *
     * @param context  reason = kError (with context.error) or kCancelled.
     */
    virtual void Abort(AbortContext & context) = 0;

    /**
     * @brief Commit the component during the apply phase. Called once per Init()'d processor.
     *
     * Default is a no-op (components flashed during Write() need nothing here).
     * @retval CHIP_NO_ERROR  Commit succeeded.
     * @retval other          Commit failed.
     */
    virtual CHIP_ERROR Apply() { return CHIP_NO_ERROR; }
};

} // namespace chip

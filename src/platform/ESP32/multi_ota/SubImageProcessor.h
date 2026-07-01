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
 * @brief Handles a single sub-image in a multi-image OTA bundle.
 */
class SubImageProcessor
{
public:
    virtual ~SubImageProcessor() = default;

    /**
     * @brief Initializes the processor for a sub-image.
     *
     * Called once before IsReadyForOTA().
     *
     * @param entry Header for the sub-image. Valid only for the duration of
     *              the call.
     * @retval CHIP_NO_ERROR on success; otherwise an error.
     */
    virtual CHIP_ERROR Init(const SubImageHeader & entry) = 0;

    /**
     * @brief Returns whether the processor has been initialized.
     */
    virtual bool IsInitialized() = 0;

    /**
     * @brief Reports whether the processor is ready for this OTA update.
     *
     * @param[out] state Receives the processor's OTA state.
     * @retval CHIP_NO_ERROR on success; otherwise an error.
     */
    virtual CHIP_ERROR IsReadyForOTA(DeviceState & state) = 0;

    /**
     * @brief Writes the next chunk of the sub-image.
     *
     * @param block Payload bytes for the sub-image. Valid only for the
     *              duration of the call.
     * @retval CHIP_NO_ERROR on success; otherwise an error.
     */
    virtual CHIP_ERROR Write(ByteSpan & block) = 0;

    /**
     * @brief Finalizes the sub-image after the last chunk has been written.
     *
     * Called once after all bytes have been received and verified.
     *
     * @retval CHIP_NO_ERROR on success; otherwise an error.
     */
    virtual CHIP_ERROR Finish() { return CHIP_NO_ERROR; }

    /**
     * @brief Aborts an in-progress OTA session.
     *
     * Called for every initialized processor.
     *
     * @param context Indicates whether the session was cancelled or aborted
     *                due to an error.
     */
    virtual void Abort(AbortContext & context) = 0;

    /**
     * @brief Applies the sub-image during the apply phase.
     *
     * Called once for every initialized processor.
     *
     * @retval CHIP_NO_ERROR on success; otherwise an error.
     */
    virtual CHIP_ERROR Apply() { return CHIP_NO_ERROR; }
};

} // namespace chip

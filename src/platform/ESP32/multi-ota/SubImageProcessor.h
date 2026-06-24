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
 */
class SubImageProcessor
{
public:
    virtual ~SubImageProcessor() = default;

    /**
     * @brief Prepare to receive this component's binary. Called once, before IsReadyForOTA().
     *
     * @param entry  Image header for this sub-image. Valid only for
     *               the duration of the call.
     * @retval CHIP_NO_ERROR if initialization was successful; other error code otherwise.
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
     * @param[out] state  reporting state of the device for this OTA cycle.
     * @retval CHIP_NO_ERROR if the operation was successful; other error code otherwise.
     */
    virtual CHIP_ERROR IsReadyForOTA(DeviceState & state) = 0;

    /**
     * @brief Consume one ordered chunk of this sub-image's binary.
     *
     * The bytes are ready to write to the component: the dispatcher has already verified the
     * on-wire integrity, and any transform helper layered in front (decryption, delta patching)
     * has already run, so an implementation only writes them to its target.
     *
     * @param block  Binary bytes for this component; valid only for the duration of the call.
     * @retval CHIP_NO_ERROR if the chunk was accepted; other error code otherwise.
     */
    virtual CHIP_ERROR Write(ByteSpan & block) = 0;

    /**
     * @brief Close the component's image after the final chunk. Called once by the dispatcher when
     *        all of this sub-image's bytes have been delivered and its integrity has been verified.
     *
     * @retval CHIP_NO_ERROR if the image was closed successfully; other error code otherwise.
     */
    virtual CHIP_ERROR Finish() { return CHIP_NO_ERROR; }

    /**
     * @brief Tear down an aborted session. Called on every Initialized processor.
     *
     * @param context  reason = kError (with context.error) or kCancelled.
     */
    virtual void Abort(AbortContext & context) = 0;

    /**
     * @brief Commit the component during the apply phase. Called once per Initialized processor.
     *
     * @retval CHIP_NO_ERROR if the commit was successful; other error code otherwise.
     */
    virtual CHIP_ERROR Apply() { return CHIP_NO_ERROR; }
};

} // namespace chip

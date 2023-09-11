/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 *    @file
 *      This file defines builder in CHIP interaction model
 *
 */

#pragma once

#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
class Builder
{
public:
    /**
     *  @brief Initialize the Builder object with TLVWriter and ContainerType
     *
     *  @param [in] apWriter A pointer to a TLVWriter
     *  @param [in] aOuterContainerType outer container type
     *
     */
    void Init(chip::TLV::TLVWriter * const apWriter, chip::TLV::TLVType aOuterContainerType);

    /**
     *  @brief Reset the Error
     *
     */
    void ResetError();

    /**
     *  @brief Reset the Error with particular aErr.
     *  @param [in] aErr the Error it would be reset with
     *
     */
    void ResetError(CHIP_ERROR aErr);

    /**
     *  @brief Get current error
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR GetError() const { return mError; };

    /**
     *  @brief Get TLV Writer
     *
     *  @return #Pointer to the TLVWriter
     */
    chip::TLV::TLVWriter * GetWriter() { return mpWriter; };

    /**
     * Checkpoint the current tlv state into a TLVWriter
     *
     * @param[out] aPoint A writer to checkpoint the state of the TLV writer into.
     *                    This writer must not outlive the builder
     */
    void Checkpoint(chip::TLV::TLVWriter & aPoint) { aPoint = *mpWriter; };

    /**
     * Rollback the request state to the checkpointed TLVWriter
     *
     * @param[in] aPoint A writer that captured the state via Checkpoint() at some point in the past
     */
    void Rollback(const chip::TLV::TLVWriter & aPoint)
    {
        *mpWriter = aPoint;
        ResetError();
    }

    void EndOfContainer();

    Builder(Builder &) = delete;

protected:
    CHIP_ERROR mError;
    chip::TLV::TLVWriter * mpWriter;
    chip::TLV::TLVType mOuterContainerType;

    Builder();
    CHIP_ERROR InitAnonymousStructure(chip::TLV::TLVWriter * const apWriter);
};
} // namespace app
} // namespace chip

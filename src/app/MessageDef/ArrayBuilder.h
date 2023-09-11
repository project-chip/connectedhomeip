/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "Builder.h"
#include "Parser.h"
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
class ArrayBuilder : public Builder
{
public:
    /**
     * Init the TLV array container with an particular context tag.
     * Required to implement arrays of arrays, and to test ArrayBuilder.
     *
     * @param[in]   apWriter    Pointer to the TLVWriter that is encoding the message.
     * @param[in]   aContextTagToUse    A contextTag to use.
     *
     * @return                  CHIP_ERROR codes returned by chip::TLV objects.
     */

    CHIP_ERROR Init(TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse);
    /**
     * Init the TLV array container with an anonymous tag.
     * Required to implement arrays of arrays, and to test ArrayBuilder.
     *
     * @param[in]   apWriter    Pointer to the TLVWriter that is encoding the message.
     *
     * @return                  CHIP_ERROR codes returned by chip::TLV objects.
     */
    CHIP_ERROR Init(TLV::TLVWriter * const apWriter);
};
} // namespace app
} // namespace chip

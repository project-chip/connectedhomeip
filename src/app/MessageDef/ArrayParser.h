/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "Parser.h"

namespace chip {
namespace app {
class ArrayParser : public Parser
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should be on the element of the array element
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(const TLV::TLVReader & aReader);
};
} // namespace app
} // namespace chip

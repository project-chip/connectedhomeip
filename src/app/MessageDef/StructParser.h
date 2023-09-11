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
class StructParser : public Parser
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should be on the element of the struct element
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(const TLV::TLVReader & aReader);

    CHIP_ERROR CheckSchemaOrdering() const;
};
} // namespace app
} // namespace chip

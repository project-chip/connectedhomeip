/**
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "PICSNormalizer.h"

#include <algorithm>
#include <cctype>

std::string PICSNormalizer::Normalize(std::string code)
{
    // Convert to all-lowercase so people who mess up cases don't have things
    // break on them in subtle ways.
    std::transform(code.begin(), code.end(), code.begin(), [](unsigned char c) { return std::tolower(c); });

    // TODO strip off "(Additional Context)" bits from the end of the code.
    return code;
}

/**
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

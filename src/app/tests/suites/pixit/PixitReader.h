/**
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

/**
 * @file
 * @brief Declaration of PICS Boolean Reader, a class that read and parse
 *        a file with PICS Code and their enabled/disabled state.
 */

#include <string>
#include <unordered_map>

#include <app/tests/suites/pixit/PixitTypeTransformer.h>
#include <lib/core/CHIPError.h>

class PixitReader
{
public:
    using PixitMap = std::unordered_map<std::string, PixitTypeTransformer::PixitValueHolder>;

    static CHIP_ERROR UpdateFromFile(std::string filepath, PixitMap & map);
    static PixitMap PopulateDefaults();
};

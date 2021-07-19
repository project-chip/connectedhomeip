/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
namespace chip {

/**
 * @brief Create ZCL string from char
 */
static uint8_t * MakeZclCharString(uint8_t * zclString, const char * cString)
{
    size_t len = strlen(cString);
    if (len > 254)
    {
        len = 254;
    }
    zclString[0] = static_cast<uint8_t>(len);
    memcpy(&zclString[1], cString, zclString[0]);
    return zclString;
}

} // namespace chip

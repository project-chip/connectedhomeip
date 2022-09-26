/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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

#include <string>

namespace chip {
namespace IniEscaping {

/**
 * @brief Escape a storage key to be INI-safe.
 *
 * All characters <= 0x20, >= 0x7F and `\` and `=` are
 * escaped as `\xYY` where `YY` is a 2-digit lowercase hex value.
 *
 * @param key - key to escape
 * @return the escaped key
 */
std::string EscapeKey(const std::string & key);

/**
 * @brief Unescape a storage key escaped by `EscapeKey`
 *
 * If any character not expected to be escaped is found, or
 * if any escape sequences are partial, or if uppercase hex is seen
 * in an escape sequence, the empty string is returned.
 *
 * @param key - key to unescape
 * @return the original key that was provided to EscapeKey or empty string on error.
 */
std::string UnescapeKey(const std::string & escapedKey);

/**
 * @brief Takes an octet string passed into a std::string and converts it to base64
 *
 * There may be `\0` characters in the data of std::string.
 *
 * @param value - Value to convert to base64.
 * @return the base64 encoding of the `value` input
 */
std::string StringToBase64(const std::string & value);

/**
 * @brief Takes a base64 buffer and converts it to an octet string buffer
 *        within a std::string.
 *
 * @param b64Value - Buffer of base64 to decode
 * @return an std::string with the bytes, or empty string on decoding errors
 */
std::string Base64ToString(const std::string & b64Value);

} // namespace IniEscaping
} // namespace chip

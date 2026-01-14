/*
 *    Copyright (c) 2025 Project CHIP Authors
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

// A compile-time type representation of a string usable in C++ 17
// (where non-type template parameters are quite restricted).
template <char... Chars>
struct CompileTimeStringType
{
    static constexpr const char * c_str() { return value; }

private:
    static constexpr char value[] = { Chars..., 0 };
};

template <typename T, T... chars>
constexpr auto operator""_CompileTimeStringType()
{
    return CompileTimeStringType<chars...>{};
}

#define _CHIP_CTST(literal) "" literal##_CompileTimeStringType

/**
 * A type expression for a CompileTimeStringType representing the provided string literal.
 *
 * Example: CHIP_CTST("hello")
 */
#define CHIP_CTST(literal) decltype(_CHIP_CTST(literal))

} // namespace chip

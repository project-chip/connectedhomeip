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

/**
 *    @file
 *      This file defines and implements a number of miscellaneous
 *      templates for simplify code with handling types.
 *
 */

#pragma once

#include <type_traits>

namespace chip {

/**
 * @brief Implemented std::to_underlying introduced in C++23.
 */
template <class T>
constexpr std::underlying_type_t<T> to_underlying(T e)
{
    static_assert(std::is_enum<T>::value, "to_underlying called to non-enum values.");
    return static_cast<std::underlying_type_t<T>>(e);
}

/**
 * @brief This template is not designed to be used directly. A common pattern to check the presence of a member of a class is:
 *
 * \cond
 * template <typename T>
 * class IsMagic
 * {
 * private:
 *     template <typename Tp>
 *     static auto TestHasMagic(int) -> TemplatedTrueType<decltype(&Tp::Magic)>;
 *
 *     template <typename Tp>
 *     static auto TestHasMagic(long) -> std::false_type;
 *
 * public:
 *     static constexpr bool value = decltype(TestHasMagic<std::decay_t<T>>(0))::value;
 * };
 * \endcond
 *
 *   The compiler will try to match TestHasMagicFunction(int) first, if MagicFunction is a member function of T, the match succeed
 * and HasMagicFunction is an alias of std::true_type. If MagicFunction is not a member function of T, the match of
 * TestHasMagicFunction(int) will result in compile error, due to SFINAE, compiler will try the next candicate, which is
 * TestHasMagicFunction(long), it will always success for all types, and HasMagicFunction becomes an alias of std::false_type.
 */
template <typename T>
using TemplatedTrueType = std::true_type;

} // namespace chip

/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <lib/core/Optional.h>

namespace chip {
namespace app {
namespace DataModel {

/*
 * Dedicated type for nullable things, to differentiate them from optional
 * things.
 */
template <typename T>
struct Nullable : protected Optional<T>
{
    //
    // The following 'using' statement is needed to make visible
    // all constructors of the base class within this derived class.
    //
    using Optional<T>::Optional;

    // Pull in APIs that make sense on Nullable with the same names as on
    // Optional.
    using Optional<T>::Value;

    constexpr void SetNull() { Optional<T>::ClearValue(); }
    constexpr bool IsNull() const { return !Optional<T>::HasValue(); }

    template <class... Args>
    constexpr T & SetNonNull(Args &&... args)
    {
        return Optional<T>::Emplace(std::forward<Args>(args)...);
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip

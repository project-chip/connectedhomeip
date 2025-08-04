/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <type_traits>
#include <utility>

namespace chip {

/**
 * A dumpable object that can log some useful state for debugging in fatal
 * error scenarios by exposing a `void DumpToLog() const` method. The method
 * should log key details about the state of object using ChipLogError().
 */
template <class, class = void>
struct IsDumpable : std::false_type
{
};
template <class T>
struct IsDumpable<T, std::void_t<decltype(std::declval<T>().DumpToLog())>> : std::true_type
{
};

struct DumpableTypeExample
{
    void DumpToLog() const {};
};
static_assert(IsDumpable<DumpableTypeExample>::value);

/**
 * Calls DumpToLog() on the object, if supported.
 */
template <class T>
void DumpObjectToLog([[maybe_unused]] const T * object)
{
    if constexpr (IsDumpable<T>::value)
    {
        object->DumpToLog();
    }
}

} // namespace chip

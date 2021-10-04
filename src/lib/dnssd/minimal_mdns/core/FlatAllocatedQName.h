/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "QName.h"

namespace mdns {
namespace Minimal {

/// A void* implementation that stores QNames as
///   [ptr array] [name1] #0 [name2] #0 .... [namen] #0
///
/// Usage:
///   - RequiredStorageSize("some", "name", "here") provides the memory block
///   - Build(ptr, "some", "name", "here") stores all data in [ptr]
///
namespace FlatAllocatedQName {

inline size_t RequiredStorageSize()
{
    return 0;
}

/// Determines the memory size required to store the given qname parts.
///
/// Example:
///   malloc(RequiredStorageSize("myhostname", "local"));
template <typename... Args>
inline size_t RequiredStorageSize(QNamePart name, Args &&... rest)
{
    static_assert(CHAR_BIT == 8, "Assumption is that names can be stored in 1 byte");

    // need to store a pointer entry in the array, the name and null terminator plus
    // the rest of the data
    return sizeof(QNamePart) + strlen(name) + 1 + RequiredStorageSize(std::forward<Args>(rest)...);
}

inline size_t RequiredStorageSizeFromArray(char const * const * names, size_t num)
{
    size_t ret = 0;
    for (size_t i = 0; i < num; ++i)
    {
        ret += sizeof(QNamePart) + strlen(names[i]) + 1;
    }
    return ret;
}
namespace Internal {

// nothing left to initialize
inline void Initialize(QNamePart * ptrLocation, char * nameLocation) {}

template <typename... Args>
inline void Initialize(QNamePart * ptrLocation, char * nameLocation, const char * name, Args &&... rest)
{
    *ptrLocation = nameLocation;
    strcpy(nameLocation, name);

    Initialize(ptrLocation + 1, nameLocation + strlen(nameLocation) + 1, std::forward<Args>(rest)...);
}

} // namespace Internal

/// Builds a qname inside the given storage.
///
/// storage MUST be aligned to hold pointers
///
/// Example:
///   void * data = malloc(RequiredStorageSize("myhostname", "local"));
///   FullQName value = Build(data, "myhostname", "local");
template <typename... Args>
inline FullQName Build(void * storage, Args &&... args)
{
    QNamePart * names = reinterpret_cast<QNamePart *>(storage);
    char * nameOut    = reinterpret_cast<char *>(names + sizeof...(args));

    Internal::Initialize(names, nameOut, std::forward<Args>(args)...);

    FullQName result;
    result.names     = names;
    result.nameCount = sizeof...(args);
    return result;
}

inline FullQName BuildFromArray(void * storage, char const * const * parts, size_t num)
{
    // Storage memory holds pointers to each name, then copies of the names after
    QNamePart * names = reinterpret_cast<QNamePart *>(storage);
    char * nameOut    = reinterpret_cast<char *>(names + num);
    for (size_t i = 0; i < num; ++i)
    {
        QNamePart * ptrLocation = names + i;
        Internal::Initialize(ptrLocation, nameOut, parts[i]);
        nameOut += strlen(parts[i]) + 1;
    }
    FullQName result;
    result.names     = names;
    result.nameCount = num;
    return result;
}

} // namespace FlatAllocatedQName

} // namespace Minimal
} // namespace mdns

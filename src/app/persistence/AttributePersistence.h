/*
 *    Copyright (c) 2021-2025 Project CHIP Authors
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

#include <app/persistence/AttributePersistenceProvider.h>
#include <app/persistence/PascalString.h>

#include <optional>

namespace chip::app {

/// Provides functionality for handling attribute persistence via
/// an AttributePersistenceProvider.
///
/// AttributePersistenceProvider works with raw bytes, however attributes
/// have known (strong) types and their load/decode logic is often
/// similar and reusable. This class implements the logic of handling
/// such attributes, so that it can be reused across cluster implementations.
///
class AttributePersistence
{
public:
    AttributePersistence(AttributePersistenceProvider & provider) : mProvider(provider) {}

    /// Loads a native-entianess stored value into `T` from the persistence provider.
    ///
    /// If load fails, `false` is returned and data is filled with `valueOnLoadFailure`.
    ///
    /// Error reason for load failure is logged (or nothing logged in case "Value not found" is the
    /// reason for the load failure).
    template <typename T>
    bool LoadNativeEdianValue(const ConcreteAttributePath & path, T & value, const T & valueOnLoadFailure)
    {
        return InternalRawLoadNativeEndianValue(path, &value, &valueOnLoadFailure, sizeof(T));
    }

    /// Loads a short pascal string from persistent storage
    ///
    /// If load fails, `false` is returned and data is filled with `valueOnLoadFailure`.
    ///
    /// Error reason for load failure is logged (or nothing logged in case "Value not found" is the
    /// reason for the load failure).
    ///
    /// if valueOnLoadFailure cannot be set into value, value will be set to NULL (which never fails)
    bool Load(const ConcreteAttributePath & path, Storage::ShortPascalString & value, std::optional<CharSpan> valueOnLoadFailure);

private:
    AttributePersistenceProvider & mProvider;

    /// Loads a raw value of size `size` into the memory pointed to by `data`.
    /// If load fails, `false` is returned and data is filled with `valueOnLoadFailure`.
    ///
    /// Error reason for load failure is logged (or nothing logged in case "Value not found" is the
    /// reason for the load failure).
    bool InternalRawLoadNativeEndianValue(const ConcreteAttributePath & path, void * data, const void * valueOnLoadFailure,
                                          size_t size);
};

} // namespace chip::app

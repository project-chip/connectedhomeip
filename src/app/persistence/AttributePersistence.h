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

#include <app/AttributeValueDecoder.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/persistence/AttributePersistenceProvider.h>
#include <app/persistence/PascalString.h>

#include <optional>
#include <type_traits>

namespace chip::app {

/// Provides functionality for handling attribute persistence via
/// an AttributePersistenceProvider.
///
/// AttributePersistenceProvider works with raw bytes, however attributes
/// have known (strong) types and their load/decode logic is often
/// similar and reusable. This class implements the logic of handling
/// such attributes, so that it can be reused across cluster implementations.
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
    template <typename T, typename std::enable_if<std::is_fundamental_v<T>>::type * = nullptr>
    bool LoadNativeEndianValue(const ConcreteAttributePath & path, T & value, const T & valueOnLoadFailure)
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

    /// Interprets the `buffer` value as a `StringType` (generally ShortPascalString or similar) for the purposes of loading
    template <typename StringType, size_t N>
    bool LoadPascalString(const ConcreteAttributePath & path, char (&buffer)[N], std::optional<CharSpan> valueOnLoadFailure)
    {

        StringType value(buffer);
        return Load(path, value, valueOnLoadFailure);
    }

    /// Performs all the steps of:
    ///   - decode the given char span
    ///   - store it in the given pascal string
    ///   - write the value to persistent storage as a "prefixed value"
    DataModel::ActionReturnStatus Store(const ConcreteAttributePath & path, AttributeValueDecoder & decoder,
                                        Storage::ShortPascalString & value);

    /// helper to not create a separate ShortPascalString out of a buffer.
    template <typename StringType, size_t N>
    DataModel::ActionReturnStatus StorePascalString(const ConcreteAttributePath & path, AttributeValueDecoder & decoder,
                                                    char (&buffer)[N])
    {
        StringType value(buffer);
        return Store(path, decoder, value);
    }

    /// Performs all the steps of:
    ///   - decode the given raw data
    ///   - write to storage
    template <typename T, typename std::enable_if<std::is_fundamental_v<T>>::type * = nullptr>
    DataModel::ActionReturnStatus StoreNativeEndianValue(const ConcreteAttributePath & path, AttributeValueDecoder & decoder,
                                                         T & value)
    {
        ReturnErrorOnFailure(decoder.Decode(value));
        return mProvider.WriteValue(path, { reinterpret_cast<const uint8_t *>(&value), sizeof(value) });
    }

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

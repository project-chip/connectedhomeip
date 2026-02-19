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

#include <app/AttributeValueDecoder.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/persistence/AttributePersistenceProvider.h>
#include <app/persistence/String.h>

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

    /// Loads a native-endianness stored value of type `T` into `value` from the persistence provider.
    ///
    /// If load fails, `false` is returned and data is filled with `valueOnLoadFailure`.
    ///
    /// Error reason for load failure is logged (or nothing logged in case "Value not found" is the
    /// reason for the load failure).
    template <typename T, typename std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>> * = nullptr>
    bool LoadNativeEndianValue(const ConcreteAttributePath & path, T & value, const T & valueOnLoadFailure)
    {
        return InternalRawLoadNativeEndianValue(path, &value, &valueOnLoadFailure, sizeof(T));
    }

    /// Nullable
    /// Loads a native-endianness stored value of type `T` into `value` from the persistence provider.
    ///
    /// If load fails, `false` is returned and data is filled with `valueOnLoadFailure`.
    ///
    /// Error reason for load failure is logged (or nothing logged in case "Value not found" is the
    /// reason for the load failure).
    template <typename T, typename std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>> * = nullptr>
    bool LoadNativeEndianValue(const ConcreteAttributePath & path, DataModel::Nullable<T> & value,
                               const DataModel::Nullable<T> & valueOnLoadFailure)
    {
        typename NumericAttributeTraits<T>::StorageType storageReadValue;
        typename NumericAttributeTraits<T>::StorageType storageDefaultValue;

        NullableToStorage(valueOnLoadFailure, storageDefaultValue);
        bool success = InternalRawLoadNativeEndianValue(path, &storageReadValue, &storageDefaultValue, sizeof(T));
        StorageToNullable(storageReadValue, value);

        return success;
    }

    /// Performs all the steps of:
    ///   - decode the given raw data
    ///   - validate that the decoded value is different from the current one
    ///   - write to storage
    template <typename T, typename std::enable_if_t<std::is_arithmetic_v<T>> * = nullptr>
    DataModel::ActionReturnStatus DecodeAndStoreNativeEndianValue(const ConcreteAttributePath & path,
                                                                  AttributeValueDecoder & decoder, T & value)
    {
        T decodedValue{};
        ReturnErrorOnFailure(decoder.Decode(decodedValue));
        VerifyOrReturnValue(decodedValue != value, DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
        value = decodedValue;
        return mProvider.WriteValue(path, { reinterpret_cast<const uint8_t *>(&value), sizeof(value) });
    }

    /// Nullable type handling
    /// Performs all the steps of:
    ///   - decode the given raw data
    ///   - validate that the decoded value is different from the current one
    ///   - write to storage
    template <typename T, typename std::enable_if_t<std::is_arithmetic_v<T>> * = nullptr>
    DataModel::ActionReturnStatus DecodeAndStoreNativeEndianValue(const ConcreteAttributePath & path,
                                                                  AttributeValueDecoder & decoder, DataModel::Nullable<T> & value)
    {
        DataModel::Nullable<T> decodedValue{};
        ReturnErrorOnFailure(decoder.Decode(decodedValue));
        VerifyOrReturnValue(decodedValue != value, DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
        value = decodedValue;

        typename NumericAttributeTraits<T>::StorageType storageValue;
        NullableToStorage(value, storageValue);

        return mProvider.WriteValue(path, { reinterpret_cast<const uint8_t *>(&storageValue), sizeof(storageValue) });
    }

    // Specialization for enums
    // - decode the given data
    // - verifies that it is a valid enum value
    // - validate that the decoded value is different from the current one
    // - writes to storage
    template <typename T, typename std::enable_if_t<std::is_enum_v<T>> * = nullptr>
    DataModel::ActionReturnStatus DecodeAndStoreNativeEndianValue(const ConcreteAttributePath & path,
                                                                  AttributeValueDecoder & decoder, T & value)
    {
        T decodedValue = T::kUnknownEnumValue;
        ReturnErrorOnFailure(decoder.Decode(decodedValue));
        VerifyOrReturnError(decodedValue != T::kUnknownEnumValue, CHIP_IM_GLOBAL_STATUS(ConstraintError));
        VerifyOrReturnValue(decodedValue != value, DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
        value = decodedValue;
        return mProvider.WriteValue(path, { reinterpret_cast<const uint8_t *>(&value), sizeof(value) });
    }

    // Nullable
    // Specialization for enums
    // - decode the given data
    // - verifies that it is a valid enum value
    // - validate that the decoded value is different from the current one
    // - writes to storage
    template <typename T, typename std::enable_if_t<std::is_enum_v<T>> * = nullptr>
    DataModel::ActionReturnStatus DecodeAndStoreNativeEndianValue(const ConcreteAttributePath & path,
                                                                  AttributeValueDecoder & decoder, DataModel::Nullable<T> & value)
    {
        DataModel::Nullable<T> decodedValue{};
        ReturnErrorOnFailure(decoder.Decode(decodedValue));
        VerifyOrReturnError(decodedValue.IsNull() || decodedValue.Value() != T::kUnknownEnumValue,
                            CHIP_IM_GLOBAL_STATUS(ConstraintError));
        VerifyOrReturnValue(decodedValue != value, DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
        value = decodedValue;

        typename NumericAttributeTraits<T>::StorageType storageValue;
        NullableToStorage(value, storageValue);

        return mProvider.WriteValue(path, { reinterpret_cast<const uint8_t *>(&storageValue), sizeof(storageValue) });
    }

    /// Load the given string from concrete storage.
    ///
    /// NOTE: `value` is take as an internal short string to avoid the templates that Storage::String
    /// implies, however callers are generally expected to pass in a `Storage::String` value and
    /// not use internal classes directly.
    ///
    /// Returns true on success, false on failure. On failure the string is reset to empty.
    bool LoadString(const ConcreteAttributePath & path, Storage::Internal::ShortString & value);

    /// Store the given string in persistent storage.
    ///
    /// NOTE: `value` is take as an internal short string to avoid the templates that Storage::String
    /// implies, however callers are generally expected to pass in a `Storage::String` value and
    /// not use internal classes directly.
    CHIP_ERROR StoreString(const ConcreteAttributePath & path, const Storage::Internal::ShortString & value);

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

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
#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <app/persistence/AttributePersistenceProvider.h>
#include <app/persistence/String.h>
#include <lib/core/TLV.h>

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

    /// Writes a TLV-encodable value (using DataModel::Encode) to the attribute storage.
    /// Uses the provided buffer for TLV encoding.
    ///
    /// The encoding format is:
    ///   Structure (Anonymous Tag)
    ///     <Value> (Context Tag 1)
    ///   EndContainer
    ///
    /// This wrapper ensures valid top-level TLV elements and allows future extensibility.
    template <typename T>
    CHIP_ERROR StoreTLV(const ConcreteAttributePath & path, const T & value, MutableByteSpan buffer)
    {
        return InternalStoreTLV(path, buffer, &value, [](const void * context, TLV::TLVWriter & writer) -> CHIP_ERROR {
            return DataModel::Encode(writer, kTLVEncodingTag, *static_cast<const T *>(context));
        });
    }

    /// Stack-allocating overload for convenience.
    template <size_t kMaxBufferSize, typename T>
    CHIP_ERROR StoreTLV(const ConcreteAttributePath & path, const T & value)
    {
        uint8_t buffer[kMaxBufferSize];
        return StoreTLV(path, value, MutableByteSpan(buffer));
    }

    /// Loads a TLV value from storage using the provided buffer.
    ///
    /// WARNING: If T contains views (e.g. Spans, DataModel::List), they will point into `buffer`.
    /// The `buffer` MUST outlive the usage of `value`.
    template <typename T>
    CHIP_ERROR LoadTLV(const ConcreteAttributePath & path, T & value, MutableByteSpan buffer)
    {
        return InternalLoadTLV(path, buffer, &value, [](void * context, TLV::TLVReader & reader) -> CHIP_ERROR {
            return DataModel::Decode(reader, *static_cast<T *>(context));
        });
    }

    /// Stack-allocating overload for convenience.
    ///
    /// @warning CRITICAL WARNING: Do NOT use this overload if T contains any data views
    /// (e.g., CharSpan, ByteSpan, DataModel::List). The backing buffer for these views is allocated
    /// on the stack and will be DESTROYED when this function returns, leading to DANGLING POINTERS
    /// and undefined behavior.
    ///
    /// If T contains any view types, you MUST use the overload that accepts an external MutableByteSpan
    /// to ensure the buffer outlives the usage of the decoded value.
    template <size_t kMaxBufferSize, typename T>
    CHIP_ERROR LoadTLV(const ConcreteAttributePath & path, T & value)
    {
        // NOTE: This overload assumes T does NOT contain views pointing to the buffer,
        // because the buffer is destroyed when this function returns.
        // Use the buffer-passing overload if T contains Views (like Span or List).
        uint8_t buffer[kMaxBufferSize];
        return LoadTLV(path, value, MutableByteSpan(buffer));
    }

private:
    static constexpr TLV::Tag kTLVEncodingTag = TLV::ContextTag(1);
    AttributePersistenceProvider & mProvider;

    /// Loads a raw value of size `size` into the memory pointed to by `data`.
    /// If load fails, `false` is returned and data is filled with `valueOnLoadFailure`.
    ///
    /// Error reason for load failure is logged (or nothing logged in case "Value not found" is the
    /// reason for the load failure).
    bool InternalRawLoadNativeEndianValue(const ConcreteAttributePath & path, void * data, const void * valueOnLoadFailure,
                                          size_t size);

    using TLVEncoderCallback = CHIP_ERROR (*)(const void * context, TLV::TLVWriter & writer);
    using TLVDecoderCallback = CHIP_ERROR (*)(void * context, TLV::TLVReader & reader);

    CHIP_ERROR InternalStoreTLV(const ConcreteAttributePath & path, MutableByteSpan buffer, const void * context,
                                TLVEncoderCallback encoder);
    CHIP_ERROR InternalLoadTLV(const ConcreteAttributePath & path, MutableByteSpan buffer, void * context,
                               TLVDecoderCallback decoder);
};

} // namespace chip::app

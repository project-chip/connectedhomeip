/**
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

#include <app/ConcreteAttributePath.h>
#include <app/storage/PascalString.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace Storage {

/// Defines an interface for persisting attribute values.
///
/// Provides read/write of values (char/byte arrays or numeric values) based
/// on a endpoint/cluster/attribute path.
///
/// To take into account storage compatibility, the class uses the concepts of:
///
///  - `Value` represents something that is writable to persistent storage
///  - `Buffer` represents something that can be read from persistent storage
///
class AttributeStorage
{
private:
    static ByteSpan asByteSpan(CharSpan span) { return { reinterpret_cast<const uint8_t *>(span.data()), span.size() }; }
    static ByteSpan asByteSpan(ByteSpan span) { return span; }

public:
    /// Represents an attribute value that can be written to storage.
    ///
    /// In particular it has information about the data content since
    /// historical ember-specific writes for strings encode data size as
    /// pascal strings.
    class Value
    {
    public:
        /// Represents the value type of an element
        ///
        /// The reason for the definitions here are to allow backwards compatibility with
        /// ember attribute storage and storage implementations can be free to ignore the
        /// data types if they want to store things "as-is"
        ///
        /// Backward compatibility considerations:
        ///   - ember storage uses native endianess, some other storages use little-endian always
        ///   - ember uses pascal strings for storing data and the length of the size prefix
        ///     is dictated by the maximum string size
        enum class Type
        {
            kNumeric,             // the data represents an integer value. The span will contain the value
            kRaw,                 // Raw bytes, not interpreted
            kStringOneByteLength, // string where length is never larger than 255 characters. Data is a pascal string.
            kStringTwoByteLength, // string where length is never larger than 0xFFFF characters. Data is a pascal string.
        };

        Value(const Value & other) = default;

        template <typename T>
        Value(ShortPascalString<T> & data) : Value({ asByteSpan(data.PascalContent()), Type::kStringOneByteLength })
        {}

        template <typename T>
        Value(LongPascalString<T> & data) : Value({ asByteSpan(data.PascalContent()), Type::kStringTwoByteLength })
        {}

        template <typename T>
        static Value Number(T & value)
        {
            return { ByteSpan(reinterpret_cast<uint8_t *>(&value), sizeof(value)), Type::kNumeric };
        }

        template <typename T>
        static Value Raw(ByteSpan bytes)
        {
            return { bytes, Type::kRaw };
        }

        ByteSpan data() const { return mData; }
        Type type() const { return mType; }

    private:
        Value(ByteSpan data, Type type) : mData(data), mType(type) {}

        const ByteSpan mData; // where the data resides
        Type mType;
    };

    /// Represents an attribute value that can be read from storage.
    class Buffer
    {
    public:
        // Separates out what `mData`
        enum class Type
        {
            kNumeric,             // the data represents an integer value
            kRaw,                 // the data represents raw, not interpreted data
            kStringOneByteLength, // string where length is never larger than 255 characters
            kStringTwoByteLength, // string where length is never larger than 0xFFFF characters
            kBytesOneByteLength,  // byte string where length is never larger than 255 characters
            kBytesTwoByteLength,  // byte string where length is never larger than 0xFFFF characters
        };

        Buffer(const Buffer & other) = default;

        Buffer(ShortPascalString<char> & data) : Buffer(data.Buffer().data(), data.Buffer().size(), Type::kStringOneByteLength) {}
        Buffer(LongPascalString<char> & data) : Buffer(data.Buffer().data(), data.Buffer().size(), Type::kStringTwoByteLength) {}

        Buffer(ShortPascalString<uint8_t> & data) : Buffer(data.Buffer().data(), data.Buffer().size(), Type::kStringOneByteLength)
        {}
        Buffer(LongPascalString<uint8_t> & data) : Buffer(data.Buffer().data(), data.Buffer().size(), Type::kStringTwoByteLength) {}

        template <typename T>
        static Buffer Number(T & value)
        {
            return { &value, sizeof(value), Type::kNumeric };
        }

        /// initializes the data as "raw". Note that `data()` in this case
        /// will return a pointer to the underlying `MutableByteSpan`
        static Buffer Raw(MutableByteSpan & data) { return { &data, 0, Type::kRaw }; }

        void * data() { return mData; }
        size_t size() const { return mBufferSize; }
        Type type() const { return mType; }

    private:
        Buffer(void * data, size_t size, Type type) : mData(data), mBufferSize(size), mType(type) {}

        void * mData;
        size_t mBufferSize; // size of the data in mData
        Type mType;
    };

    virtual ~AttributeStorage() = default;

    /// Writes the given value to persistent storage
    ///
    /// It is expected that `Read` will read back the same data as read.
    virtual CHIP_ERROR Write(const ConcreteAttributePath & path, const Value & value) = 0;

    /// Read the given value from persistent storage.
    ///
    /// Notable possible errors:
    ///    CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND - path does not exist in storage
    ///    CHIP_ERROR_BUFFER_TOO_SMALL - insufficient storage to fetch the data
    virtual CHIP_ERROR Read(const ConcreteAttributePath & path, Buffer buffer) = 0;
};

} // namespace Storage
} // namespace app
} // namespace chip

/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "lib/core/TLVWriter.h"
#include <app/util/attribute-metadata.h>
#include <app/util/ember-io-storage.h>
#include <lib/core/TLVReader.h>
#include <lib/core/TLVTypes.h>
#include <lib/support/BufferReader.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace Ember {

/// This class represents a pointer to an ember-encoded attribute in a specific memory location.
///
/// Ember attributes are stored as raw bytes for numeric types (i.e. memcpy-like storage except
/// unaligned) and strings are Pascal-like (short with 1-byte length prefix or long with 2-byte length
/// prefix).
///
/// Class is to be used as a one-shot:
///   - create it out of metadata + data span
///   - call Decode (which modifies the input data span)
class EmberAttributeDataBuffer
{
public:
#if CHIP_CONFIG_BIG_ENDIAN_TARGET
    using EndianWriter = Encoding::BigEndian::BufferWriter;
    using EndianReader = Encoding::BigEndian::Reader;
#else
    using EndianWriter = Encoding::LittleEndian::BufferWriter;
    using EndianReader = Encoding::LittleEndian::Reader;
#endif

    enum class PascalStringType
    {
        kShort,
        kLong,
    };

    static constexpr bool kIsFabricScoped = false;

    EmberAttributeDataBuffer(const EmberAfAttributeMetadata * meta, MutableByteSpan & data) :
        mIsNullable(meta->IsNullable()), mAttributeType(chip::app::Compatibility::Internal::AttributeBaseType(meta->attributeType)),
        mDataBuffer(data)
    {}

    /// Reads the data pointed into by `reader` and updates the data
    /// internally into mDataBuffer (which is then reflected outwards)
    ///
    /// Generally should be called ONLY ONCE as the internal mutable byte span gets
    /// modified by this call.
    CHIP_ERROR Decode(chip::TLV::TLVReader & reader);

    /// Writes the data encoded in the underlying buffer into the given `writer`
    ///
    /// The data in the internal data buffer is assumed to be already formatted correctly
    /// HOWEVER the size inside it will not be fully considered (i.e. encoding will use
    /// the data encoding line integer or string sizes and NOT the databuffer max size)
    CHIP_ERROR Encode(chip::TLV::TLVWriter & writer, TLV::Tag tag) const;

private:
    /// Decodes the UNSIGNED integer stored in `reader` and places its content into `writer`
    /// Takes into account internal mIsNullable.
    CHIP_ERROR DecodeUnsignedInteger(chip::TLV::TLVReader & reader, EndianWriter & writer);

    /// Decodes the SIGNED integer stored in `reader` and places its content into `writer`
    /// Takes into account internal mIsNullable.
    CHIP_ERROR DecodeSignedInteger(chip::TLV::TLVReader & reader, EndianWriter & writer);

    /// Encodes the UNSIGNED integer into `writer`.
    /// Takes into account internal mIsNullable.
    CHIP_ERROR EncodeInteger(chip::TLV::TLVWriter & writer, TLV::Tag tag, EndianReader & reader) const;

    /// Decodes the string/byte string contained in `reader` and stores it into `writer`.
    /// String is encoded using a pascal-prefix of size `stringType`.
    /// Takes into account internal mIsNullable.
    ///
    /// The string in `reader` is expected to be of type `tlvType`
    CHIP_ERROR DecodeAsString(chip::TLV::TLVReader & reader, PascalStringType stringType, TLV::TLVType tlvType,
                              EndianWriter & writer);

    const bool mIsNullable;                    // Contains if the attribute metadata marks the field as NULLABLE
    const EmberAfAttributeType mAttributeType; // Initialized with the attribute type from the metadata
    MutableByteSpan & mDataBuffer;             // output buffer, modified by `Decode`
};

} // namespace Ember

namespace DataModel {

/// Helper method to forward the decode of this type to the class specific implementation
inline CHIP_ERROR Decode(TLV::TLVReader & reader, Ember::EmberAttributeDataBuffer & buffer)
{
    return buffer.Decode(reader);
}

inline CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag, Ember::EmberAttributeDataBuffer & buffer)
{
    return buffer.Encode(writer, tag);
}

} // namespace DataModel
} // namespace app
} // namespace chip

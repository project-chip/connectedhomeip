/**
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

#include <app/data-model/Nullable.h>
#include <app/util/basic-types.h>
#include <app/util/endpoint-config-defines.h>
#include <cstdint>
#include <lib/support/Span.h>
#include <lib/support/attribute-storage-null-handling.h>
#include <protocols/interaction_model/StatusCode.h>

/**
 * @brief Type for referring to ZCL attribute type
 */
typedef uint8_t EmberAfAttributeType;

/**
 * @brief Type for the attribute mask
 */
typedef uint8_t EmberAfAttributeMask;

/**
 * @brief Type for default values.
 *
 * Default value is either a value itself, if it is 2 bytes or less,
 * or a pointer to the value itself, if attribute type is longer than
 * 2 bytes.
 */
union EmberAfDefaultAttributeValue
{
    constexpr EmberAfDefaultAttributeValue(const uint8_t * ptr) : ptrToDefaultValue(ptr) {}
    constexpr EmberAfDefaultAttributeValue(uint16_t val) : defaultValue(val) {}

    /**
     * Points to data if size is more than 2 bytes.
     * If size is more than 2 bytes, and this value is NULL,
     * then the default value is all zeroes.
     */
    const uint8_t * ptrToDefaultValue;

    /**
     * Actual default value if the attribute size is 2 bytes or less.
     */
    uint16_t defaultValue;
};

/**
 * @brief Type describing the attribute default, min and max values.
 *
 * This struct is required if the attribute mask specifies that this
 * attribute has a known min and max values.
 */
typedef struct
{
    /**
     * Default value of the attribute.
     */
    EmberAfDefaultAttributeValue defaultValue;
    /**
     * Minimum allowed value
     */
    EmberAfDefaultAttributeValue minValue;
    /**
     * Maximum allowed value.
     */
    EmberAfDefaultAttributeValue maxValue;
} EmberAfAttributeMinMaxValue;

/**
 * @brief Union describing the attribute default/min/max values.
 */
union EmberAfDefaultOrMinMaxAttributeValue
{
    constexpr EmberAfDefaultOrMinMaxAttributeValue(const uint8_t * ptr) : ptrToDefaultValue(ptr) {}
    constexpr EmberAfDefaultOrMinMaxAttributeValue(uint32_t val) : defaultValue(val) {}
    constexpr EmberAfDefaultOrMinMaxAttributeValue(const EmberAfAttributeMinMaxValue * ptr) : ptrToMinMaxValue(ptr) {}

    /**
     * Points to data if the attribute type is a string or the size of the data is more than 4 bytes.
     * If the attribute type is a string or the data size is more than 4 bytes, and this value is NULL,
     * then the default value is all zeroes.
     */
    const uint8_t * ptrToDefaultValue;
    /**
     * Actual default value if the attribute is non string and size
     * is 4 bytes or less.
     */
    uint32_t defaultValue;
    /**
     * Points to the min max attribute value structure, if min/max is
     * supported for this attribute.
     */
    const EmberAfAttributeMinMaxValue * ptrToMinMaxValue;
};

// Attribute masks modify how attributes are used by the framework
// The following define names are relevant to the ZAP_ATTRIBUTE_MASK macro.
// Attribute that has this mask is writable
#define MATTER_ATTRIBUTE_FLAG_WRITABLE (0x01)
// Attribute that has this mask is saved in non-volatile memory
#define MATTER_ATTRIBUTE_FLAG_NONVOLATILE (0x02)
// Alias until ZAP gets updated to output MATTER_ATTRIBUTE_FLAG_NONVOLATILE
#define MATTER_ATTRIBUTE_FLAG_TOKENIZE MATTER_ATTRIBUTE_FLAG_NONVOLATILE
// Attribute that has this mask has a min/max values
#define MATTER_ATTRIBUTE_FLAG_MIN_MAX (0x04)
// Attribute requires a timed interaction to write
#define MATTER_ATTRIBUTE_FLAG_MUST_USE_TIMED_WRITE (0x08)
// Attribute deferred to external storage
#define MATTER_ATTRIBUTE_FLAG_EXTERNAL_STORAGE (0x10)
// Attribute that has this mask is readable
#define MATTER_ATTRIBUTE_FLAG_READABLE (0x20)
// Attribute is nullable
#define MATTER_ATTRIBUTE_FLAG_NULLABLE (0x40)
// Attribute has no default value configured
#define MATTER_ATTRIBUTE_FLAG_NO_DEFAULT_VALUE (0x80)

/**
 * @brief Each attribute has it's metadata stored in such struct.
 *
 * There is only one of these per attribute across all endpoints.
 */
struct EmberAfAttributeMetadata
{
    /**
     * Pointer to the default value union. Actual value stored
     * depends on the mask.
     */
    EmberAfDefaultOrMinMaxAttributeValue defaultValue;

    /**
     * Attribute ID, according to ZCL specs.
     */
    chip::AttributeId attributeId;

    /**
     * Size of this attribute in bytes.
     */
    uint16_t size;

    /**
     * Attribute type, according to ZCL specs.
     */
    EmberAfAttributeType attributeType;

    /**
     * Attribute mask, tagging attribute with specific
     * functionality.
     */
    EmberAfAttributeMask mask;

    /**
     * Check whether this attribute was declared with no default value.
     */
    bool HasEmptyDefault() const { return (mask & MATTER_ATTRIBUTE_FLAG_NO_DEFAULT_VALUE) != 0; }

    /**
     * Check wether this attribute is a boolean based on its type according to the spec.
     */
    bool IsBoolean() const;

    /**
     * Check wether this attribute is signed based on its type according to the spec.
     */
    bool IsSignedIntegerAttribute() const;

    /**
     * Check whether this attribute has a define min and max.
     */
    bool HasMinMax() const { return mask & MATTER_ATTRIBUTE_FLAG_MIN_MAX; }

    /**
     * Check whether this attribute is nullable.
     */
    bool IsNullable() const { return mask & MATTER_ATTRIBUTE_FLAG_NULLABLE; }

    /**
     * Check whether this attribute is writable.
     */
    bool IsWritable() const { return mask & MATTER_ATTRIBUTE_FLAG_WRITABLE; }

    /**
     * Check whether this attribute is readable.
     */
    bool IsReadable() const { return mask & MATTER_ATTRIBUTE_FLAG_READABLE; }

    /**
     * Check whether this attribute is readonly.
     * Note: IsReadOnly is deprecated: Keeping it for backward compatibility
     */
    bool IsReadOnly() const { return IsReadable() && !IsWritable(); }

    /**
     * Check whether this attribute requires a timed write.
     */
    bool MustUseTimedWrite() const { return mask & MATTER_ATTRIBUTE_FLAG_MUST_USE_TIMED_WRITE; }

    /**
     * Check whether this attibute's storage is managed outside the built-in
     * attribute store.
     */
    bool IsExternal() const { return mask & MATTER_ATTRIBUTE_FLAG_EXTERNAL_STORAGE; }

    /**
     * Check whether this attribute is automatically stored in non-volatile
     * memory.
     */
    bool IsAutomaticallyPersisted() const { return (mask & MATTER_ATTRIBUTE_FLAG_NONVOLATILE) && !IsExternal(); }
};

/** @brief Returns true if the given attribute type is a string. */
bool emberAfIsStringAttributeType(EmberAfAttributeType attributeType);

/** @brief Returns true if the given attribute type is a long string. */
bool emberAfIsLongStringAttributeType(EmberAfAttributeType attributeType);

namespace chip {
namespace app {

/**
 * @brief Represents an attribute default value, normally referenced directly from flash metadata.
 *
 * Lifetime: rawData usually points into flash and outlives this object. The exception is a value
 * supplied at runtime by a dynamic endpoint (see the endpoint-level emberAfGetAttributeDefaultValue
 * in attribute-storage.h), which is held inside this object. Copying is therefore disallowed.
 *
 * String Storage in Flash:
 * - Non-empty strings are stored in flash with a Pascal length prefix (1 byte for short
 *   strings, 2 bytes in little-endian for long strings).
 * - Empty string defaults (from ZAP_EMPTY_DEFAULT() / nullptr flash pointer) have an empty
 *   rawData ByteSpan (size == 0).
 * - Explicit Null string defaults for nullable strings are stored in flash with the length
 *   sentinel: { 0xFF } for short strings, or { 0xFF, 0xFF } for long strings.
 *
 * Scalar Storage in Flash:
 * - Flash storage for scalars (GENERATED_DEFAULTS and inline union defaultValue) is pre-compiled
 *   in target native-endian format.
 * - CopyScalar copies raw native-endian storage bytes into the destination buffer (or zeroes
 *   the buffer if rawData is empty), which directly maps to NumericAttributeTraits<T>::StorageType.
 */
struct AttributeDefaultValue
{
    /// Largest value SetOwnedValue accepts: the widest ember scalar.
    static constexpr size_t kMaxOwnedValueSize = sizeof(uint64_t);

    AttributeDefaultValue() = default;

    // rawData may alias the owned value, so copying or moving would leave the destination span
    // pointing into the source. Fixing up the span is possible but nothing needs it, and a
    // relocatable value would obscure whether the bytes live in flash or in the object.
    AttributeDefaultValue(const AttributeDefaultValue &)             = delete;
    AttributeDefaultValue & operator=(const AttributeDefaultValue &) = delete;
    AttributeDefaultValue(AttributeDefaultValue &&)                  = delete;
    AttributeDefaultValue & operator=(AttributeDefaultValue &&)      = delete;

    ByteSpan rawData;              // Flash pointer and size in bytes (empty span if zero-filled / omitted in flash)
    EmberAfAttributeType type = 0; // ZCL attribute type (used to distinguish short vs long string prefixes)

    /// Takes a copy of a value that does not live in flash, and points rawData at it.
    ///
    /// Dynamic endpoints have no ZAP configuration, so their values are supplied at runtime by
    /// emberAfExternalAttributeReadCallback. Returns false, leaving the object unchanged, if the
    /// value does not fit: only scalars are served this way, since copying a string default would
    /// defeat the zero-copy views above.
    bool SetOwnedValue(ByteSpan data, EmberAfAttributeType attributeType);

    /// Direct zero-copy CharSpan view (returns empty CharSpan() if rawData is empty or length is 0)
    CharSpan ToCharSpan() const;

    /// Direct zero-copy ByteSpan view (returns empty ByteSpan() if rawData is empty or length is 0)
    ByteSpan ToByteSpan() const;

    /// Nullable zero-copy CharSpan view (returns Null if length prefix is 0xFF / 0xFFFF)
    DataModel::Nullable<CharSpan> ToNullableCharSpan() const;

    /// Nullable zero-copy ByteSpan view (returns Null if length prefix is 0xFF / 0xFFFF)
    DataModel::Nullable<ByteSpan> ToNullableByteSpan() const;

    /// Decodes a non-nullable scalar default (uint8_t..uint64_t, int8_t..int64_t, bool, float, enum, BitMask, OddSizedInteger).
    ///
    /// The configured value is returned verbatim: a non-nullable attribute whose configuration happens to hold the type's
    /// null sentinel (0xFF for uint8, INT16_MIN for int16, ...) yields that sentinel rather than an error. Validating a
    /// static configuration at runtime costs flash on every device, so the configuration is trusted to be sane.
    template <typename T>
    typename NumericAttributeTraits<T>::WorkingType As() const
    {
        using Traits = NumericAttributeTraits<T>;
        typename Traits::StorageType temp;
        CopyScalar(&temp, sizeof(temp));
        return Traits::StorageToWorking(temp);
    }

    /// Decodes a nullable scalar default into DataModel::Nullable<WorkingType> (returns Null if rawData is empty).
    template <typename T>
    DataModel::Nullable<typename NumericAttributeTraits<T>::WorkingType> AsNullable() const
    {
        if (rawData.empty())
        {
            return DataModel::Nullable<typename NumericAttributeTraits<T>::WorkingType>();
        }
        using Traits = NumericAttributeTraits<T>;
        typename Traits::StorageType temp;
        CopyScalar(&temp, sizeof(temp));
        DataModel::Nullable<typename Traits::WorkingType> value;
        if (Traits::IsNullValue(temp))
        {
            value.SetNull();
        }
        else
        {
            value.SetNonNull(Traits::StorageToWorking(temp));
        }
        return value;
    }

private:
    /// Copies raw native-endian scalar storage bytes into destination buffer (zero-fills if rawData is empty).
    ///
    /// The buffer is untyped because the caller supplies a NumericAttributeTraits<T>::StorageType;
    /// As<T>() / AsNullable<T>() are the typed entry points.
    void CopyScalar(void * outBuffer, size_t bufferSize) const;

    /// Decodes the Pascal length-prefixed string payload held in rawData.
    ///
    /// Returns false (and clears outPayload) when the value is the Null sentinel, the type is not
    /// a string type, or the prefix does not agree with the size of rawData.
    bool DecodeStringPayload(ByteSpan & outPayload) const;

    uint8_t mOwnedValue[kMaxOwnedValueSize];
};

/// Extract default value given attribute metadata
///
/// Returns Success when outDefault holds the configured default, or NotFound when the attribute
/// exists but no default was configured. The endpoint-level overload in attribute-storage.h shares
/// this contract and adds UnsupportedCluster / UnsupportedAttribute; all of its implementations
/// (ember, mock, dynamic_server) must agree on those meanings.
///
/// This overload reads flash only. It never consults the application, so for an attribute belonging
/// to a dynamic endpoint it reports what the declaration holds rather than the live value.
Protocols::InteractionModel::Status emberAfGetAttributeDefaultValue(const EmberAfAttributeMetadata & metadata,
                                                                    AttributeDefaultValue & outDefault);

} // namespace app
} // namespace chip

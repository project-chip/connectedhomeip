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

#include <app/util/basic-types.h>
#include <cstdint>

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
//
// Attribute that has this mask is NOT read-only
#define ATTRIBUTE_MASK_WRITABLE (0x01)
// Attribute that has this mask is saved in non-volatile memory
#define ATTRIBUTE_MASK_NONVOLATILE (0x02)
// Alias until ZAP gets updated to output ATTRIBUTE_MASK_NONVOLATILE
#define ATTRIBUTE_MASK_TOKENIZE ATTRIBUTE_MASK_NONVOLATILE
// Attribute that has this mask has a min/max values
#define ATTRIBUTE_MASK_MIN_MAX (0x04)
// Attribute requires a timed interaction to write
#define ATTRIBUTE_MASK_MUST_USE_TIMED_WRITE (0x08)
// Attribute deferred to external storage
#define ATTRIBUTE_MASK_EXTERNAL_STORAGE (0x10)
// Attribute is singleton
#define ATTRIBUTE_MASK_SINGLETON (0x20)
// Attribute is nullable
#define ATTRIBUTE_MASK_NULLABLE (0x40)

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
    bool HasMinMax() const { return mask & ATTRIBUTE_MASK_MIN_MAX; }

    /**
     * Check whether this attribute is nullable.
     */
    bool IsNullable() const { return mask & ATTRIBUTE_MASK_NULLABLE; }

    /**
     * Check whether this attribute is readonly.
     */
    bool IsReadOnly() const { return !(mask & ATTRIBUTE_MASK_WRITABLE); }

    /**
     * Check whether this attribute requires a timed write.
     */
    bool MustUseTimedWrite() const { return mask & ATTRIBUTE_MASK_MUST_USE_TIMED_WRITE; }

    /**
     * Check whether this attibute's storage is managed outside the built-in
     * attribute store.
     */
    bool IsExternal() const { return mask & ATTRIBUTE_MASK_EXTERNAL_STORAGE; }

    /**
     * Check whether this is a "singleton" attribute, in the sense that it has a
     * single value across multiple instances of the cluster.  This is not
     * mutually exclusive with the attribute being external.
     */
    bool IsSingleton() const { return mask & ATTRIBUTE_MASK_SINGLETON; }

    /**
     * Check whether this attribute is automatically stored in non-volatile
     * memory.
     */
    bool IsAutomaticallyPersisted() const { return (mask & ATTRIBUTE_MASK_NONVOLATILE) && !IsExternal(); }
};

/** @brief Returns true if the given attribute type is a string. */
bool emberAfIsStringAttributeType(EmberAfAttributeType attributeType);

/** @brief Returns true if the given attribute type is a long string. */
bool emberAfIsLongStringAttributeType(EmberAfAttributeType attributeType);

/*
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <app/util/attribute-metadata.h>
#include <lib/support/Span.h>
#include <app/data-model/Nullable.h>
#include <app-common/zap-generated/attribute-type.h>
#include <lib/support/BufferReader.h>
#include <lib/support/BufferWriter.h>
#include <cstring>

namespace chip {
namespace app {

/**
 * Interface for persisting attribute values.
 */

class AttributePersistenceProvider
{
public:
    virtual ~AttributePersistenceProvider() = default;
    AttributePersistenceProvider()          = default;

    /**
     * Write an attribute value from the attribute store (i.e. not a struct or
     * list) to non-volatile memory.
     *
     * @param [in] aPath the attribute path for the data being written.
     * @param [in] aValue the data to write.  Integers and floats are
     *             represented in native endianness.  Strings are represented
     *             as Pascal-style strings, as in ZCL, with a length prefix
     *             whose size depends on the actual string type.  The length is
     *             stored as little-endian.
     *
     *             Integer and float values have a size that matches the `size`
     *             member of aMetadata.
     *
     *             String values have a size that corresponds to the actual size
     *             of the data in the string (including the length prefix),
     *             which is no larger than the `size` member of aMetadata.
     */
    virtual CHIP_ERROR WriteValue(const ConcreteAttributePath & aPath, const ByteSpan & aValue) = 0;

    /**
     * Read an attribute value from non-volatile memory.
     *
     * @param [in]     aPath the attribute path for the data being persisted.
     * @param [in]     aType the attribute type.
     * @param [in]     aSize the attribute size.
     * @param [in,out] aValue where to place the data.  The size of the buffer
     *                 will be equal to `size`.
     *
     *                 The data is expected to be in native endianness for
     *                 integers and floats.  For strings, see the string
     *                 representation description in the WriteValue
     *                 documentation.
     */
    virtual CHIP_ERROR ReadValue(const ConcreteAttributePath & aPath, EmberAfAttributeType aType,
                                 uint16_t aSize, MutableByteSpan & aValue) = 0;

    // The following API provides helper functions to simplify the access of commonly used types.
    // The API may not be complete.
    // Currently implemented write and read types are: uint8_t, uint16_t, uint32_t, unit64_t and
    // their nullable varieties, and bool.

    /**
     * Write an attribute value of type unsigned intX or bool to non-volatile memory.
     *
     * @param [in] aPath the attribute path for the data being written.
     * @param [in] aValue the data to write.
     */
    template <typename T, std::enable_if_t<std::is_unsigned<T>::value, bool> = true >
    CHIP_ERROR WriteValue(const ConcreteAttributePath & aPath, T & aValue)
    {
        uint8_t value[sizeof(T)];
        auto w = Encoding::LittleEndian::BufferWriter(value, sizeof(T));
        w.EndianPut(aValue, sizeof(T));

        return WriteValue(aPath, ByteSpan(value));
    }

    /**
     * Read an attribute of type unsigned intX or bool from non-volatile memory.
     *
     * @param [in]     aPath the attribute path for the data being persisted.
     * @param [in,out] aValue where to place the data.
     */
    template <typename T, std::enable_if_t<std::is_unsigned<T>::value, bool> = true >
    CHIP_ERROR ReadValue(const ConcreteAttributePath & aPath, T & aValue)
    {
        uint8_t attrData[sizeof(T)];
        MutableByteSpan tempVal(attrData, sizeof(T));
        // **Note** aType in the ReadValue function is only used to check if the value is of a string type. Since this template
        // function is only enabled for integral values, we know that this case will not occur, so we can pass the enum of an
        // arbitrary integral type.
        auto err = ReadValue(aPath, ZCL_INT8U_ATTRIBUTE_TYPE, sizeof(T), tempVal);
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }

        chip::Encoding::LittleEndian::Reader r(tempVal.data(), tempVal.size());
        r.RawRead(&aValue);
        return r.StatusCode();
    }

    /**
     * Write an attribute value of type nullable unsigned intX to non-volatile memory.
     *
     * @param [in] aPath the attribute path for the data being written.
     * @param [in] aValue the data to write.
     */
    template <typename T, std::enable_if_t<std::is_unsigned<T>::value && !std::is_same<bool, T>::value, bool> = true >
    CHIP_ERROR WriteValue(const ConcreteAttributePath & aPath, DataModel::Nullable<T> & aValue)
    {
        if (aValue.IsNull())
        {
            T nullValue = 0;
            nullValue = ~nullValue;
            return WriteValue(aPath, nullValue);
        }
        return WriteValue(aPath, aValue.Value());
    }

    /**
     * Read an attribute of type nullable unsigned intX from non-volatile memory.
     *
     * @param [in]     aPath the attribute path for the data being persisted.
     * @param [in,out] aValue where to place the data.
     */
    template <typename T, std::enable_if_t<std::is_unsigned<T>::value && !std::is_same<bool, T>::value, bool> = true >
    CHIP_ERROR ReadValue(const ConcreteAttributePath & aPath, DataModel::Nullable<T> & aValue)
    {
        T tempIntegral;
        T nullValue = 0;
        nullValue = ~nullValue;

        CHIP_ERROR err = ReadValue(aPath, tempIntegral);
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }

        if (tempIntegral == nullValue)
        {
            aValue.SetNull();
            return CHIP_NO_ERROR;
        }

        aValue.SetNonNull(tempIntegral);
        return CHIP_NO_ERROR;
    }

};

/**
 * Instance getter for the global AttributePersistenceProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * @return The global AttributePersistenceProvider.  This must never be null.
 */
AttributePersistenceProvider * GetAttributePersistenceProvider();

/**
 * Instance setter for the global AttributePersistenceProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * If the `provider` is nullptr, the value is not changed.
 *
 * @param[in] aProvider the AttributePersistenceProvider implementation to use.
 */
void SetAttributePersistenceProvider(AttributePersistenceProvider * aProvider);

} // namespace app
} // namespace chip

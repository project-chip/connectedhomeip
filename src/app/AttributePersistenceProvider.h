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
#include <app/data-model/Nullable.h>
#include <app/util/attribute-metadata.h>
#include <cstring>
#include <inttypes.h>
#include <lib/support/BufferReader.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/Span.h>

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
    virtual CHIP_ERROR ReadValue(const ConcreteAttributePath & aPath, EmberAfAttributeType aType, size_t aSize,
                                 MutableByteSpan & aValue) = 0;

    /**
     * Get the KVS representation of null for the given type.
     * @tparam T The type for which the null representation should be returned.
     * @return A value of type T that in the KVS represents null.
     */
    template <typename T, std::enable_if_t<std::is_same<bool, T>::value, bool> = true>
    static uint8_t GetNullValueForNullableType()
    {
        return 0xff;
    }

    /**
     * Get the KVS representation of null for the given type.
     * @tparam T The type for which the null representation should be returned.
     * @return A value of type T that in the KVS represents null.
     */
    template <typename T, std::enable_if_t<std::is_unsigned<T>::value && !std::is_same<bool, T>::value, bool> = true>
    static T GetNullValueForNullableType()
    {
        T nullValue = 0;
        nullValue   = T(~nullValue);
        return nullValue;
    }

    /**
     * Get the KVS representation of null for the given type.
     * @tparam T The type for which the null representation should be returned.
     * @return A value of type T that in the KVS represents null.
     */
    template <typename T, std::enable_if_t<std::is_signed<T>::value && !std::is_same<bool, T>::value, bool> = true>
    static T GetNullValueForNullableType()
    {
        T shiftBit = 1;
        return T(shiftBit << ((sizeof(T) * 8) - 1));
    }

    // The following API provides helper functions to simplify the access of commonly used types.
    // The API may not be complete.
    // Currently implemented write and read types are: uint8_t, uint16_t, uint32_t, unit64_t and
    // their nullable varieties, and bool.

    /**
     * Write an attribute value of type intX, uintX or bool to non-volatile memory.
     *
     * @param [in] aPath the attribute path for the data being written.
     * @param [in] aValue the data to write.
     */
    template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
    CHIP_ERROR WriteScalarValue(const ConcreteAttributePath & aPath, T & aValue)
    {
        uint8_t value[sizeof(T)];
        auto w = Encoding::LittleEndian::BufferWriter(value, sizeof(T));
        w.EndianPut(uint64_t(aValue), sizeof(T));

        return WriteValue(aPath, ByteSpan(value));
    }

    /**
     * Read an attribute of type intX, uintX or bool from non-volatile memory.
     *
     * @param [in]     aPath the attribute path for the data being persisted.
     * @param [in,out] aValue where to place the data.
     */
    template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
    CHIP_ERROR ReadScalarValue(const ConcreteAttributePath & aPath, T & aValue)
    {
        uint8_t attrData[sizeof(T)];
        MutableByteSpan tempVal(attrData);
        // **Note** aType in the ReadValue function is only used to check if the value is of a string type. Since this template
        // function is only enabled for integral values, we know that this case will not occur, so we can pass the enum of an
        // arbitrary integral type. 0x20 is the ZCL enum type for ZCL_INT8U_ATTRIBUTE_TYPE.
        auto err = ReadValue(aPath, 0x20, sizeof(T), tempVal);
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }

        chip::Encoding::LittleEndian::Reader r(tempVal.data(), tempVal.size());
        r.RawReadLowLevelBeCareful(&aValue);
        return r.StatusCode();
    }

    /**
     * Write an attribute value of type nullable intX, uintX or bool to non-volatile memory.
     *
     * @param [in] aPath the attribute path for the data being written.
     * @param [in] aValue the data to write.
     */
    template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
    CHIP_ERROR WriteScalarValue(const ConcreteAttributePath & aPath, DataModel::Nullable<T> & aValue)
    {
        if (aValue.IsNull())
        {
            auto nullVal = GetNullValueForNullableType<T>();
            return WriteScalarValue(aPath, nullVal);
        }
        return WriteScalarValue(aPath, aValue.Value());
    }

    /**
     * Read an attribute of type nullable intX, uintX from non-volatile memory.
     *
     * @param [in]     aPath the attribute path for the data being persisted.
     * @param [in,out] aValue where to place the data.
     */
    template <typename T, std::enable_if_t<std::is_integral<T>::value && !std::is_same<bool, T>::value, bool> = true>
    CHIP_ERROR ReadScalarValue(const ConcreteAttributePath & aPath, DataModel::Nullable<T> & aValue)
    {
        T tempIntegral;

        CHIP_ERROR err = ReadScalarValue(aPath, tempIntegral);
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }

        if (tempIntegral == GetNullValueForNullableType<T>())
        {
            aValue.SetNull();
            return CHIP_NO_ERROR;
        }

        aValue.SetNonNull(tempIntegral);
        return CHIP_NO_ERROR;
    }

    /**
     * Read an attribute of type nullable bool from non-volatile memory.
     *
     * @param [in]     aPath the attribute path for the data being persisted.
     * @param [in,out] aValue where to place the data.
     */
    template <typename T, std::enable_if_t<std::is_same<bool, T>::value, bool> = true>
    CHIP_ERROR ReadScalarValue(const ConcreteAttributePath & aPath, DataModel::Nullable<T> & aValue)
    {
        uint8_t tempIntegral;

        CHIP_ERROR err = ReadScalarValue(aPath, tempIntegral);
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }

        if (tempIntegral == GetNullValueForNullableType<T>())
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

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
 * Interface for persisting attribute values. This will always write attributes in storage as little-endian
 * and uses a different key space from AttributePersistenceProvider.
 */

class SafeAttributePersistenceProvider
{
public:
    virtual ~SafeAttributePersistenceProvider() = default;
    SafeAttributePersistenceProvider()          = default;

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
        if constexpr (std::is_signed_v<T>)
        {
            w.EndianPutSigned(aValue, sizeof(T));
        }
        else
        {
            w.EndianPut(aValue, sizeof(T));
        }

        return SafeWriteValue(aPath, ByteSpan(value));
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
        auto err = SafeReadValue(aPath, tempVal);
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }

        Encoding::LittleEndian::Reader r(tempVal.data(), tempVal.size());
        r.RawReadLowLevelBeCareful(&aValue);
        return r.StatusCode();
    }

    /**
     * Write an attribute value of type nullable intX, uintX to non-volatile memory.
     *
     * @param [in] aPath the attribute path for the data being written.
     * @param [in] aValue the data to write.
     */
    template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
    CHIP_ERROR WriteScalarValue(const ConcreteAttributePath & aPath, DataModel::Nullable<T> & aValue)
    {
        typename NumericAttributeTraits<T>::StorageType storageValue;
        if (aValue.IsNull())
        {
            NumericAttributeTraits<T>::SetNull(storageValue);
        }
        else
        {
            NumericAttributeTraits<T>::WorkingToStorage(aValue.Value(), storageValue);
        }
        return WriteScalarValue(aPath, storageValue);
    }

    /**
     * Read an attribute of type nullable intX, uintX from non-volatile memory.
     *
     * @param [in]     aPath the attribute path for the data being persisted.
     * @param [in,out] aValue where to place the data.
     */
    template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
    CHIP_ERROR ReadScalarValue(const ConcreteAttributePath & aPath, DataModel::Nullable<T> & aValue)
    {
        typename NumericAttributeTraits<T>::StorageType storageValue;
        CHIP_ERROR err = ReadScalarValue(aPath, storageValue);
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }

        if (NumericAttributeTraits<T>::IsNullValue(storageValue))
        {
            aValue.SetNull();
            return CHIP_NO_ERROR;
        }

        // Consider checking CanRepresentValue here, so we don't produce invalid data
        // if the storage hands us invalid values.
        aValue.SetNonNull(NumericAttributeTraits<T>::StorageToWorking(storageValue));
        return CHIP_NO_ERROR;
    }

protected:
    /**
     * Write an attribute value from the attribute store (i.e. not a struct or
     * list) to non-volatile memory.
     *
     * @param [in] aPath the attribute path for the data being written.
     * @param [in] aValue the data to write. The value should be stored as-is.
     */
    virtual CHIP_ERROR SafeWriteValue(const ConcreteAttributePath & aPath, const ByteSpan & aValue) = 0;

    /**
     * Read an attribute value from non-volatile memory.
     * It can be assumed that this method will never be called upon to read
     * an attribute of type string or long-string.
     *
     * @param [in]     aPath the attribute path for the data being persisted.
     * @param [in,out] aValue where to place the data.  The size of the buffer
     *                 will be equal to `aValue.size()`.  The callee is expected to adjust
     *                 aValue's size to the actual number of bytes read.
     */
    virtual CHIP_ERROR SafeReadValue(const ConcreteAttributePath & aPath, MutableByteSpan & aValue) = 0;
};

/**
 * Instance getter for the global SafeAttributePersistenceProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * @return The global SafeAttributePersistenceProvider.  This must never be null.
 */
SafeAttributePersistenceProvider * GetSafeAttributePersistenceProvider();

/**
 * Instance setter for the global SafeAttributePersistenceProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * If the `provider` is nullptr, the value is not changed.
 *
 * @param[in] aProvider the SafeAttributePersistenceProvider implementation to use.
 */
void SetSafeAttributePersistenceProvider(SafeAttributePersistenceProvider * aProvider);

} // namespace app
} // namespace chip

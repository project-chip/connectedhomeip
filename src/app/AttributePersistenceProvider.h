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
     * @param [in]     aMetadata the attribute metadata, as a convenience.
     * @param [in,out] aValue where to place the data.  The size of the buffer
     *                 will be equal to `size` member of aMetadata.
     *
     *                 The data is expected to be in native endianness for
     *                 integers and floats.  For strings, see the string
     *                 representation description in the WriteValue
     *                 documentation.
     */
    virtual CHIP_ERROR ReadValue(const ConcreteAttributePath & aPath, const EmberAfAttributeMetadata * aMetadata,
                                 MutableByteSpan & aValue) = 0;
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

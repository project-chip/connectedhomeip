/*
 *    Copyright (c) 2021-2025 Project CHIP Authors
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
#include <lib/support/Span.h>

namespace chip {
namespace app {

/**
 * Describes the type of an attribute stored in persistent storage.
 *
 * Attribute types supported:
 *   - Pascal strings: one or two byte prefixes
 *   - Fixed size scalars (i.e. known size of data)
 *   - Variable size buffers (i.e. opaque bytes)
 */
enum class AttributeValueType : uint8_t
{
    kShortPascal,  // one byte prefix for size (either chars or bytes)
    kLongPascal,   // two byte prefix for size (either chars or bytes). Prefix is LittleEndian encoded.
    kFixedSize,    // fixed size value like uint8_t, uint16_t or float/double or similar
    kVariableSize, // variable size, no size validation done
};

struct AttributeValueInformation
{
    const AttributeValueType type;
    const uint16_t size; // applies to kFixedSize only

    constexpr AttributeValueInformation(AttributeValueType t, uint16_t s) : type(t), size(s) {}

    /// Convenience to specify a type if `Fixed<uint32_t>` for example
    template <typename T>
    static constexpr AttributeValueInformation Fixed()
    {
        return { AttributeValueType::kFixedSize, sizeof(T) };
    }

    static constexpr AttributeValueInformation ShortPascal() { return { AttributeValueType::kShortPascal, 0 }; }
    static constexpr AttributeValueInformation LongPascal() { return { AttributeValueType::kLongPascal, 0 }; }
    static constexpr AttributeValueInformation VariableSize() { return { AttributeValueType::kVariableSize, 0 }; }
};

/**
 * Interface for persisting attribute values.
 *
 * COMPATIBILITY NOTE:
 *   - This generally is assumed to write under a different key space from
 *     SafeAttributePersistenceProvider.
 *
 * TODO: Determine why SafeAttributePersistenceProvider exists compared
 *       to this interface.
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
     * @param [in] aInfo the information about the encoding of the attribute data
     * @param [in] aValue the data to write.
     */
    virtual CHIP_ERROR WriteValue(const ConcreteAttributePath & aPath, const AttributeValueInformation & aInfo,
                                  const ByteSpan & aValue) = 0;

    /**
     * Read an attribute value from non-volatile memory.
     *
     * @param [in]     aPath the attribute path for the data being persisted.
     * @param [in]     aInfo the information about the encoding of the attribute data
     * @param [in,out] aValue where to place the data.
     */
    virtual CHIP_ERROR ReadValue(const ConcreteAttributePath & aPath, const AttributeValueInformation & aInfo,
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

/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CommonIterator.h>
#include <lib/support/Span.h>

#include <cstdint>
#include <cstring>
#include <type_traits>

namespace chip {
namespace app {

class ThreadNetworkDirectoryStorage
{
public:
    static constexpr size_t kMaxThreadDatasetLen = 254;

    /**
     * A Thread Extended PAN ID is an opaque 8 byte value,
     * and can optionally be interpreted as a big-endian number.
     *
     * ExtendedPanId structs or arrays thereof can be directly
     * read from or written to storage and are byte-order independent.
     */
    struct ExtendedPanId final
    {
        uint8_t bytes[8];
        static constexpr size_t size() { return sizeof(bytes); }

        constexpr ExtendedPanId() : bytes{} {}
        explicit ExtendedPanId(ByteSpan source)
        {
            VerifyOrDie(source.size() == size());
            memcpy(bytes, source.data(), size());
        }

        explicit ExtendedPanId(uint64_t number) { Encoding::BigEndian::Put64(bytes, number); }

        constexpr ByteSpan AsSpan() const { return ByteSpan(bytes); }
        uint64_t AsNumber() const { return Encoding::BigEndian::Get64(bytes); }

        bool operator==(const ExtendedPanId & other) const { return memcmp(bytes, other.bytes, size()) == 0; }
        bool operator!=(const ExtendedPanId & other) const { return !(*this == other); }
    };

    static_assert(std::is_trivially_copyable_v<ExtendedPanId> && sizeof(ExtendedPanId) == sizeof(ExtendedPanId::bytes));

    using ExtendedPanIdIterator = CommonIterator<ExtendedPanId>;

    virtual ~ThreadNetworkDirectoryStorage() = default;

    /**
     * Returns the maximum number of networks that can be stored.
     */
    virtual uint8_t Capacity() = 0;

    /**
     *  Creates an iterator over the list of stored networks.
     *  Release() must be called on the iterator after the iteration is finished.
     *  Adding or removing networks during the iteration is not supported.
     *
     *  The iteration order of stored networks should remain stable as long as
     *  as no networks are added or removed.
     *
     *  @retval An instance of ExtendedPanIdIterator on success
     *  @retval nullptr if no iterator instances are available.
     */
    virtual ExtendedPanIdIterator * IterateNetworkIds() = 0;

    /**
     *  Retrieves the dataset associated with the specified Extended PAN ID.
     *
     *  @retval CHIP_ERROR_NOT_FOUND if there is no matching network stored.
     *  @retval CHIP_ERROR_BUFFER_TOO_SMALL if the provided buffer buffer is too small.
     *  @retval CHIP_ERROR_* for other errors.
     */
    virtual CHIP_ERROR GetNetworkDataset(const ExtendedPanId & exPanId, MutableByteSpan & dataset) = 0;

    /**
     *  Adds (or updates, if a matching network already exists) the network with the specified
     *  Extended PAN ID and dataset. Note that the dataset must be treated as an opaque blob;
     *  no validation of any kind is expected to be performed on the dataset contents.
     *
     *  @retval CHIP_ERROR_INVALID_ARGUMENT if the dataset is empty or too long.
     *  @retval CHIP_ERROR_NO_MEMORY if adding the network would exceed the storage capacity.
     *  @retval CHIP_ERROR_* for other errors.
     */
    virtual CHIP_ERROR AddOrUpdateNetwork(const ExtendedPanId & exPanId, ByteSpan dataset) = 0;

    /**
     *  Removes the network with the specified Extended PAN ID.
     *
     *  @retval CHIP_ERROR_NOT_FOUND if there is no matching network stored.
     *  @retval CHIP_ERROR_* for other errors.
     */
    virtual CHIP_ERROR RemoveNetwork(const ExtendedPanId & exPanId) = 0;

    /**
     *  Returns true if a network with the specified Extended PAN ID is stored, or false otherwise.
     */
    virtual bool ContainsNetwork(const ExtendedPanId & exPanId)
    {
        MutableByteSpan empty;
        return GetNetworkDataset(exPanId, empty) != CHIP_ERROR_NOT_FOUND;
    }
};

} // namespace app
} // namespace chip

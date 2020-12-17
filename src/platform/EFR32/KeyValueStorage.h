/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP Inet
 *          Layer on the ESP32 platform.
 *
 */

#pragma once

#include "em_msc.h"
#include <platform/PersistedStorage.h>
/* ignore GCC Wconversion warnings for pigweed */
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include <pw_kvs/crc16_checksum.h>
#include <pw_kvs/flash_memory.h>
#include <pw_kvs/key_value_store.h>

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// KVS is only available for EFR32 when these macros are defined.
#if defined(CHIP_KVS_SECTOR_COUNT) && defined(CHIP_KVS_BASE_SECTOR_INDEX)
#define CHIP_KVS_AVAILABLE 1
#else // defined(CHIP_KVS_SECTOR_COUNT) && defined(CHIP_KVS_BASE_ADDRESS)
#define CHIP_KVS_AVAILABLE 0
#endif // defined(CHIP_KVS_SECTOR_COUNT) && defined(CHIP_KVS_BASE_ADDRESS)

#if defined(CHIP_KVS_AVAILABLE) && CHIP_KVS_AVAILABLE
namespace chip {
namespace Platform {
namespace PersistedStorage {

class Efr32KeyValueStore : public KeyValueStore
{
public:
    /**
     * @brief
     * Initalize the KVS, must be called before using.
     */
    void Init() { mKvs.Init(); }

    /**
     * @brief
     *   Reads the value of an entry in the KVS. The value is read into the
     *   provided buffer and the number of bytes read is returned. If desired,
     *   the read can be started at an offset.
     *
     *   If the output buffer is too small for the value, Get returns
     *   CHIP_ERROR_BUFFER_TOO_SMALL with the number of bytes read returned in
     *   read_bytes_size, which should be the buffer_size.
     *
     *   The remainder of the  value can be read by calling get with an offset.
     *
     * @param[in]     key               The name of the key to get, this is a
     *                                  null-terminated string.
     * @param[in,out] buffer            A buffer to read the value into.
     * @param[in]     buffer_size       The size of the buffer in bytes.
     * @param[in]     read_bytes_size   The number of bytes which were
     *                                  copied into the buffer. Optionally can
     *                                  provide nullptr if not needed.
     * @param[in]     offset_bytes      The offset byte index to start the read.
     *
     * @return CHIP_NO_ERROR the entry was successfully read
     *         CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND the key is not
     *                                                       present in the KVS
     *         CHIP_ERROR_INTEGRITY_CHECK_FAILED found the entry, but the data
     *                                           was corrupted
     *         CHIP_ERROR_BUFFER_TOO_SMALL the buffer could not fit the entire
     *                                     value, but as many bytes as possible
     *                                     were written to it
     *         CHIP_ERROR_WELL_UNINITIALIZED the KVS is not initialized
     *         CHIP_ERROR_INVALID_ARGUMENT key is empty or too long or value is
     *                                     too large
     */
    CHIP_ERROR Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size = nullptr,
                   size_t offset = 0) const override;

    /**
     * @brief
     * Removes a key-value entry from the KVS.
     *
     * @param[in]      key       The name of the key to delete, this is a
     *                           null-terminated string.
     *
     * @return CHIP_NO_ERROR the entry was successfully deleted.
     *         CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND the key is not
     *                                                      present in the KVS
     *         CHIP_ERROR_INTEGRITY_CHECK_FAILED checksum validation failed after
     *                                           erasing data
     *         CHIP_ERROR_PERSISTED_STORAGE_FAILED failed to erase the value.
     *         CHIP_ERROR_WELL_UNINITIALIZED the KVS is not initialized
     *         CHIP_ERROR_INVALID_ARGUMENT key is empty or too long
     */
    CHIP_ERROR Delete(const char * key) override;

    /**
     * @brief
     * Erases all data in the KVS partition, KVS needs to be initalized after
     * this operation.
     *
     * @return CHIP_NO_ERROR the partiton was erased.
     *         CHIP_ERROR_TIMEOUT timed out while doing erase.
     *         CHIP_ERROR_ACCESS_DENIED flash locked, erase failed.
     */
    CHIP_ERROR ErasePartition();

    /**
     * @brief
     * Adds a key-value entry to the KVS. If the key was already present, its
     * value is overwritten.
     *
     * @param[in]  key          The name of the key to update, this is a
     *                          null-terminated string.
     * @param[in]  value        Pointer to the data.
     * @param[in]  value_size   Size of the data.
     *
     * @return CHIP_NO_ERROR the entry was successfully added or updated
     *         CHIP_ERROR_INTEGRITY_CHECK_FAILED checksum validation failed after
     *                                           writing the data
     *         CHIP_ERROR_PERSISTED_STORAGE_FAILED failed to write the value.
     *         CHIP_ERROR_WELL_UNINITIALIZED the KVS is not initialized
     *         CHIP_ERROR_INVALID_ARGUMENT key is empty or too long or value is
     *                                     too large
     */
    CHIP_ERROR Put(const char * key, const void * value, size_t value_size) override;

private:
    // KVS flash interface
    class Efr32FlashMemory : public pw::kvs::FlashMemory
    {
    public:
        Efr32FlashMemory() : pw::kvs::FlashMemory(FLASH_PAGE_SIZE, FLASH_SIZE / FLASH_PAGE_SIZE, sizeof(uint32_t), FLASH_BASE) {}

        // Enabling flash handled by platform
        pw::Status Enable() override { return pw::Status::OK; }
        pw::Status Disable() override { return pw::Status::OK; }
        bool IsEnabled() const override { return true; }

        pw::Status Erase(Address flash_address, size_t num_sectors) override
        {
            for (size_t i = 0; i < num_sectors; i++)
            {
                auto status =
                    MscStatusToPwStatus(MSC_ErasePage(reinterpret_cast<uint32_t *>(flash_address + i * sector_size_bytes())));
                if (!status.ok())
                {
                    return status;
                }
            }
            return pw::Status::OK;
        }

        pw::StatusWithSize Read(Address address, std::span<std::byte> output) override
        {
            memcpy(output.data(), reinterpret_cast<void *>(address), output.size());
            return pw::StatusWithSize::Ok(output.size());
        }

        pw::StatusWithSize Write(Address destination_flash_address, std::span<const std::byte> data) override
        {
            return pw::StatusWithSize(MscStatusToPwStatus(MSC_WriteWord(reinterpret_cast<uint32_t *>(destination_flash_address),
                                                                        data.data(), data.size())),
                                      data.size());
        }

    private:
        static pw::Status MscStatusToPwStatus(MSC_Status_TypeDef msc_status)
        {
            switch (msc_status)
            {
            case mscReturnOk:
                return pw::Status::OK;
            case mscReturnUnaligned:
            case mscReturnInvalidAddr:
                return pw::Status::INVALID_ARGUMENT;
            case mscReturnLocked:
                return pw::Status::PERMISSION_DENIED;
            case mscReturnTimeOut:
                return pw::Status::DEADLINE_EXCEEDED;
            default:
                break;
            }
            return pw::Status::INTERNAL;
        }
    };

    static constexpr size_t kMaxEntries = 50;
    pw::kvs::ChecksumCrc16 mKvsChecksum;
    const pw::kvs::EntryFormat kEntryFormat{ .magic = 0x64d51134, .checksum = &mKvsChecksum };

    Efr32FlashMemory mFlash;
    pw::kvs::FlashPartition mKvsPartition{ &mFlash, CHIP_KVS_BASE_SECTOR_INDEX, CHIP_KVS_SECTOR_COUNT };
    pw::kvs::KeyValueStoreBuffer<kMaxEntries, CHIP_KVS_SECTOR_COUNT> mKvs{ &mKvsPartition, kEntryFormat };
};

/**
 * @brief
 * Returns a reference to the singleton for the default Chip KVS storage. This is
 * the same object as ChipKeyValueStoreInstance, but returns the object of the
 * type Efr32KeyValueStore.
 */
Efr32KeyValueStore & ChipEfr32KeyValueStoreInstance();

} // namespace PersistedStorage
} // namespace Platform
} // namespace chip
#endif //  defined(CHIP_KVS_AVAILABLE) && CHIP_KVS_AVAILABLE

/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *          Platform-specific key value storage implementation for EFR32.
 *
 */

#pragma once

#include "em_msc.h"

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

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

#if defined(CHIP_KVS_AVAILABLE) && CHIP_KVS_AVAILABLE
class KeyValueStoreManagerImpl final : public KeyValueStoreManager
{
    // Allow the KeyValueStoreManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class KeyValueStoreManager;

public:
    void Init() { mKvs.Init(); }

    CHIP_ERROR _Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size = nullptr, size_t offset = 0) const;

    CHIP_ERROR _Delete(const char * key);

    /**
     * @brief
     * Erases all data in the KVS partition, KVS needs to be initialized after
     * this operation.
     *
     * @return CHIP_NO_ERROR the partiton was erased.
     *         CHIP_ERROR_TIMEOUT timed out while doing erase.
     *         CHIP_ERROR_ACCESS_DENIED flash locked, erase failed.
     */
    CHIP_ERROR ErasePartition();

    CHIP_ERROR _Put(const char * key, const void * value, size_t value_size);

private:
    // KVS flash interface
    class Efr32FlashMemory : public pw::kvs::FlashMemory
    {
    public:
        Efr32FlashMemory() : pw::kvs::FlashMemory(FLASH_PAGE_SIZE, FLASH_SIZE / FLASH_PAGE_SIZE, sizeof(uint32_t), FLASH_BASE) {}

        // Enabling flash handled by platform
        pw::Status Enable() override { return pw::OkStatus(); }
        pw::Status Disable() override { return pw::OkStatus(); }
        bool IsEnabled() const override { return true; }

        pw::Status Erase(Address flash_address, size_t num_sectors) override
        {
            assert((flash_address % sizeof(uint32_t)) == 0);
            for (size_t i = 0; i < num_sectors; i++)
            {
                auto status =
                    MscStatusToPwStatus(MSC_ErasePage(reinterpret_cast<uint32_t *>(flash_address + i * sector_size_bytes())));
                if (!status.ok())
                {
                    return status;
                }
            }
            return pw::OkStatus();
        }

        pw::StatusWithSize Read(Address address, std::span<std::byte> output) override
        {
            memcpy(output.data(), reinterpret_cast<void *>(address), output.size());
            return pw::StatusWithSize(output.size());
        }

        pw::StatusWithSize Write(Address destination_flash_address, std::span<const std::byte> data) override
        {
            assert((destination_flash_address % sizeof(uint32_t)) == 0);
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
                return pw::OkStatus();
            case mscReturnUnaligned:
            case mscReturnInvalidAddr:
                return pw::Status::InvalidArgument();
            case mscReturnLocked:
                return pw::Status::PermissionDenied();
            case mscReturnTimeOut:
                return pw::Status::DeadlineExceeded();
            default:
                break;
            }
            return pw::Status::Internal();
        }
    };

    static constexpr size_t kMaxEntries = 50;
    pw::kvs::ChecksumCrc16 mKvsChecksum;
    const pw::kvs::EntryFormat kEntryFormat{ .magic = 0x64d51134, .checksum = &mKvsChecksum };

    Efr32FlashMemory mFlash;
    pw::kvs::FlashPartition mKvsPartition{ &mFlash, CHIP_KVS_BASE_SECTOR_INDEX, CHIP_KVS_SECTOR_COUNT };
    pw::kvs::KeyValueStoreBuffer<kMaxEntries, CHIP_KVS_SECTOR_COUNT> mKvs{ &mKvsPartition, kEntryFormat };

    // ===== Members for internal use by the following friends.

    friend KeyValueStoreManager & KeyValueStoreMgr();
    friend KeyValueStoreManagerImpl & KeyValueStoreMgrImpl();

    static KeyValueStoreManagerImpl sInstance;
};

#else //  defined(CHIP_KVS_AVAILABLE) && CHIP_KVS_AVAILABLE

// Empty implementation which just asserts if used
class KeyValueStoreManagerImpl final : public KeyValueStoreManager
{
public:
    CHIP_ERROR _Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size = nullptr, size_t offset = 0) const
    {
        assert(CHIP_KVS_AVAILABLE);
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR _Delete(const char * key)
    {
        assert(CHIP_KVS_AVAILABLE);
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR _Put(const char * key, const void * value, size_t value_size)
    {
        assert(CHIP_KVS_AVAILABLE);
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

private:
    // ===== Members for internal use by the following friends.

    friend KeyValueStoreManager & KeyValueStoreMgr();
    friend KeyValueStoreManagerImpl & KeyValueStoreMgrImpl();

    static KeyValueStoreManagerImpl sInstance;
};

#endif //  defined(CHIP_KVS_AVAILABLE) && CHIP_KVS_AVAILABLE

/**
 * Returns the public interface of the KeyValueStoreManager singleton object.
 *
 * Chip applications should use this to access features of the KeyValueStoreManager object
 * that are common to all platforms.
 */
inline KeyValueStoreManager & KeyValueStoreMgr(void)
{
    return KeyValueStoreManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the KeyValueStoreManager singleton object.
 *
 * Chip applications can use this to gain access to features of the KeyValueStoreManager
 * that are specific to the ESP32 platform.
 */
inline KeyValueStoreManagerImpl & KeyValueStoreMgrImpl(void)
{
    return KeyValueStoreManagerImpl::sInstance;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip

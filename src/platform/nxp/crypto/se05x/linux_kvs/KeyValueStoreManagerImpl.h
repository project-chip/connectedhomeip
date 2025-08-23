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
 *          Platform-specific implementation of KVS for SE05x + Linux.
 */

#pragma once

#include "CHIPCryptoPALHsm_se05x_readClusters.h"
#include "CHIPCryptoPALHsm_se05x_utils.h"
#include <lib/core/TLV.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/Base64.h>
#include <platform/Linux/CHIPLinuxStorage.h>
#include <unistd.h>
#include <vector>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

class KeyValueStoreManagerImpl : public KeyValueStoreManager
{
public:
    /**
     * @brief
     * Initialize the KVS, must be called before using.
     */

    CHIP_ERROR Init(const char * file)
    {
        ChipLogDetail(Crypto, "SE05x :: KVS Initialization ");

        if (se05x_is_nfc_commissioning_done() != CHIP_NO_ERROR)
        {
            ChipLogDetail(Crypto, "SE05x :: No NFC commissioned data found ");
            VerifyOrReturnError(se05x_close_session() == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);
            return mStorage.Init(file);
        }

        ChipLogDetail(Crypto, "SE05x :: NFC commissioned data found. Reading the contents from SE05x");

        /*  NFC commissioning is done, read the credentials from SE and
            write to chip kvs file.
        */

#if 0
        if (access(file, F_OK) == 0) {
            if (remove(file) != 0) {
                return CHIP_ERROR_INTERNAL;
            }
        }
#endif

        CHIP_ERROR fCreate = mStorage.Init(file);
        if (fCreate != CHIP_NO_ERROR)
        {
            return fCreate;
        }

        CHIP_ERROR status = CHIP_NO_ERROR;

        uint8_t buffer_cert_1[chip::Credentials::kMaxCHIPCertLength];
        uint8_t buffer_cert_2[chip::Credentials::kMaxCHIPCertLength];
        uint8_t buffer_cert_3[chip::Credentials::kMaxCHIPCertLength];
        uint8_t buffer_cert_4[chip::Credentials::kMaxDERCertLength];

        uint8_t * nocBuf           = &buffer_cert_1[0];
        size_t nocBufLen           = sizeof(buffer_cert_1);
        uint8_t * rootCertBuf      = &buffer_cert_2[0];
        size_t rootCertBufLen      = sizeof(buffer_cert_2);
        uint8_t * icacBuf          = &buffer_cert_3[0];
        size_t icacBufLen          = sizeof(buffer_cert_3);
        uint8_t * ipkBuf           = &buffer_cert_4[0];
        size_t ipkLen              = sizeof(buffer_cert_4);
        uint8_t * refkeyBuf        = NULL;
        size_t refkeyBufLen        = 0;
        uint8_t * aclBuf           = NULL;
        size_t aclLen              = 0;
        uint8_t * fabgrpData       = NULL;
        size_t fabrgrpLen          = 0;
        uint8_t * metaData         = NULL;
        size_t metaLen             = 0;
        uint8_t * fabindexinfoData = NULL;
        size_t fabindexinfoLen     = 0;

        char nocKey[SE05X_KEY_BUFFER_LEN_KVS_FUNCTIONS]          = { 0 };
        char rootCertKey[SE05X_KEY_BUFFER_LEN_KVS_FUNCTIONS]     = { 0 };
        char icackey[SE05X_KEY_BUFFER_LEN_KVS_FUNCTIONS]         = { 0 };
        char ipkey[SE05X_KEY_BUFFER_LEN_KVS_FUNCTIONS]           = { 0 };
        char opKey[SE05X_KEY_BUFFER_LEN_KVS_FUNCTIONS]           = { 0 };
        char aclKey[SE05X_KEY_BUFFER_LEN_KVS_FUNCTIONS]          = { 0 };
        char fabgrpkey[SE05X_KEY_BUFFER_LEN_KVS_FUNCTIONS]       = { 0 };
        char metakey[SE05X_KEY_BUFFER_LEN_KVS_FUNCTIONS]         = { 0 };
        char fabindexinfokey[SE05X_KEY_BUFFER_LEN_KVS_FUNCTIONS] = { 0 };

        status = se05x_read_operational_credentials_cluster(&nocKey[0], nocBuf, &nocBufLen, &rootCertKey[0], rootCertBuf,
                                                            &rootCertBufLen, &icackey[0], icacBuf, &icacBufLen, &ipkey[0], ipkBuf,
                                                            &ipkLen);
        VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

        status = _Put(nocKey, nocBuf, nocBufLen);
        VerifyOrReturnError(status == CHIP_NO_ERROR, status);
        status = _Put(rootCertKey, rootCertBuf, rootCertBufLen);
        VerifyOrReturnError(status == CHIP_NO_ERROR, status);
        status = _Put(icackey, icacBuf, icacBufLen);
        VerifyOrReturnError(status == CHIP_NO_ERROR, status);
        status = _Put(ipkey, ipkBuf, ipkLen);
        VerifyOrReturnError(status == CHIP_NO_ERROR, status);

        memset(buffer_cert_1, 0, sizeof(buffer_cert_1));
        refkeyBuf    = &buffer_cert_1[0];
        refkeyBufLen = sizeof(buffer_cert_1);
        status       = se05x_read_node_operational_keypair(&opKey[0], refkeyBuf, &refkeyBufLen);
        VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

        status = _Put(opKey, refkeyBuf, refkeyBufLen);
        VerifyOrReturnError(status == CHIP_NO_ERROR, status);

        memset(buffer_cert_4, 0, sizeof(buffer_cert_4));
        aclBuf = &buffer_cert_4[0];
        aclLen = sizeof(buffer_cert_4);
        status = se05x_read_acl_data(&aclKey[0], aclBuf, &aclLen);
        VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

        status = _Put(aclKey, aclBuf, aclLen);
        VerifyOrReturnError(status == CHIP_NO_ERROR, status);

        memset(buffer_cert_1, 0, sizeof(buffer_cert_1));
        fabgrpData = &buffer_cert_1[0];
        fabrgrpLen = sizeof(buffer_cert_1);
        status     = se05x_read_fabric_groups(&fabgrpkey[0], fabgrpData, &fabrgrpLen);
        VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

        status = _Put(fabgrpkey, fabgrpData, fabrgrpLen);
        VerifyOrReturnError(status == CHIP_NO_ERROR, status);

        memset(buffer_cert_1, 0, sizeof(buffer_cert_1));
        metaData = &buffer_cert_1[0];
        metaLen  = sizeof(buffer_cert_1);
        status   = se05x_read_meta_data(&metakey[0], metaData, &metaLen);
        VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

        status = _Put(metakey, metaData, metaLen);
        VerifyOrReturnError(status == CHIP_NO_ERROR, status);

        memset(buffer_cert_1, 0, sizeof(buffer_cert_1));
        fabindexinfoData = &buffer_cert_1[0];
        fabindexinfoLen  = sizeof(buffer_cert_1);
        status           = se05x_read_fabric_index_info_data(&fabindexinfokey[0], fabindexinfoData, &fabindexinfoLen);
        VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

        status = _Put(fabindexinfokey, fabindexinfoData, fabindexinfoLen);
        VerifyOrReturnError(status == CHIP_NO_ERROR, status);

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR _Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size = nullptr, size_t offset = 0);
    CHIP_ERROR _Delete(const char * key);
    CHIP_ERROR _Put(const char * key, const void * value, size_t value_size);

private:
    DeviceLayer::Internal::ChipLinuxStorage mStorage;

    // ===== Members for internal use by the following friends.
    friend KeyValueStoreManager & KeyValueStoreMgr();
    friend KeyValueStoreManagerImpl & KeyValueStoreMgrImpl();

    static KeyValueStoreManagerImpl sInstance;
};

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

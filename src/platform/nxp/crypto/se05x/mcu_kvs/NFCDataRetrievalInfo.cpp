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

#include "NFCDataRetrievalInfo.hpp"

#include <credentials/CHIPCert.h>
#include <cstdio>
#include <cstring>
#include <inttypes.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConnectivityManager.h>
#include <platform/KeyValueStoreManager.h>
#include <platform/nxp/crypto/se05x/CHIPCryptoPALHsm_se05x_utils.h>
#include <platform/nxp/crypto/se05x/kvs_utilities/CHIPCryptoPALHsm_se05x_readClusters.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ThreadStackManager.h>
#endif

namespace chip {
namespace DeviceLayer {
namespace NFCCommissioning {

NFCDataRetrievalInfo NFCDataRetrievalInfo::sInstance;

NFCDataRetrievalInfo & NFCDataRetrievalInfo::GetInstance()
{
    return sInstance;
}

CHIP_ERROR NFCDataRetrievalInfo::Init()
{
    ChipLogDetail(Crypto, "SE05x :: NFC Data Retrieval Initialization");

    // Reset breadcrumb and commissioning state
    ReturnErrorOnFailure(se05x_reset_breadcrumb());
    ReturnErrorOnFailure(se05x_reset_iscomm_without_power(false));

    // Check if NFC commissioning was performed
    if (se05x_is_nfc_commissioning_done() != CHIP_NO_ERROR)
    {
        ChipLogDetail(Crypto, "SE05x :: No NFC commissioned data found");
        ReturnErrorOnFailure(se05x_close_session());

#if defined(CONFIG_SE05X_HOST_GPIO)
        // Check commissioning status and initialize GPIO if needed
        CHIP_ERROR gpioStatus = CheckCommissioningStatusAndInitGPIO();
        if (gpioStatus == CHIP_NO_ERROR)
        {
            ChipLogProgress(Crypto, "SE05x :: GPIO notification initialized for NFC");
        }
        else if (gpioStatus != CHIP_ERROR_INCORRECT_STATE)
        {
            ChipLogError(Crypto, "SE05x :: Failed to initialize GPIO notification (error: %" CHIP_ERROR_FORMAT ")",
                         gpioStatus.Format());
        }
#endif
        return CHIP_NO_ERROR;
    }

    ChipLogDetail(Crypto, "SE05x :: NFC commissioned data found. Reading contents from SE05x");

    // Disable NFC commissioning after successful detection
    ReturnErrorOnFailure(se05x_disable_nfc_commision());

    // Check if KVS is already synchronized with SE05x
    if (IsKVSAlreadySynchronized())
    {
        ChipLogDetail(Crypto, "SE05x :: KVS is already updated with SE contents. Skipping synchronization.");
        return CHIP_NO_ERROR;
    }

    // Mark that we're reading from SE05x for fail-safe timer handling
    mSe05xReadFailSafe = 1;

    // Synchronize all credentials from SE05x to KVS
    ReturnErrorOnFailure(SynchronizeOperationalCredentials());
    ReturnErrorOnFailure(SynchronizeNetworkCredentials());

    ChipLogDetail(Crypto, "SE05x :: Successfully synchronized all credentials to KVS");
    return CHIP_NO_ERROR;
}

uint32_t NFCDataRetrievalInfo::GetRemainingFailSafeTimerForSE05x()
{
    // If not reading from SE05x for the first time after NFC commissioning, return 0
    if (mSe05xReadFailSafe == 0)
    {
        ChipLogDetail(Crypto, "SE05x: Fail-safe timer not active (not first read after NFC commissioning)");
        if (CHIP_NO_ERROR != se05x_close_session())
        {
            ChipLogError(Crypto, "SE05x: Error in session close");
        }
        return 0;
    }

    uint16_t fail_safe_time = 0;
    CHIP_ERROR status       = se05x_get_remain_fail_safe_time(&fail_safe_time);

    if (status != CHIP_NO_ERROR)
    {
        ChipLogError(Crypto, "SE05x: Failed to read fail-safe time from secure element (error: %" CHIP_ERROR_FORMAT ")",
                     status.Format());
        // Reset the flag since we attempted to read
        mSe05xReadFailSafe = 0;
        if (CHIP_NO_ERROR != se05x_close_session())
        {
            ChipLogError(Crypto, "SE05x: Error in session close");
        }
        return 0;
    }

    // Reset the flag after successful read
    mSe05xReadFailSafe = 0;

    if (CHIP_NO_ERROR != se05x_close_session())
    {
        ChipLogError(Crypto, "SE05x: Error in session close");
    }
    return fail_safe_time;
}

#if defined(CONFIG_SE05X_HOST_GPIO)
CHIP_ERROR NFCDataRetrievalInfo::CheckCommissioningStatusAndInitGPIO()
{
    constexpr size_t kMaxBufferSize        = 256;
    constexpr const char * kFabricIndexKey = "g/fidx";

    uint8_t fab_idx[kMaxBufferSize] = { 0 };
    size_t fab_idx_len              = sizeof(fab_idx);

    // Check if fabric index info exists in KVS (indicates device is commissioned)
    CHIP_ERROR status = PersistedStorage::KeyValueStoreMgrImpl()._Get(kFabricIndexKey, fab_idx, sizeof(fab_idx), &fab_idx_len, 0);

    if (status == CHIP_NO_ERROR)
    {
        ChipLogProgress(Crypto, "SE05x :: Device is commissioned (fabric index found), skipping GPIO notification");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (status != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogError(Crypto, "SE05x :: Failed to read fabric index from KVS (error: %" CHIP_ERROR_FORMAT ")", status.Format());
        return status;
    }

    ChipLogProgress(Crypto, "SE05x :: Device not commissioned, initializing GPIO notification for NFC");

    // Initialize GPIO notification for NFC commissioning
    void * gpioResult = se05x_host_gpio_notification_monitor_init(nullptr);
    if (gpioResult != NULL)
    {
        ChipLogError(Crypto, "SE05x :: Failed to initialize GPIO notification ");
        return CHIP_ERROR_INTERNAL;
    }

    ChipLogProgress(Crypto, "SE05x :: GPIO notification successfully initialized");
    return CHIP_NO_ERROR;
}
#endif

bool NFCDataRetrievalInfo::IsKVSAlreadySynchronized()
{
    constexpr size_t kMaxKeySize     = 32;
    constexpr size_t kMaxOperKeySize = 256;

    char kvs_key_name[kMaxKeySize];
    uint8_t kvs_node_oper_key[kMaxOperKeySize] = { 0 };
    uint8_t se_node_oper_key[kMaxOperKeySize]  = { 0 };
    size_t kvs_key_len                         = sizeof(kvs_node_oper_key);
    size_t se_key_len                          = sizeof(se_node_oper_key);

    // Build key name for operational keypair
    VerifyOrReturnValue(snprintf(kvs_key_name, sizeof(kvs_key_name), "f/%x/o", 1) > 0, false);

    // Try to read operational key from KVS
    CHIP_ERROR status =
        PersistedStorage::KeyValueStoreMgrImpl()._Get(kvs_key_name, kvs_node_oper_key, sizeof(kvs_node_oper_key), &kvs_key_len, 0);
    VerifyOrReturnValue(status == CHIP_NO_ERROR, false);

    // Read operational key from SE05x
    status = se05x_read_node_operational_keypair(se_node_oper_key, &se_key_len);
    VerifyOrReturnValue(status == CHIP_NO_ERROR, false);

    // Compare keys - if identical, KVS is already synchronized
    return (kvs_key_len == se_key_len) && (memcmp(kvs_node_oper_key, se_node_oper_key, kvs_key_len) == 0);
}

CHIP_ERROR NFCDataRetrievalInfo::SynchronizeOperationalCredentials()
{
    constexpr size_t kMaxBufferSize  = (2 * chip::Credentials::kMaxCHIPCertLength) + 32;
    constexpr size_t kMaxKeyNameSize = 32;

    uint8_t buffer[kMaxBufferSize] = { 0 };
    char key_name[kMaxKeyNameSize] = { 0 };
    size_t buffer_len;

    CHIP_ERROR status = CHIP_NO_ERROR;

    // Read and store operational keypair
    buffer_len = sizeof(buffer);
    ReturnErrorOnFailure(se05x_read_node_operational_keypair(buffer, &buffer_len));
    VerifyOrReturnError(snprintf(key_name, sizeof(key_name), "f/%x/o", 1) > 0, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgrImpl()._Put(key_name, buffer, buffer_len));

    // Read and store Node Operational Certificate (NOC)
    buffer_len = sizeof(buffer);
    ReturnErrorOnFailure(se05x_read_node_oper_cert(buffer, &buffer_len));
    VerifyOrReturnError(snprintf(key_name, sizeof(key_name), "f/%x/n", 1) > 0, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgrImpl()._Put(key_name, buffer, buffer_len));

    const uint32_t fabric_id = se05x_get_fabric_id();

    // Read and store Root Certificate
    buffer_len = sizeof(buffer);
    ReturnErrorOnFailure(se05x_read_root_cert(buffer, &buffer_len));
    VerifyOrReturnError(snprintf(key_name, sizeof(key_name), "f/%" PRIx32 "/r", fabric_id) > 0, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgrImpl()._Put(key_name, buffer, buffer_len));

    // Read and store Intermediate CA Certificate (if present)
    buffer_len = sizeof(buffer);
    status     = se05x_read_ICA(buffer, &buffer_len);
    if (status == CHIP_NO_ERROR && buffer_len > 0)
    {
        VerifyOrReturnError(snprintf(key_name, sizeof(key_name), "f/%" PRIx32 "/i", fabric_id) > 0, CHIP_ERROR_INTERNAL);
        ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgrImpl()._Put(key_name, buffer, buffer_len));
    }

    // Read and store Identity Protection Key (IPK)
    buffer_len = sizeof(buffer);
    ReturnErrorOnFailure(se05x_read_ipk(buffer, &buffer_len));
    VerifyOrReturnError(snprintf(key_name, sizeof(key_name), "f/%" PRIx32 "/k/0", fabric_id) > 0, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgrImpl()._Put(key_name, buffer, buffer_len));

    // Read and store Access Control List (ACL)
    buffer_len = sizeof(buffer);
    ReturnErrorOnFailure(se05x_read_acl_data(buffer, &buffer_len));
    VerifyOrReturnError(snprintf(key_name, sizeof(key_name), "f/%" PRIx32 "/ac/0/0", fabric_id) > 0, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgrImpl()._Put(key_name, buffer, buffer_len));

    // Read and store fabric groups
    buffer_len = sizeof(buffer);
    ReturnErrorOnFailure(se05x_read_fabric_groups(buffer, &buffer_len));
    VerifyOrReturnError(snprintf(key_name, sizeof(key_name), "f/%" PRIx32 "/g", fabric_id) > 0, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgrImpl()._Put(key_name, buffer, buffer_len));

    // Read and store metadata
    buffer_len = sizeof(buffer);
    ReturnErrorOnFailure(se05x_read_meta_data(buffer, &buffer_len));
    VerifyOrReturnError(snprintf(key_name, sizeof(key_name), "f/%" PRIx32 "/m", fabric_id) > 0, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgrImpl()._Put(key_name, buffer, buffer_len));

    // Read and store fabric index info
    buffer_len = sizeof(buffer);
    ReturnErrorOnFailure(se05x_read_fabric_index_info_data(buffer, &buffer_len));
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgrImpl()._Put("g/fidx", buffer, buffer_len));

    return CHIP_NO_ERROR;
}

CHIP_ERROR NFCDataRetrievalInfo::SynchronizeNetworkCredentials()
{
    constexpr size_t kMaxBufferSize = 512;

    uint8_t buffer[kMaxBufferSize];
    char password[DeviceLayer::Internal::kMaxWiFiKeyLength] = { 0 };
    size_t ssid_len                                         = sizeof(this->op_data_set);
    size_t password_len                                     = sizeof(password);
    uint32_t network_cred_id                                = 0;

    // Get network credential ID from network commissioning cluster
    ReturnErrorOnFailure(se05x_net_id_from_net_comm_cluster(&network_cred_id));

    // Read WiFi/Thread credentials
    CHIP_ERROR status = se05x_read_wifi_and_thread_credentials(buffer, sizeof(buffer), op_data_set, &ssid_len, password,
                                                               &password_len, op_data_set, &op_data_set_len, &network_cred_id);

    if (status != CHIP_NO_ERROR)
    {
        ChipLogDetail(Crypto, "SE05x: Reading network credentials from secure element failed");
        return CHIP_NO_ERROR; // Non-fatal - continue without network credentials
    }

    // Store WiFi credentials if present
    if (ssid_len > 0 && password_len > 0)
    {
        ChipLogDetail(Crypto, "SE05x: Setting Wi-Fi credentials");
        ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgrImpl()._Put("wifi-ssid", op_data_set, ssid_len));
        ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgrImpl()._Put("wifi-pass", password, password_len));
    }
    // Store Thread credentials if available
    else if (op_data_set_len > 0)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#ifdef __linux__
        // On Linux, Thread provisioning is handled by LinuxThreadDriver in PostInit()
        // Just store the operational dataset for later use
        ChipLogDetail(Crypto, "SE05x: Thread operational data stored (length: %zu bytes), will be provisioned in PostInit",
                      op_data_set_len);
#else
        // On embedded platforms, provision Thread directly
        ChipLogDetail(Crypto, "SE05x: Setting Thread operational data");
        ByteSpan dataset(reinterpret_cast<const uint8_t *>(op_data_set), op_data_set_len);
        ReturnErrorOnFailure(DeviceLayer::ThreadStackMgrImpl().SetThreadProvision(dataset));
        if (DeviceLayer::ThreadStackMgrImpl().IsThreadProvisioned())
        {
            ReturnErrorOnFailure(DeviceLayer::ThreadStackMgrImpl().SetThreadEnabled(true));
        }
        else
        {
            ChipLogDetail(Crypto, "SE05x: Thread not provisioned");
        }
#endif // __linux__
#else
        ChipLogDetail(Crypto, "SE05x: SE05x commissioned for Thread, but example is not built with Thread support");
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
    }
    else
    {
        ChipLogDetail(Crypto, "SE05x: No valid network credentials found");
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR NFCDataRetrievalInfo::GetOperationalDataSetFormSE05x(char * opDataSet, size_t * opDataSetLen)
{
    VerifyOrReturnError(opDataSet != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(opDataSetLen != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // If we already have the operational dataset cached, just return it
    if (op_data_set_len > 0)
    {
        VerifyOrReturnError(*opDataSetLen >= op_data_set_len, CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(opDataSet, op_data_set, op_data_set_len);
        *opDataSetLen = op_data_set_len;
        return CHIP_NO_ERROR;
    }

    // No cached data available
    *opDataSetLen = 0;
    return CHIP_NO_ERROR;
}

} // namespace NFCCommissioning
} // namespace DeviceLayer
} // namespace chip

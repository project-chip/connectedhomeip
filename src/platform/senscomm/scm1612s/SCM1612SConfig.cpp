/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          Utilities for accessing persisted device configuration on
 *          platforms based on the MediaTek SDK.
 */
/* this file behaves like a config.h, comes first */
#include <functional>

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/PlatformManager.h>
#include <platform/senscomm/scm1612s/SCM1612SConfig.h>

#include <lib/core/CHIPEncoding.h>
#include <platform/internal/testing/ConfigUnitTest.h>

#include "FreeRTOS.h"
#ifdef __no_stub__
#include "nvdm.h"
#endif /* __no_stub__ */

namespace chip {
namespace DeviceLayer {
namespace Internal {

// Factory config keys
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_SerialNum   = { .Namespace = kConfigNamespace_ChipFactory, .Name = "serial-num" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_UniqueId    = { .Namespace = kConfigNamespace_ChipFactory, .Name = "unique-id" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_MfrDeviceId = { .Namespace = kConfigNamespace_ChipFactory, .Name = "device-id" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_MfrDeviceCert       = { .Namespace = kConfigNamespace_ChipFactory,
                                                                         .Name      = "device-cert" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_MfrDevicePrivateKey = { .Namespace = kConfigNamespace_ChipFactory,
                                                                         .Name      = "device-key" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_ManufacturingDate   = { .Namespace = kConfigNamespace_ChipFactory,
                                                                         .Name      = "mfg-date" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_SetupPinCode = { .Namespace = kConfigNamespace_ChipFactory, .Name = "pin-code" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_MfrDeviceICACerts     = { .Namespace = kConfigNamespace_ChipFactory,
                                                                           .Name      = "device-ca-certs" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_SetupDiscriminator    = { .Namespace = kConfigNamespace_ChipFactory,
                                                                           .Name      = "discriminator" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_Spake2pIterationCount = { .Namespace = kConfigNamespace_ChipFactory,
                                                                           .Name      = "iteration-count" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_Spake2pSalt     = { .Namespace = kConfigNamespace_ChipFactory, .Name = "salt" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_Spake2pVerifier = { .Namespace = kConfigNamespace_ChipFactory,
                                                                     .Name      = "verifier" };
// CHIP Config Keys
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_FabricId      = { .Namespace = kConfigNamespace_ChipConfig, .Name = "fabric-id" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_ServiceConfig = { .Namespace = kConfigNamespace_ChipConfig,
                                                                   .Name      = "service-config" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_PairedAccountId = { .Namespace = kConfigNamespace_ChipConfig,
                                                                     .Name      = "account-id" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_ServiceId    = { .Namespace = kConfigNamespace_ChipConfig, .Name = "service-id" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_FabricSecret = { .Namespace = kConfigNamespace_ChipConfig,
                                                                  .Name      = "fabric-secret" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_LastUsedEpochKeyId = { .Namespace = kConfigNamespace_ChipConfig,
                                                                        .Name      = "last-ek-id" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_FailSafeArmed      = { .Namespace = kConfigNamespace_ChipConfig,
                                                                        .Name      = "fail-safe-armed" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_GroupKey = { .Namespace = kConfigNamespace_ChipConfig, .Name = "group-key" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_HardwareVersion    = { .Namespace = kConfigNamespace_ChipConfig,
                                                                        .Name      = "hardware-ver" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_RegulatoryLocation = { .Namespace = kConfigNamespace_ChipConfig,
                                                                        .Name      = "reg-location" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_CountryCode = { .Namespace = kConfigNamespace_ChipConfig, .Name = "country-code" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_Breadcrumb  = { .Namespace = kConfigNamespace_ChipConfig, .Name = "breadcrumb" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_WiFiSSID    = { .Namespace = kConfigNamespace_ChipConfig, .Name = "wifi-ssid" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_WiFiPSK     = { .Namespace = kConfigNamespace_ChipConfig, .Name = "wifi-psk" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_WiFiSEC     = { .Namespace = kConfigNamespace_ChipConfig, .Name = "wifi-sec" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_GroupKeyBase = { .Namespace = kConfigNamespace_ChipConfig,
                                                                  .Name      = "group-key-base" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_GroupKeyMax  = { .Namespace = kConfigNamespace_ChipConfig,
                                                                  .Name      = "group-key-max" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_LockUser     = { .Namespace = kConfigNamespace_ChipConfig, .Name = "lock-user" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_Credential   = { .Namespace = kConfigNamespace_ChipConfig, .Name = "credential" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_LockUserName = { .Namespace = kConfigNamespace_ChipConfig,
                                                                  .Name      = "lock-user-name" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_CredentialData   = { .Namespace = kConfigNamespace_ChipConfig,
                                                                      .Name      = "credential-data" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_UserCredentials  = { .Namespace = kConfigNamespace_ChipConfig,
                                                                      .Name      = "user-credential" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_WeekDaySchedules = { .Namespace = kConfigNamespace_ChipConfig,
                                                                      .Name      = "weekday-sched" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_YearDaySchedules = { .Namespace = kConfigNamespace_ChipConfig,
                                                                      .Name      = "yearday-sched" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_HolidaySchedules = { .Namespace = kConfigNamespace_ChipConfig,
                                                                      .Name      = "holiday-sched" };
// CHIP Counter Keys
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_BootCount = { .Namespace = kConfigNamespace_ChipCounters, .Name = "boot-count" };
const SCM1612SConfig::Key SCM1612SConfig::kConfigKey_TotalOperationalHours = { .Namespace = kConfigNamespace_ChipCounters,
                                                                           .Name      = "total-hours" };

#ifdef __no_stub__
#define NVDM_SEM_TIMEOUT_MS 5

static SemaphoreHandle_t nvdm_sem;
static StaticSemaphore_t nvdm_sem_struct;
#endif /* __no_stub__ */

CHIP_ERROR SCM1612SConfig::Init()
{
#ifdef __no_stub__
    CHIP_ERROR err;
    nvdm_status_t nvdm_status;

    nvdm_sem = xSemaphoreCreateBinaryStatic(&nvdm_sem_struct);

    if (nvdm_sem == NULL)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    nvdm_status = nvdm_init();
    err         = MapNvdmStatus(nvdm_status);
    SuccessOrExit(err);

exit:
    OnExit();
#endif /* __no_stub__ */
    return CHIP_NO_ERROR;
}

CHIP_ERROR SCM1612SConfig::ReadConfigValue(Key key, bool & val)
{
#ifdef __no_stub__
    CHIP_ERROR err;
    uint32_t intVal;
    uint32_t len = sizeof(bool);

    if (pdFALSE == xSemaphoreTake(nvdm_sem, pdMS_TO_TICKS(NVDM_SEM_TIMEOUT_MS)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    // Get NVDM item
    err = MapNvdmStatus(nvdm_read_data_item(key.Namespace, key.Name, (uint8_t *) &intVal, &len));
    SuccessOrExit(err);

    val = (intVal != 0);

exit:
    OnExit();
    return err;
#else /* __no_stub__ */
    val = false;
    return CHIP_NO_ERROR;
#endif /* __no_stub__ */
}

CHIP_ERROR SCM1612SConfig::ReadConfigValue(Key key, uint32_t & val)
{
#ifdef __no_stub__
    CHIP_ERROR err;
    uint32_t len = sizeof(uint32_t);

    if (pdFALSE == xSemaphoreTake(nvdm_sem, pdMS_TO_TICKS(NVDM_SEM_TIMEOUT_MS)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    if (key.Namespace == SCM1612SConfig::kConfigKey_SetupDiscriminator.Namespace &&
        key.Name == SCM1612SConfig::kConfigKey_SetupDiscriminator.Name)
    {
        uint8_t mac_addr[WIFI_MAC_ADDRESS_LENGTH] = { 0 };
        auto mFilogicCtx                          = PlatformMgrImpl().mFilogicCtx;

        filogic_wifi_mac_addr_get_sync(mFilogicCtx, FILOGIC_WIFI_OPMODE_STA, mac_addr);

        val = (*(reinterpret_cast<uint32_t *>(mac_addr))) & 0xFFF;
        err = CHIP_NO_ERROR;
    }
    else
    {
        // Get NVDM item
        err = MapNvdmStatus(nvdm_read_data_item(key.Namespace, key.Name, (uint8_t *) &val, &len));
        SuccessOrExit(err);
    }
exit:
    OnExit();
    return err;
#else /* __no_stub__ */
    val = 0;
    return CHIP_NO_ERROR;
#endif /* __no_stub__ */
}

CHIP_ERROR SCM1612SConfig::ReadConfigValue(Key key, uint64_t & val)
{
#ifdef __no_stub__
    CHIP_ERROR err;
    uint32_t len = sizeof(uint64_t);

    if (pdFALSE == xSemaphoreTake(nvdm_sem, pdMS_TO_TICKS(NVDM_SEM_TIMEOUT_MS)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    // Get NVDM item
    err = MapNvdmStatus(nvdm_read_data_item(key.Namespace, key.Name, (uint8_t *) &val, &len));
    SuccessOrExit(err);

exit:
    OnExit();
    return err;
#else /* __no_stub__ */
    val = 0;
    return CHIP_NO_ERROR;
#endif /* __no_stub__ */
}

CHIP_ERROR SCM1612SConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
#ifdef __no_stub__
    CHIP_ERROR err;
    outLen = bufSize;
    if (pdFALSE == xSemaphoreTake(nvdm_sem, pdMS_TO_TICKS(NVDM_SEM_TIMEOUT_MS)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    // Get NVDM item
    err = MapNvdmStatus(nvdm_read_data_item(key.Namespace, key.Name, (uint8_t *) buf, (uint32_t *) &outLen));
    SuccessOrExit(err);

exit:
    OnExit();
    return err;
#else /* __no_stub__ */
    return CHIP_NO_ERROR;
#endif /* __no_stub__ */
}

CHIP_ERROR SCM1612SConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
#ifdef __no_stub__
    CHIP_ERROR err;
    outLen = bufSize;
    if (pdFALSE == xSemaphoreTake(nvdm_sem, pdMS_TO_TICKS(NVDM_SEM_TIMEOUT_MS)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    // Get NVDM item
    err = MapNvdmStatus(nvdm_read_data_item(key.Namespace, key.Name, (uint8_t *) buf, (uint32_t *) &outLen));
    SuccessOrExit(err);

exit:
    OnExit();
    return err;
#else /* __no_stub__ */
    return CHIP_NO_ERROR;
#endif /* __no_stub__ */
}

CHIP_ERROR SCM1612SConfig::ReadConfigValueCounter(uint8_t counterIdx, uint32_t & val)
{
    val = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR SCM1612SConfig::WriteConfigValue(Key key, bool val)
{
#ifdef __no_stub__
    CHIP_ERROR err;
    uint32_t intVal = val ? 1 : 0;
    uint32_t len    = sizeof(bool);

    if (pdFALSE == xSemaphoreTake(nvdm_sem, pdMS_TO_TICKS(NVDM_SEM_TIMEOUT_MS)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    // Set NVDM item
    err = MapNvdmStatus(nvdm_write_data_item(key.Namespace, key.Name, NVDM_DATA_ITEM_TYPE_RAW_DATA, (uint8_t *) &intVal, len));
    SuccessOrExit(err);
exit:
    OnExit();
    return err;
#else /* __no_stub__ */
    return CHIP_NO_ERROR;
#endif /* __no_stub__ */
}

CHIP_ERROR SCM1612SConfig::WriteConfigValue(Key key, uint32_t val)
{
#ifdef __no_stub__
    CHIP_ERROR err;
    uint32_t len = sizeof(uint32_t);

    if (pdFALSE == xSemaphoreTake(nvdm_sem, pdMS_TO_TICKS(NVDM_SEM_TIMEOUT_MS)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    // Set NVDM item
    err = MapNvdmStatus(nvdm_write_data_item(key.Namespace, key.Name, NVDM_DATA_ITEM_TYPE_RAW_DATA, (uint8_t *) &val, len));
    SuccessOrExit(err);
exit:
    OnExit();
    return err;
#else /* __no_stub__ */
    return CHIP_NO_ERROR;
#endif /* __no_stub__ */
}

CHIP_ERROR SCM1612SConfig::WriteConfigValue(Key key, uint64_t val)
{
#ifdef __no_stub__
    CHIP_ERROR err;
    uint32_t len = sizeof(uint64_t);

    if (pdFALSE == xSemaphoreTake(nvdm_sem, pdMS_TO_TICKS(NVDM_SEM_TIMEOUT_MS)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    // Set NVDM item
    err = MapNvdmStatus(nvdm_write_data_item(key.Namespace, key.Name, NVDM_DATA_ITEM_TYPE_RAW_DATA, (uint8_t *) &val, len));
    SuccessOrExit(err);
exit:
    OnExit();
    return err;
#else /* __no_stub__ */
    return CHIP_NO_ERROR;
#endif /* __no_stub__ */
}

CHIP_ERROR SCM1612SConfig::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueStr(key, str, (str != NULL) ? strlen(str) : 0);
}

CHIP_ERROR SCM1612SConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
#ifdef __no_stub__
    CHIP_ERROR err;

    if (pdFALSE == xSemaphoreTake(nvdm_sem, pdMS_TO_TICKS(NVDM_SEM_TIMEOUT_MS)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    // Set NVDM item
    err = MapNvdmStatus(nvdm_write_data_item(key.Namespace, key.Name, NVDM_DATA_ITEM_TYPE_STRING, (uint8_t *) str, strLen));
    SuccessOrExit(err);
exit:
    OnExit();
    return err;
#else /* __no_stub__ */
    return CHIP_NO_ERROR;
#endif /* __no_stub__ */
}

CHIP_ERROR SCM1612SConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
#ifdef __no_stub__
    CHIP_ERROR err;

    if (pdFALSE == xSemaphoreTake(nvdm_sem, pdMS_TO_TICKS(NVDM_SEM_TIMEOUT_MS)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    // Set NVDM item
    err = MapNvdmStatus(nvdm_write_data_item(key.Namespace, key.Name, NVDM_DATA_ITEM_TYPE_RAW_DATA, (uint8_t *) data, dataLen));
    SuccessOrExit(err);
exit:
    OnExit();
    return err;
#else /* __no_stub__ */
    return CHIP_NO_ERROR;
#endif /* __no_stub__ */
}

CHIP_ERROR SCM1612SConfig::WriteConfigValueCounter(uint8_t counterIdx, uint32_t val)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR SCM1612SConfig::ClearConfigValue(Key key)
{
#ifdef __no_stub__
    CHIP_ERROR err;

    if (pdFALSE == xSemaphoreTake(nvdm_sem, pdMS_TO_TICKS(NVDM_SEM_TIMEOUT_MS)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    // Delete NVDM item
    err = MapNvdmStatus(nvdm_delete_data_item(key.Namespace, key.Name));
    SuccessOrExit(err);
exit:
    OnExit();
    return err;
#else /* __no_stub__ */
    return CHIP_NO_ERROR;
#endif /* __no_stub__ */
}

bool SCM1612SConfig::ConfigValueExists(Key key)
{
#ifdef __no_stub__
    CHIP_ERROR err;
    char group_name[32];
    char data_item_name[32];
    bool ret = false;
    if (pdFALSE == xSemaphoreTake(nvdm_sem, pdMS_TO_TICKS(NVDM_SEM_TIMEOUT_MS)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    // Query NVDM item
    nvdm_query_begin();
    while (nvdm_query_next_group_name(group_name) == NVDM_STATUS_OK)
    {
        if (!strcmp(group_name, key.Namespace))
        {
            while (nvdm_query_next_data_item_name(data_item_name) == NVDM_STATUS_OK)
            {
                if (!strcmp(data_item_name, key.Name))
                {
                    ret = true;
                }
            }
        }
    }
    nvdm_query_end();

exit:
    OnExit();
    return ret;
#else /* __no_stub__ */
    return false;
#endif /* __no_stub__ */
}

CHIP_ERROR SCM1612SConfig::FactoryResetConfig(void)
{
#ifdef __no_stub__
    CHIP_ERROR err;

    if (pdFALSE == xSemaphoreTake(nvdm_sem, pdMS_TO_TICKS(NVDM_SEM_TIMEOUT_MS)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    // Deletes all 'Config' type objects
    // Note- 'Factory' and 'Counter' type are NOT deleted.
    err = MapNvdmStatus(nvdm_delete_group(kConfigNamespace_ChipConfig));
    SuccessOrExit(err);
exit:
    OnExit();
    return err;
#else /* __no_stub__ */
    return CHIP_NO_ERROR;
#endif /* __no_stub__ */
}

#ifdef __no_stub__
CHIP_ERROR SCM1612SConfig::MapNvdmStatus(nvdm_status_t nvdm_status)
{
    CHIP_ERROR err;

    //    NVDM_STATUS_INVALID_PARAMETER = -5,  /**< The user parameter is invalid. */
    //    NVDM_STATUS_ITEM_NOT_FOUND = -4,     /**< The data item wasn't found by the NVDM. */
    //    NVDM_STATUS_INSUFFICIENT_SPACE = -3, /**< No space is available in the flash. */
    //    NVDM_STATUS_INCORRECT_CHECKSUM = -2, /**< The NVDM found a checksum error when reading the data item. */
    //    NVDM_STATUS_ERROR = -1,              /**< An unknown error occurred. */
    //    NVDM_STATUS_OK = 0,                  /**< The operation was successful. */

    switch (nvdm_status)
    {
    case NVDM_STATUS_OK:
        err = CHIP_NO_ERROR;
        break;
    case NVDM_STATUS_ITEM_NOT_FOUND:
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
        break;
    case NVDM_STATUS_INCORRECT_CHECKSUM:
        err = CHIP_ERROR_INTEGRITY_CHECK_FAILED;
        break;
    case NVDM_STATUS_INSUFFICIENT_SPACE:
        err = CHIP_ERROR_BUFFER_TOO_SMALL;
        break;
    case NVDM_STATUS_INVALID_PARAMETER:
        err = CHIP_ERROR_INVALID_ARGUMENT;
        break;
    case NVDM_STATUS_ERROR:
        err = CHIP_ERROR_INTERNAL;
        break;
    default:
        err = CHIP_ERROR_INTERNAL;
        break;
    }

    return err;
}
#endif /* __no_stub__ */

void SCM1612SConfig::RunConfigUnitTest()
{
    // Run common unit test.
    ::chip::DeviceLayer::Internal::RunConfigUnitTest<SCM1612SConfig>();
}

void SCM1612SConfig::OnExit()
{
#ifdef __no_stub__
    xSemaphoreGive(nvdm_sem);
#endif /* __no_stub__ */
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

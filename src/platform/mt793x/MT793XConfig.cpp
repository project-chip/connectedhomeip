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
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/mt793x/MT793XConfig.h>

#include <lib/core/CHIPEncoding.h>
#include <platform/internal/testing/ConfigUnitTest.h>

#include "FreeRTOS.h"
#include "nvdm.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

// Factory config keys
const MT793XConfig::Key MT793XConfig::kConfigKey_SerialNum   = { .Namespace = kConfigNamespace_ChipFactory, .Name = "serial-num" };
const MT793XConfig::Key MT793XConfig::kConfigKey_UniqueId    = { .Namespace = kConfigNamespace_ChipFactory, .Name = "unique-id" };
const MT793XConfig::Key MT793XConfig::kConfigKey_MfrDeviceId = { .Namespace = kConfigNamespace_ChipFactory, .Name = "device-id" };
const MT793XConfig::Key MT793XConfig::kConfigKey_MfrDeviceCert       = { .Namespace = kConfigNamespace_ChipFactory,
                                                                   .Name      = "device-cert" };
const MT793XConfig::Key MT793XConfig::kConfigKey_MfrDevicePrivateKey = { .Namespace = kConfigNamespace_ChipFactory,
                                                                         .Name      = "device-key" };
const MT793XConfig::Key MT793XConfig::kConfigKey_ManufacturingDate   = { .Namespace = kConfigNamespace_ChipFactory,
                                                                       .Name      = "mfg-date" };
const MT793XConfig::Key MT793XConfig::kConfigKey_SetupPinCode = { .Namespace = kConfigNamespace_ChipFactory, .Name = "pin-code" };
const MT793XConfig::Key MT793XConfig::kConfigKey_MfrDeviceICACerts     = { .Namespace = kConfigNamespace_ChipFactory,
                                                                       .Name      = "device-ca-certs" };
const MT793XConfig::Key MT793XConfig::kConfigKey_SetupDiscriminator    = { .Namespace = kConfigNamespace_ChipFactory,
                                                                        .Name      = "discriminator" };
const MT793XConfig::Key MT793XConfig::kConfigKey_Spake2pIterationCount = { .Namespace = kConfigNamespace_ChipFactory,
                                                                           .Name      = "iteration-count" };
const MT793XConfig::Key MT793XConfig::kConfigKey_Spake2pSalt     = { .Namespace = kConfigNamespace_ChipFactory, .Name = "salt" };
const MT793XConfig::Key MT793XConfig::kConfigKey_Spake2pVerifier = { .Namespace = kConfigNamespace_ChipFactory,
                                                                     .Name      = "verifier" };
// CHIP Config Keys
const MT793XConfig::Key MT793XConfig::kConfigKey_FabricId      = { .Namespace = kConfigNamespace_ChipConfig, .Name = "fabric-id" };
const MT793XConfig::Key MT793XConfig::kConfigKey_ServiceConfig = { .Namespace = kConfigNamespace_ChipConfig,
                                                                   .Name      = "service-config" };
const MT793XConfig::Key MT793XConfig::kConfigKey_PairedAccountId = { .Namespace = kConfigNamespace_ChipConfig,
                                                                     .Name      = "account-id" };
const MT793XConfig::Key MT793XConfig::kConfigKey_ServiceId    = { .Namespace = kConfigNamespace_ChipConfig, .Name = "service-id" };
const MT793XConfig::Key MT793XConfig::kConfigKey_FabricSecret = { .Namespace = kConfigNamespace_ChipConfig,
                                                                  .Name      = "fabric-secret" };
const MT793XConfig::Key MT793XConfig::kConfigKey_LastUsedEpochKeyId = { .Namespace = kConfigNamespace_ChipConfig,
                                                                        .Name      = "last-ek-id" };
const MT793XConfig::Key MT793XConfig::kConfigKey_FailSafeArmed      = { .Namespace = kConfigNamespace_ChipConfig,
                                                                   .Name      = "fail-safe-armed" };
const MT793XConfig::Key MT793XConfig::kConfigKey_GroupKey = { .Namespace = kConfigNamespace_ChipConfig, .Name = "group-key" };
const MT793XConfig::Key MT793XConfig::kConfigKey_HardwareVersion    = { .Namespace = kConfigNamespace_ChipConfig,
                                                                     .Name      = "hardware-ver" };
const MT793XConfig::Key MT793XConfig::kConfigKey_RegulatoryLocation = { .Namespace = kConfigNamespace_ChipConfig,
                                                                        .Name      = "reg-location" };
const MT793XConfig::Key MT793XConfig::kConfigKey_CountryCode = { .Namespace = kConfigNamespace_ChipConfig, .Name = "country-code" };
const MT793XConfig::Key MT793XConfig::kConfigKey_Breadcrumb  = { .Namespace = kConfigNamespace_ChipConfig, .Name = "breadcrumb" };
const MT793XConfig::Key MT793XConfig::kConfigKey_WiFiSSID    = { .Namespace = kConfigNamespace_ChipConfig, .Name = "wifi-ssid" };
const MT793XConfig::Key MT793XConfig::kConfigKey_WiFiPSK     = { .Namespace = kConfigNamespace_ChipConfig, .Name = "wifi-psk" };
const MT793XConfig::Key MT793XConfig::kConfigKey_WiFiSEC     = { .Namespace = kConfigNamespace_ChipConfig, .Name = "wifi-sec" };
const MT793XConfig::Key MT793XConfig::kConfigKey_GroupKeyBase = { .Namespace = kConfigNamespace_ChipConfig,
                                                                  .Name      = "group-key-base" };
const MT793XConfig::Key MT793XConfig::kConfigKey_GroupKeyMax  = { .Namespace = kConfigNamespace_ChipConfig,
                                                                 .Name      = "group-key-max" };
// CHIP Counter Keys
const MT793XConfig::Key MT793XConfig::kConfigKey_BootCount = { .Namespace = kConfigNamespace_ChipCounters, .Name = "boot-count" };
const MT793XConfig::Key MT793XConfig::kConfigKey_TotalOperationalHours = { .Namespace = kConfigNamespace_ChipCounters,
                                                                           .Name      = "total-hours" };

#define NVDM_SEM_TIMEOUT_MS 5

static SemaphoreHandle_t nvdm_sem;
static StaticSemaphore_t nvdm_sem_struct;

CHIP_ERROR MT793XConfig::Init()
{
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
    return CHIP_NO_ERROR;
}

CHIP_ERROR MT793XConfig::ReadConfigValue(Key key, bool & val)
{
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
}

CHIP_ERROR MT793XConfig::ReadConfigValue(Key key, uint32_t & val)
{
    CHIP_ERROR err;
    uint32_t len = sizeof(uint32_t);

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
}

CHIP_ERROR MT793XConfig::ReadConfigValue(Key key, uint64_t & val)
{
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
}

CHIP_ERROR MT793XConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
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
}

CHIP_ERROR MT793XConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
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
}

CHIP_ERROR MT793XConfig::ReadConfigValueCounter(uint8_t counterIdx, uint32_t & val)
{
    val = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR MT793XConfig::WriteConfigValue(Key key, bool val)
{
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
}

CHIP_ERROR MT793XConfig::WriteConfigValue(Key key, uint32_t val)
{
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
}

CHIP_ERROR MT793XConfig::WriteConfigValue(Key key, uint64_t val)
{
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
}

CHIP_ERROR MT793XConfig::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueStr(key, str, (str != NULL) ? strlen(str) : 0);
}

CHIP_ERROR MT793XConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
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
}

CHIP_ERROR MT793XConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
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
}

CHIP_ERROR MT793XConfig::WriteConfigValueCounter(uint8_t counterIdx, uint32_t val)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR MT793XConfig::ClearConfigValue(Key key)
{
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
}

bool MT793XConfig::ConfigValueExists(Key key)
{
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
}

CHIP_ERROR MT793XConfig::FactoryResetConfig(void)
{
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
}

CHIP_ERROR MT793XConfig::MapNvdmStatus(nvdm_status_t nvdm_status)
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

void MT793XConfig::RunConfigUnitTest()
{
    // Run common unit test.
    ::chip::DeviceLayer::Internal::RunConfigUnitTest<MT793XConfig>();
}

void MT793XConfig::OnExit()
{
    xSemaphoreGive(nvdm_sem);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

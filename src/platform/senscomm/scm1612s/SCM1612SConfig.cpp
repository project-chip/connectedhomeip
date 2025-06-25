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

#include "wise_event_loop.h"
#include "wise_event.h"
#include "scm_fs.h"
#include "scm_wifi.h"
#include "wise_wifi_types.h"

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

#define CONFIG_DIR "/config"
#define CONFIG_PATH_MAX 128


CHIP_ERROR SCM1612SConfig::Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR SCM1612SConfig::ReadConfigValue(Key key, bool & val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t value = 0;

    int read_len = scm_fs_read_config_value(key.Namespace, key.Name, (char *)&value, sizeof(value));
    if (read_len < 0) {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    } else if (read_len != sizeof(value)) {
        return CHIP_ERROR_READ_FAILED;
    }

    val = (value != 0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR SCM1612SConfig::ReadConfigValue(Key key, uint32_t & val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint32_t value = 0;

    int read_len = scm_fs_read_config_value(key.Namespace, key.Name, (char *)&value, sizeof(value));
    if (read_len < 0)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    else if (read_len != sizeof(value))
    {
        err = CHIP_ERROR_READ_FAILED;
    }
    else
    {
        val = value;
    }

    return err;
}

CHIP_ERROR SCM1612SConfig::ReadConfigValue(Key key, uint64_t & val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint64_t value = 0;

    int read_len = scm_fs_read_config_value(key.Namespace, key.Name, (char *)&value, sizeof(value));
    if (read_len < 0)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    else if (read_len != sizeof(value))
    {
        err = CHIP_ERROR_READ_FAILED;
    }
    else
    {
        val = value;
    }

    return err;
}

CHIP_ERROR SCM1612SConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (buf == nullptr || bufSize == 0)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    int read_len = scm_fs_read_config_value(key.Namespace, key.Name, buf, bufSize);
    if (read_len < 0)
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    outLen = static_cast<size_t>(read_len);

    return CHIP_NO_ERROR;
}

CHIP_ERROR SCM1612SConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    if (buf == nullptr || bufSize == 0)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    int read_len = scm_fs_read_config_value(key.Namespace, key.Name, reinterpret_cast<char *>(buf), bufSize);
    if (read_len < 0)
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    outLen = static_cast<size_t>(read_len);

    return CHIP_NO_ERROR;
}

CHIP_ERROR SCM1612SConfig::ReadConfigValueCounter(uint8_t counterIdx, uint32_t & val)
{
    val = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR SCM1612SConfig::WriteConfigValue(Key key, bool val)
{
    uint8_t value = val ? 1 : 0;

    int written = scm_fs_write_config_value(key.Namespace, key.Name, (const char *)&value, sizeof(value));
    if (written != sizeof(value))
        return CHIP_ERROR_WRITE_FAILED;

    return CHIP_NO_ERROR;
}

CHIP_ERROR SCM1612SConfig::WriteConfigValue(Key key, uint32_t val)
{
    int written = scm_fs_write_config_value(key.Namespace, key.Name, (const char *)&val, sizeof(val));
    if (written != sizeof(val))
        return CHIP_ERROR_WRITE_FAILED;

    return CHIP_NO_ERROR;
}

CHIP_ERROR SCM1612SConfig::WriteConfigValue(Key key, uint64_t val)
{
    int written = scm_fs_write_config_value(key.Namespace, key.Name, (const char *)&val, sizeof(val));
    if (written != sizeof(val))
        return CHIP_ERROR_WRITE_FAILED;

    return CHIP_NO_ERROR;
}

CHIP_ERROR SCM1612SConfig::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueStr(key, str, (str != NULL) ? strlen(str) : 0);
}

CHIP_ERROR SCM1612SConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    if (str == nullptr || strLen == 0)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    int written = scm_fs_write_config_value(key.Namespace, key.Name, str, static_cast<int>(strLen));
    if (written != static_cast<int>(strLen))
    {
        return CHIP_ERROR_WRITE_FAILED;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR SCM1612SConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    if (data == nullptr || dataLen == 0)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    int written = scm_fs_write_config_value(key.Namespace, key.Name, reinterpret_cast<const char *>(data), static_cast<int>(dataLen));
    if (written != static_cast<int>(dataLen))
    {
        return CHIP_ERROR_WRITE_FAILED;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR SCM1612SConfig::WriteConfigValueCounter(uint8_t counterIdx, uint32_t val)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR SCM1612SConfig::ClearConfigValue(Key key)
{
    if (strcmp(key.Namespace, "chip-factory") == 0)
    {
        const char * configPartitionPath = "/config";
        if (scm_fs_unmount(configPartitionPath) < 0)
        {
            return CHIP_ERROR_INTERNAL;
        }
        if (scm_fs_format(configPartitionPath) < 0)
        {
            return CHIP_ERROR_INTERNAL;
        }

        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

bool SCM1612SConfig::ConfigValueExists(Key key)
{
    return (scm_fs_exists_config_value(key.Namespace, key.Name) == 0);
}

CHIP_ERROR SCM1612SConfig::FactoryResetConfig(void)
{
    const char * configPartitionPath = "/config";

    if (scm_fs_unmount(configPartitionPath) < 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    if (scm_fs_format(configPartitionPath) < 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

void SCM1612SConfig::RunConfigUnitTest()
{
    // Run common unit test.
    ::chip::DeviceLayer::Internal::RunConfigUnitTest<SCM1612SConfig>();
}

void SCM1612SConfig::OnExit()
{
    return;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

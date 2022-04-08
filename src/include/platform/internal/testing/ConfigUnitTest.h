/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#include <lib/core/CHIPCore.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ConfigClass>
void RunConfigUnitTest()
{
    CHIP_ERROR err;

    // ===== Test 1: Store and read uint32_t
    {
        uint32_t v = 42;

        err = ConfigClass::WriteConfigValue(ConfigClass::kConfigKey_LastUsedEpochKeyId, v);
        VerifyOrDie(err == CHIP_NO_ERROR);

        v = 0;

        err = ConfigClass::ReadConfigValue(ConfigClass::kConfigKey_LastUsedEpochKeyId, v);
        VerifyOrDie(err == CHIP_NO_ERROR);

        VerifyOrDie(v == 42);
    }

    // ===== Test 2: Store and read uint64_t
    {
        uint64_t v = 9872349687345;

        err = ConfigClass::WriteConfigValue(ConfigClass::kConfigKey_MfrDeviceId, v);
        VerifyOrDie(err == CHIP_NO_ERROR);

        v = 0;

        err = ConfigClass::ReadConfigValue(ConfigClass::kConfigKey_MfrDeviceId, v);
        VerifyOrDie(err == CHIP_NO_ERROR);

        VerifyOrDie(v == 9872349687345);
    }

    // ===== Test 3: Store and read bool value
    {
        bool v = true;

        err = ConfigClass::WriteConfigValue(ConfigClass::kConfigKey_FailSafeArmed, v);
        VerifyOrDie(err == CHIP_NO_ERROR);

        v = false;

        err = ConfigClass::ReadConfigValue(ConfigClass::kConfigKey_FailSafeArmed, v);
        VerifyOrDie(err == CHIP_NO_ERROR);

        VerifyOrDie(v == true);
    }

    // ===== Test 4: Clear value
    {
        uint32_t v;

        err = ConfigClass::ClearConfigValue(ConfigClass::kConfigKey_LastUsedEpochKeyId);
        VerifyOrDie(err == CHIP_NO_ERROR);

        err = ConfigClass::ReadConfigValue(ConfigClass::kConfigKey_LastUsedEpochKeyId, v);
        VerifyOrDie(err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);
    }

    // ===== Test 5: Store and read string
    {
        const static char kTestString1[] = "This is a test";
        const static char kTestString2[] = "";
        char buf[64];
        size_t strLen;

        err = ConfigClass::WriteConfigValueStr(ConfigClass::kConfigKey_PairedAccountId, kTestString1);
        VerifyOrDie(err == CHIP_NO_ERROR);

        err = ConfigClass::ReadConfigValueStr(ConfigClass::kConfigKey_PairedAccountId, buf, sizeof(buf), strLen);
        VerifyOrDie(err == CHIP_NO_ERROR);

        VerifyOrDie(strLen == strlen(kTestString1));
        VerifyOrDie(memcmp(buf, kTestString1, strLen + 1) == 0);

        err = ConfigClass::WriteConfigValueStr(ConfigClass::kConfigKey_PairedAccountId, kTestString2);
        VerifyOrDie(err == CHIP_NO_ERROR);

        err = ConfigClass::ReadConfigValueStr(ConfigClass::kConfigKey_PairedAccountId, buf, sizeof(buf), strLen);
        VerifyOrDie(err == CHIP_NO_ERROR);

        VerifyOrDie(strLen == strlen(kTestString2));
        VerifyOrDie(memcmp(buf, kTestString2, strLen + 1) == 0);
    }

    // ===== Test 6: Clear string
    {
        char buf[64];
        size_t strLen;

        err = ConfigClass::WriteConfigValueStr(ConfigClass::kConfigKey_PairedAccountId, nullptr);
        VerifyOrDie(err == CHIP_NO_ERROR);

        err = ConfigClass::ReadConfigValueStr(ConfigClass::kConfigKey_PairedAccountId, buf, sizeof(buf), strLen);
        VerifyOrDie(err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);
    }

    // ===== Test 7: Store and read binary data
    {
        const static uint8_t kTestData[] = {
            0xD5, 0x00, 0x00, 0x04, 0x00, 0x01, 0x00, 0x30, 0x01, 0x08, 0x79, 0x55, 0x9F, 0x15, 0x1F, 0x66, 0x3D, 0x8F, 0x24,
            0x02, 0x05, 0x37, 0x03, 0x27, 0x13, 0x02, 0x00, 0x00, 0xEE, 0xEE, 0x30, 0xB4, 0x18, 0x18, 0x26, 0x04, 0x80, 0x41,
            0x1B, 0x23, 0x26, 0x05, 0x7F, 0xFF, 0xFF, 0x52, 0x37, 0x06, 0x27, 0x11, 0x01, 0x00, 0x00, 0x00, 0x00, 0x30, 0xB4,
            0x18, 0x18, 0x24, 0x07, 0x02, 0x26, 0x08, 0x25, 0x00, 0x5A, 0x23, 0x30, 0x0A, 0x39, 0x04, 0x9E, 0xC7, 0x77, 0xC5,
            0xA4, 0x13, 0x31, 0xF7, 0x72, 0x2E, 0x27, 0xC2, 0x86, 0x3D, 0xC5, 0x2E, 0xD5, 0xD2, 0x3C, 0xCF, 0x7E, 0x06, 0xE3,
            0x48, 0x53, 0x87, 0xE8, 0x4D, 0xB0, 0x27, 0x07, 0x58, 0x4A, 0x38, 0xB4, 0xF3, 0xB2, 0x47, 0x94, 0x45, 0x58, 0x65,
            0x80, 0x08, 0x17, 0x6B, 0x8E, 0x4F, 0x07, 0x41, 0xA3, 0x3D, 0x5D, 0xCE, 0x76, 0x86, 0x35, 0x83, 0x29, 0x01, 0x18,
            0x35, 0x82, 0x29, 0x01, 0x24, 0x02, 0x05, 0x18, 0x35, 0x84, 0x29, 0x01, 0x36, 0x02, 0x04, 0x02, 0x04, 0x01, 0x18,
            0x18, 0x35, 0x81, 0x30, 0x02, 0x08, 0x42, 0xBD, 0x2C, 0x6B, 0x5B, 0x3A, 0x18, 0x16, 0x18, 0x35, 0x80, 0x30, 0x02,
            0x08, 0x44, 0xE3, 0x40, 0x38, 0xA9, 0xD4, 0xB5, 0xA7, 0x18, 0x35, 0x0C, 0x30, 0x01, 0x19, 0x00, 0xA6, 0x5D, 0x54,
            0xF5, 0xAE, 0x5D, 0x63, 0xEB, 0x69, 0xD8, 0xDB, 0xCB, 0xE2, 0x20, 0x0C, 0xD5, 0x6F, 0x43, 0x5E, 0x96, 0xA8, 0x54,
            0xB2, 0x74, 0x30, 0x02, 0x19, 0x00, 0xE0, 0x37, 0x02, 0x8B, 0xB3, 0x04, 0x06, 0xDD, 0xBD, 0x28, 0xAA, 0xC4, 0xF1,
            0xFF, 0xFB, 0xB1, 0xD4, 0x1C, 0x78, 0x40, 0xDA, 0x2C, 0xD8, 0x40, 0x18, 0x18,
        };
        uint8_t buf[512];
        size_t dataLen;

        err = ConfigClass::WriteConfigValueBin(ConfigClass::kConfigKey_MfrDeviceCert, kTestData, sizeof(kTestData));
        VerifyOrDie(err == CHIP_NO_ERROR);

        err = ConfigClass::ReadConfigValueBin(ConfigClass::kConfigKey_MfrDeviceCert, buf, sizeof(buf), dataLen);
        VerifyOrDie(err == CHIP_NO_ERROR);

        VerifyOrDie(dataLen == sizeof(kTestData));
        VerifyOrDie(memcmp(buf, kTestData, dataLen) == 0);
    }

    // ===== Test 8: Clear binary data
    {
        uint8_t buf[512];
        size_t dataLen;

        err = ConfigClass::WriteConfigValueBin(ConfigClass::kConfigKey_MfrDeviceCert, nullptr, 0);
        VerifyOrDie(err == CHIP_NO_ERROR);

        err = ConfigClass::ReadConfigValueBin(ConfigClass::kConfigKey_MfrDeviceCert, buf, sizeof(buf), dataLen);
        VerifyOrDie(err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);
    }

    // ===== Test 9: Config value exists
    {
        bool v;

        v = ConfigClass::ConfigValueExists(ConfigClass::kConfigKey_MfrDeviceId);
        VerifyOrDie(v == true);

        v = ConfigClass::ConfigValueExists(ConfigClass::kConfigKey_FailSafeArmed);
        VerifyOrDie(v == true);

        v = ConfigClass::ConfigValueExists(ConfigClass::kConfigKey_MfrDeviceCert);
        VerifyOrDie(v == false);
    }

    // ===== Test 10: Factory reset config
    {
        bool v;

        err = ConfigClass::FactoryResetConfig();
        VerifyOrDie(err == CHIP_NO_ERROR);

        v = ConfigClass::ConfigValueExists(ConfigClass::kConfigKey_MfrDeviceId);
        VerifyOrDie(v == true);

        v = ConfigClass::ConfigValueExists(ConfigClass::kConfigKey_FailSafeArmed);
        VerifyOrDie(v == false);
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

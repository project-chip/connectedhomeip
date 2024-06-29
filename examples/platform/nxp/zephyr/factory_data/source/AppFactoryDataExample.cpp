/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "AppFactoryData.h"

#include <credentials/DeviceAttestationCredsProvider.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>

#if CONFIG_CHIP_PLAT_LOAD_REAL_FACTORY_DATA
#include "FactoryDataProvider.h"
/*
 * Test key used to encrypt factory data before storing it to the flash.
 * The software key should be used only during development stage.
 * For production usage, it is recommended to use the OTP key which needs to be fused in the RT1060 SW_GP2.
 */
static const uint8_t aes128TestKey[]
    __attribute__((aligned)) = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
#else
#include <credentials/examples/DeviceAttestationCredsExample.h>
#endif

#if CONFIG_CHIP_FACTORY_DATA
#include <platform/nxp/common/factory_data/FactoryDataProvider.h>
#else
#include <platform/nxp/zephyr/DeviceInstanceInfoProviderImpl.h>
#endif

#if CONFIG_CHIP_FACTORY_DATA && CONFIG_CHIP_ENCRYPTED_FACTORY_DATA
#ifdef CONFIG_CHIP_ENCRYPTED_FACTORY_DATA_AES128_KEY

#define KEY CONFIG_CHIP_ENCRYPTED_FACTORY_DATA_AES128_KEY
#define HEXTONIBBLE(c) (*(c) >= 'A' ? (*(c) - 'A') + 10 : (*(c) - '0'))
#define HEXTOBYTE(c) (HEXTONIBBLE(c) * 16 + HEXTONIBBLE(c + 1))
#define AES128_KEY_ARRAY                                                                                                           \
    HEXTOBYTE(KEY + 0), HEXTOBYTE(KEY + 2), HEXTOBYTE(KEY + 4), HEXTOBYTE(KEY + 6), HEXTOBYTE(KEY + 8), HEXTOBYTE(KEY + 10),       \
        HEXTOBYTE(KEY + 12), HEXTOBYTE(KEY + 14), HEXTOBYTE(KEY + 16), HEXTOBYTE(KEY + 18), HEXTOBYTE(KEY + 20),                   \
        HEXTOBYTE(KEY + 22), HEXTOBYTE(KEY + 24), HEXTOBYTE(KEY + 26), HEXTOBYTE(KEY + 28), HEXTOBYTE(KEY + 30)
#else
#define AES128_KEY_ARRAY 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
#endif /* CONFIG_CHIP_ENCRYPTED_FACTORY_DATA_AES128_KEY */

/*
 * Test key used to encrypt factory data before storing it to the flash.
 */
static const uint8_t aes128TestKey[] __attribute__((aligned)) = { AES128_KEY_ARRAY };

#endif /* CONFIG_CHIP_FACTORY_DATA && CONFIG_CHIP_ENCRYPTED_FACTORY_DATA */

using namespace chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

/**
 * Allows to register Matter factory data before initializing the Matter stack
 * Load factory data from the flash to the RAM.
 * Needs to be done before starting other Matter modules to avoid concurrent access issues with DCP hardware module.
 *
 * This example demonstrates the usage of the ecb with a software key, to use other encryption mode,
 * or to use hardware keys, check available methodes from the FactoryDataProviderImpl class.
 */
CHIP_ERROR NXP::App::AppFactoryData_PreMatterStackInit(void)
{
    return CHIP_NO_ERROR;
}

/**
 * Allows to register Matter factory data  after initializing the Matter stack
 */
CHIP_ERROR NXP::App::AppFactoryData_PostMatterStackInit(void)
{
#if CONFIG_CHIP_FACTORY_DATA
#if CONFIG_CHIP_ENCRYPTED_FACTORY_DATA
    FactoryDataPrvdImpl().SetEncryptionMode(FactoryDataProvider::encrypt_ecb);
    FactoryDataPrvdImpl().SetAes128Key(&aes128TestKey[0]);
#endif /* CONFIG_CHIP_ENCRYPTED_FACTORY_DATA */
    ReturnErrorOnFailure(FactoryDataPrvdImpl().Init());
    SetDeviceInstanceInfoProvider(&FactoryDataPrvd());
    SetDeviceAttestationCredentialsProvider(&FactoryDataPrvd());
    SetCommissionableDataProvider(&FactoryDataPrvd());
#else
    SetDeviceInstanceInfoProvider(&DeviceInstanceInfoProviderMgrImpl());
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif /* CONFIG_CHIP_FACTORY_DATA */
    return CHIP_NO_ERROR;
}

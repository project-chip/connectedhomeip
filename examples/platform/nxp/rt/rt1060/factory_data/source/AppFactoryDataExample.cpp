/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#if CONFIG_CHIP_PLAT_LOAD_REAL_FACTORY_DATA

    FactoryDataPrvdImpl().SetEncryptionMode(FactoryDataProvider::encrypt_ecb);
    FactoryDataPrvdImpl().SetAes128Key(&aes128TestKey[0]);

    return FactoryDataPrvdImpl().Init();
#else
    return CHIP_NO_ERROR;
#endif
}

/**
 * Allows to register Matter factory data  after initializing the Matter stack
 */
CHIP_ERROR NXP::App::AppFactoryData_PostMatterStackInit(void)
{
#if CONFIG_CHIP_PLAT_LOAD_REAL_FACTORY_DATA
    SetDeviceInstanceInfoProvider(&FactoryDataPrvd());
    SetDeviceAttestationCredentialsProvider(&FactoryDataPrvd());
    SetCommissionableDataProvider(&FactoryDataPrvd());
#else
    // Initialize device attestation with example one (only for debug purpose)
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif
    return CHIP_NO_ERROR;
}

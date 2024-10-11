/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    Copyright 2023-2024 NXP
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
#ifdef ENABLE_SECURE_WHOLE_FACTORY_DATA
static uint8_t aes256TestKey[]
    __attribute__((aligned)) = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
                                 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
#endif
#else
#include <credentials/examples/DeviceAttestationCredsExample.h>
#endif

using namespace chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

/**
 * Allows to register Matter factory data before initializing the Matter stack
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
#if CONFIG_CHIP_PLAT_LOAD_REAL_FACTORY_DATA
#ifdef ENABLE_SECURE_WHOLE_FACTORY_DATA
    /* Please Note, because currently we only support AES-256 key provisioning and de-wrap, so the trasfterred AES key should be 256
     * bit size*/
    FactoryDataPrvdImpl().SetEncryptionMode(FactoryDataProvider::encrypt_ecb);
    FactoryDataPrvdImpl().SetAes256Key(&aes256TestKey[0]);
#endif
    FactoryDataPrvdImpl().Init();
    SetDeviceInstanceInfoProvider(&FactoryDataPrvd());
    SetDeviceAttestationCredentialsProvider(&FactoryDataPrvd());
    SetCommissionableDataProvider(&FactoryDataPrvd());
#else
    // Initialize device attestation with example one (only for debug purpose)
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif
    return CHIP_NO_ERROR;
}

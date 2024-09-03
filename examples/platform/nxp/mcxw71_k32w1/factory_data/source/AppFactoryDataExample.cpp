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
#include "FactoryDataDriver.h"
#include "FactoryDataProvider.h"
#else
#include <credentials/examples/DeviceAttestationCredsExample.h>
#endif

using namespace chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

#if CONFIG_CHIP_LOAD_REAL_FACTORY_DATA && CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR
/**
 * Custom factory data restore mechanism. This function must be implemented by vendors.
 */
CHIP_ERROR CustomFactoryDataRestoreMechanism(void)
{
    ChipLogProgress(DeviceLayer, "This is a custom factory data restore mechanism.");

    return CHIP_NO_ERROR;
}
#endif

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
#if CONFIG_CHIP_LOAD_REAL_FACTORY_DATA
    ReturnErrorOnFailure(FactoryDataDrv().Init());
#if CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR
    FactoryDataPrvd().RegisterRestoreMechanism(CustomFactoryDataRestoreMechanism);
#endif
    ReturnErrorOnFailure(FactoryDataPrvd().Init());
    SetDeviceInstanceInfoProvider(&FactoryDataPrvd());
    SetDeviceAttestationCredentialsProvider(&FactoryDataPrvd());
    SetCommissionableDataProvider(&FactoryDataPrvd());
#else
    // Initialize device attestation with example one (only for debug purpose)
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif
    return CHIP_NO_ERROR;
}

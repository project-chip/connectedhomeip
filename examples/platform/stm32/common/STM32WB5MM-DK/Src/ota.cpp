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

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#if (OTA_SUPPORT == 1)
#include "ota.h"
#include <platform/CHIPDeviceLayer.h>

#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorUserConsent.h>

using namespace chip;
using namespace chip::DeviceLayer;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

DefaultOTARequestor gRequestorCore;
DefaultOTARequestorStorage gRequestorStorage;
DefaultOTARequestorDriver gRequestorUser;
BDXDownloader gDownloader;
OTAImageProcessorImpl gImageProcessor;
FactoryDataProvider gFactoryDataProvider;
chip::ota::DefaultOTARequestorUserConsent gUserConsentProvider;
static chip::ota::UserConsentState gUserConsentState = chip::ota::UserConsentState::kGranted;

/*****************************************************************************
 *                    Application Function Definitions
 *****************************************************************************/

bool OtaHeaderValidation(Ota_ImageHeader_t imageHeader)
{

    uint16_t vendorId  = 0;
    uint16_t productId = 0;

    if (gFactoryDataProvider.GetVendorId(vendorId) != CHIP_NO_ERROR)
    {
        return false;
    }
    if (gFactoryDataProvider.GetProductId(productId) != CHIP_NO_ERROR)
    {
        return false;
    }

    // Check that the image matches vendor and product ID and that the version is higher than what we currently have
    if (imageHeader.vendorId != vendorId || imageHeader.productId != productId ||
        imageHeader.softwareVersion <= CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION)
    {
        return false;
    }

    return true;
}

void InitializeOTARequestor(void)
{
    ChipLogProgress(DeviceLayer, "Initialising OTA Requestor");
    // Initialize and interconnect the Requestor and Image Processor objects
    SetRequestorInstance(&gRequestorCore);

    gRequestorStorage.Init(chip::Server::GetInstance().GetPersistentStorage());
    gRequestorCore.Init(chip::Server::GetInstance(), gRequestorStorage, gRequestorUser, gDownloader);
    gImageProcessor.SetOTADownloader(&gDownloader);
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);
    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);
    gUserConsentProvider.SetUserConsentState(gUserConsentState);
    // Test to trigger ota. this function can be trigger by a Push Button
    TriggerOTAQuery();
}

void TriggerOTAQuery(void)
{
    CHIP_ERROR err                    = CHIP_NO_ERROR;
    OTARequestorInterface * requestor = GetRequestorInstance();

    if (requestor != nullptr)
    {
        err = requestor->TriggerImmediateQuery(kUndefinedFabricIndex);

        if (CHIP_NO_ERROR != err)
        {
            ChipLogError(DeviceLayer, "Failed trigger OTA query: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
    else
    {
        ChipLogProgress(DeviceLayer, "No OTA requestor instance, can't query OTA");
    }
}
#endif

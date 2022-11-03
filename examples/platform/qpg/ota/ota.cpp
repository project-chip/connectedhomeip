/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/** @file "ota.cpp"
 *
 * OTA handling for an application
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include <platform/CHIPDeviceLayer.h>

#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <platform/qpg/OTAImageProcessorImpl.h>

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

/*****************************************************************************
 *                    Application Function Definitions
 *****************************************************************************/

bool OtaHeaderValidationCb(qvCHIP_Ota_ImageHeader_t imageHeader)
{
    // Check that the image matches vendor and product ID and that the version is higher than what we currently have
    if (imageHeader.vendorId != CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID ||
        imageHeader.productId != CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID ||
        imageHeader.softwareVersion <= CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION)
    {
        return false;
    }

    return true;
}

void InitializeOTARequestor(void)
{
    ChipLogDetail(DeviceLayer, "Initialising OTA Requestor");
    // Initialize and interconnect the Requestor and Image Processor objects
    SetRequestorInstance(&gRequestorCore);

    gRequestorStorage.Init(chip::Server::GetInstance().GetPersistentStorage());
    gRequestorCore.Init(chip::Server::GetInstance(), gRequestorStorage, gRequestorUser, gDownloader);
    gImageProcessor.SetOTADownloader(&gDownloader);
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);
    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);

    // Initialize OTA image validation callback
    qvCHIP_OtaSetHeaderValidationCb(OtaHeaderValidationCb);
}

void TriggerOTAQuery(void)
{
    GetRequestorInstance()->TriggerImmediateQuery();
}

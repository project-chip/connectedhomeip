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

/**
 *    @file
 *      This file provides the Device Firmware Update manager class implementation.
 *      It provides firmware update functionality based on Matter OTA Requestor cluster.
 */

#include "openiotsdk_dfu_manager.h"

#include "psa/fwu_config.h"

DFUManager DFUManager::sDFUMgr;

UserConsentState CustomOTARequestorUserConsent::GetUserConsentState(const UserConsentSubject & subject)
{
    UserConsentState curUserConsentState = CheckDeferredUserConsentState();

    ChipLogProgress(SoftwareUpdate, "New version of the software is available: %d", subject.requestorTargetVersion);
    SetUserConsentState(chip::ota::UserConsentState::kGranted);

    return curUserConsentState;
}

CHIP_ERROR DFUManager::Init()
{
    int ret;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Set the global instance of the OTA requestor core component
    SetRequestorInstance(&mRequestorCore);

    // Periodic query timeout must be set prior to the driver being initialized
    mRequestorDriver.SetPeriodicQueryTimeout(0);

    // Watchdog timeout can be set any time before a query image is sent
    mRequestorDriver.SetWatchdogTimeout(0);

    mRequestorStorage.Init(chip::Server::GetInstance().GetPersistentStorage());
    mRequestorCore.Init(chip::Server::GetInstance(), mRequestorStorage, mRequestorDriver, mDownloader);
    mRequestorDriver.Init(&mRequestorCore, &mImageProcessor);

    mImageProcessor.SetOTADownloader(&mDownloader);
    mImageProcessor.SetImageId(FWU_COMPONENT_ID_NONSECURE);

    // Set the image processor instance used for handling image being downloaded
    mDownloader.SetImageProcessorDelegate(&mImageProcessor);

    mRequestorDriver.SetUserConsentDelegate(&mUserConsentProvider);

    return err;
}

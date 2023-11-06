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

#pragma once

#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorUserConsent.h>
#include <app/clusters/ota-requestor/ExtendedOTARequestorDriver.h>
#include <platform/openiotsdk/OTAImageProcessorImpl.h>

using namespace ::chip;
using namespace ::chip::ota;
using namespace ::chip::DeviceLayer;

class CustomOTARequestorUserConsent : public DefaultOTARequestorUserConsent
{
public:
    UserConsentState GetUserConsentState(const UserConsentSubject & subject) override;
};

class DFUManager
{
public:
    CHIP_ERROR Init();

private:
    friend DFUManager & GetDFUManager(void);

    static DFUManager sDFUMgr;

    DefaultOTARequestor mRequestorCore;
    DefaultOTARequestorStorage mRequestorStorage;
    ExtendedOTARequestorDriver mRequestorDriver;
    BDXDownloader mDownloader;
    OTAImageProcessorImpl mImageProcessor;
    CustomOTARequestorUserConsent mUserConsentProvider;
};

inline DFUManager & GetDFUManager(void)
{
    return DFUManager::sDFUMgr;
}

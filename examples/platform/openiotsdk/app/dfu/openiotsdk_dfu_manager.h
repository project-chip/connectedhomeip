/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

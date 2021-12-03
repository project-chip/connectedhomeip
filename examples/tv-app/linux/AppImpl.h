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

/**
 * @brief Manages Content Apps
 */

#pragma once

#include <app-common/zap-generated/enums.h>
#include <app/util/ContentApp.h>
#include <app/util/ContentAppPlatform.h>
#include <app/util/attribute-storage.h>
#include <functional>
#include <stdbool.h>
#include <stdint.h>

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

namespace chip {
namespace AppPlatform {

class DLL_EXPORT ApplicationBasicImpl : public ApplicationBasic
{
public:
    virtual ~ApplicationBasicImpl() {}

    inline const char * GetVendorName() override { return mVendorName; };
    inline uint16_t GetVendorId() override { return mVendorId; };
    inline const char * GetApplicationName() override { return mApplicationName; };
    inline uint16_t GetProductId() override { return mProductId; };
    inline app::Clusters::ApplicationBasic::ApplicationBasicStatus GetApplicationStatus() override { return mApplicationStatus; };
    inline const char * GetApplicationVersion() override { return mApplicationVersion; };

    inline void SetApplicationStatus(app::Clusters::ApplicationBasic::ApplicationBasicStatus applicationStatus) override
    {
        mApplicationStatus = applicationStatus;
    };

    void SetVendorName(const char * szVendorName);
    inline void SetVendorId(uint16_t vendorId) { mVendorId = vendorId; };
    void SetApplicationName(const char * szApplicationName);
    inline void SetProductId(uint16_t productId) { mProductId = productId; };
    void SetApplicationVersion(const char * szApplicationVersion);

protected:
    static const int kVendorNameSize         = 32;
    static const int kApplicationNameSize    = 32;
    static const int kApplicationVersionSize = 32;

    char mVendorName[kVendorNameSize];
    uint16_t mVendorId;
    char mApplicationName[kApplicationNameSize];
    uint16_t mProductId;
    app::Clusters::ApplicationBasic::ApplicationBasicStatus mApplicationStatus =
        app::Clusters::ApplicationBasic::ApplicationBasicStatus::kStopped;
    char mApplicationVersion[kApplicationVersionSize];
};

class DLL_EXPORT AccountLoginImpl : public AccountLogin
{
public:
    virtual ~AccountLoginImpl() {}

    inline void SetSetupPIN(uint32_t setupPIN) override { mSetupPIN = setupPIN; };
    uint32_t GetSetupPIN(const char * tempAccountId) override;
    bool Login(const char * tempAccountId, uint32_t setupPin) override;

protected:
    uint32_t mSetupPIN = 0;
};

class DLL_EXPORT ContentAppImpl : public ContentApp
{
public:
    ContentAppImpl(const char * szVendorName, uint16_t vendorId, const char * szApplicationName, uint16_t productId,
                   const char * szApplicationVersion);
    virtual ~ContentAppImpl() {}

    inline ApplicationBasic * GetApplicationBasic() override { return &mApplicationBasic; };
    inline AccountLogin * GetAccountLogin() override { return &mAccountLogin; };

protected:
    ApplicationBasicImpl mApplicationBasic;
    AccountLoginImpl mAccountLogin;
};

class DLL_EXPORT ContentAppFactoryImpl : public ContentAppFactory
{
public:
    ContentAppFactoryImpl();
    virtual ~ContentAppFactoryImpl() {}

    ContentApp * LoadContentAppByVendorId(uint16_t vendorId);

protected:
    ContentAppImpl mContentApps[3] = { ContentAppImpl("Vendor1", 1, "App1", 11, "Version1"),
                                       ContentAppImpl("Vendor2", 2, "App2", 22, "Version2"),
                                       ContentAppImpl("Vendor3", 9050, "App3", 22, "Version3") };
};

} // namespace AppPlatform
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

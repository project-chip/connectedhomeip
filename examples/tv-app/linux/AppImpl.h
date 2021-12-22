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
    inline app::Clusters::ApplicationBasic::ApplicationStatusEnum GetApplicationStatus() override { return mApplicationStatus; };
    inline const char * GetApplicationVersion() override { return mApplicationVersion; };

    inline void SetApplicationStatus(app::Clusters::ApplicationBasic::ApplicationStatusEnum applicationStatus) override
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
    app::Clusters::ApplicationBasic::ApplicationStatusEnum mApplicationStatus =
        app::Clusters::ApplicationBasic::ApplicationStatusEnum::kStopped;
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

class DLL_EXPORT KeypadInputImpl : public KeypadInput
{
public:
    virtual ~KeypadInputImpl() {}

protected:
};

class DLL_EXPORT ApplicationLauncherImpl : public ApplicationLauncher
{
public:
    virtual ~ApplicationLauncherImpl() {}

    ApplicationLauncherResponse LaunchApp(Application application, std::string data) override;

protected:
};

class DLL_EXPORT ContentLauncherImpl : public ContentLauncher
{
public:
    virtual ~ContentLauncherImpl() {}

    LaunchResponse LaunchContent(std::list<Parameter> parameterList, bool autoplay, std::string data) override;

protected:
};

class DLL_EXPORT MediaPlaybackImpl : public MediaPlayback
{
public:
    virtual ~MediaPlaybackImpl() {}

protected:
};

class DLL_EXPORT TargetNavigatorImpl : public TargetNavigator
{
public:
    TargetNavigatorImpl() : TargetNavigator{ { "home", "search", "info", "guide", "menu" }, 0 } {};
    virtual ~TargetNavigatorImpl() {}

protected:
};

class DLL_EXPORT ChannelImpl : public Channel
{
public:
    virtual ~ChannelImpl() {}

protected:
};

class DLL_EXPORT ContentAppImpl : public ContentApp
{
public:
    ContentAppImpl(const char * szVendorName, uint16_t vendorId, const char * szApplicationName, uint16_t productId,
                   const char * szApplicationVersion);
    virtual ~ContentAppImpl() {}

    inline ApplicationBasic * GetApplicationBasic() override { return &mApplicationBasic; };
    inline AccountLogin * GetAccountLogin() override { return &mAccountLogin; };
    inline KeypadInput * GetKeypadInput() override { return &mKeypadInput; };
    inline ApplicationLauncher * GetApplicationLauncher() override { return &mApplicationLauncher; };
    inline ContentLauncher * GetContentLauncher() override { return &mContentLauncher; };
    inline MediaPlayback * GetMediaPlayback() override { return &mMediaPlayback; };
    inline TargetNavigator * GetTargetNavigator() override { return &mTargetNavigator; };
    inline Channel * GetChannel() override { return &mChannel; };

protected:
    ApplicationBasicImpl mApplicationBasic;
    AccountLoginImpl mAccountLogin;
    KeypadInputImpl mKeypadInput;
    ApplicationLauncherImpl mApplicationLauncher;
    ContentLauncherImpl mContentLauncher;
    MediaPlaybackImpl mMediaPlayback;
    TargetNavigatorImpl mTargetNavigator;
    ChannelImpl mChannel;
};

class DLL_EXPORT ContentAppFactoryImpl : public ContentAppFactory
{
#define APP_LIBRARY_SIZE 4
public:
    ContentAppFactoryImpl();
    virtual ~ContentAppFactoryImpl() {}

    ContentApp * LoadContentAppByVendorId(uint16_t vendorId);
    ContentApp * LoadContentAppByAppId(Application application);

protected:
    ContentAppImpl mContentApps[APP_LIBRARY_SIZE] = { ContentAppImpl("Vendor1", 1, "App1", 11, "Version1"),
                                                      ContentAppImpl("Vendor2", 2222, "App2", 22, "Version2"),
                                                      ContentAppImpl("Vendor3", 9050, "App3", 22, "Version3"),
                                                      ContentAppImpl("TestSuiteVendor", 1111, "applicationId", 22, "v2") };
};

} // namespace AppPlatform
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

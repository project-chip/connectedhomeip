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
 * @brief classes relating to Content App platform of the Video Player.
 */

#pragma once

#include <app-common/zap-generated/enums.h>
#include <app/app-platform/ContentApp.h>
#include <app/util/attribute-storage.h>
#include <functional>
#include <platform/CHIPDeviceLayer.h>
#include <stdbool.h>
#include <stdint.h>

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
namespace chip {
namespace AppPlatform {

class DLL_EXPORT ContentAppFactory
{
public:
    virtual ~ContentAppFactory()                                        = default;
    virtual ContentApp * LoadContentAppByVendorId(uint16_t vendorId)    = 0;
    virtual ContentApp * LoadContentAppByAppId(Application application) = 0;

    // Gets the vendor ID for this platform
    virtual uint16_t GetPlatformCatalogVendorId() = 0;

    // Gets the Application ID for the given Application in the platform catalog
    virtual CharSpan GetPlatformCatalogApplicationId(Application application) = 0;
};

class DLL_EXPORT AppPlatform
{
public:
    static AppPlatform & GetInstance()
    {
        static AppPlatform instance;
        return instance;
    }

    void SetupAppPlatform();

    inline void SetContentAppFactory(ContentAppFactory * factory) { mContentAppFactory = factory; };

    // add and remove apps from the platform.
    // This will assign the app to an endpoint and make it accessible via Matter
    int AddContentApp(ContentApp * app, EmberAfEndpointType * ep, uint16_t deviceType);
    int RemoveContentApp(ContentApp * app);

    // load and unload by vendor id
    void UnloadContentAppByVendorId(uint16_t vendorId);
    ContentApp * GetLoadContentAppByVendorId(uint16_t vendorId);
    ContentApp * GetLoadContentAppByAppId(ApplicationLauncherApplication application);

    // helpful method to get a Content App by endpoint in order to perform attribute or command ops
    ContentApp * GetContentAppByEndpointId(chip::EndpointId id);
    ContentApp * GetContentAppByAppId(Application application);

    // sets the current app for this platform
    void SetCurrentApp(uint16_t catalogVendorId, CharSpan appId, chip::EndpointId endpoint);

    // returns true if there is a current app for this platform
    inline bool HasCurrentApp() { return !mNoCurrentApp; }

    // returns true if the vendor/app arguments are the current app
    bool IsCurrentApp(uint16_t catalogVendorId, CharSpan appId);

    // returns the current app
    chip::app::Clusters::ApplicationLauncher::Structs::ApplicationEP::Type * GetCurrentApp();

    // unset this as current app, if it is current app
    void UnsetIfCurrentApp(uint16_t catalogVendorId, CharSpan appId);

protected:
    bool mNoCurrentApp = true;
    chip::app::Clusters::ApplicationLauncher::Structs::ApplicationEP::Type mCurrentApp;
    static const int kEndpointStringSize = 6;
    char mCurrentApplicationEndpoint[kEndpointStringSize];

    ContentAppFactory * mContentAppFactory = nullptr;
    EndpointId mCurrentEndpointId;
    EndpointId mFirstDynamicEndpointId;
    ContentApp * mContentApps[CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT];
};

} // namespace AppPlatform
} // namespace chip
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

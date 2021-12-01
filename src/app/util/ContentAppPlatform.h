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
    virtual ~ContentAppFactory()                                     = default;
    virtual ContentApp * LoadContentAppByVendorId(uint16_t vendorId) = 0;
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

    // helpful method to get a Content App by endpoint in order to perform attribute or command ops
    ContentApp * GetContentAppByEndpointId(chip::EndpointId id);

protected:
    ContentAppFactory * mContentAppFactory = nullptr;
    EndpointId mCurrentEndpointId;
    EndpointId mFirstDynamicEndpointId;
    ContentApp * mContentApps[CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT];
};

} // namespace AppPlatform
} // namespace chip
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

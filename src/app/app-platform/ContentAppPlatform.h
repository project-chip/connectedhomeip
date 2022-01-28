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

using chip::app::Clusters::ApplicationBasic::CatalogVendorApp;

namespace chip {
namespace AppPlatform {

class DLL_EXPORT ContentAppFactory
{
public:
    virtual ~ContentAppFactory() = default;

    // Lookup CatalogVendor App for this client (vendor id/product id client)
    // and then write it to destinationApp
    // return error if not found
    virtual CHIP_ERROR LookupCatalogVendorApp(uint16_t vendorId, uint16_t productId, CatalogVendorApp * destinationApp) = 0;

    // Lookup ContentApp for this catalog id / app id and load it
    virtual ContentApp * LoadContentApp(CatalogVendorApp vendorApp) = 0;

    // Gets the catalog vendor ID used by this platform
    virtual uint16_t GetPlatformCatalogVendorId() = 0;

    // Converts application (any catalog) into the platform's catalog Vendor
    // and then writes it to destinationApp
    virtual CHIP_ERROR ConvertToPlatformCatalogVendorApp(CatalogVendorApp sourceApp, CatalogVendorApp * destinationApp) = 0;
};

class DLL_EXPORT ContentAppPlatform
{
public:
    static ContentAppPlatform & GetInstance()
    {
        static ContentAppPlatform instance;
        return instance;
    }

    void SetupAppPlatform();

    inline void SetContentAppFactory(ContentAppFactory * factory) { mContentAppFactory = factory; };

    // add apps to the platform.
    // This will assign the app to an endpoint (if it is not already added) and make it accessible via Matter
    // returns the global endpoint for this app, or 0 if an error occurred
    EndpointId AddContentApp(ContentApp * app, EmberAfEndpointType * ep, uint16_t deviceType);

    // remove app from the platform.
    // returns the endpoint id where the app was, or 0 if app was not loaded
    EndpointId RemoveContentApp(ContentApp * app);

    // load and unload by vendor id
    // void UnloadContentAppByVendorId(uint16_t vendorId, uint16_t productId);

    // Lookup ContentApp for this client (vendor id/product id client) and load it
    ContentApp * LoadContentAppByClient(uint16_t vendorId, uint16_t productId);

    // Lookup ContentApp described by this application and load it
    ContentApp * LoadContentApp(CatalogVendorApp application);

    // helpful method to get a Content App by endpoint in order to perform attribute or command ops
    ContentApp * GetContentApp(EndpointId id);

    // helpful method to get a Content App by application, does not load if not found
    ContentApp * GetContentApp(CatalogVendorApp application);

    // sets the current app for this platform
    void SetCurrentApp(ContentApp * app);

    // returns true if there is a current app for this platform
    inline bool HasCurrentApp() { return mCurrentAppEndpointId != kNoCurrentEndpointId; }

    // returns true if the vendor/app arguments are the current app
    bool IsCurrentApp(ContentApp * app);

    // returns the current app endpoint
    inline EndpointId GetCurrentAppEndpointId() { return mCurrentAppEndpointId; };

    // unset this as current app, if it is current app
    void UnsetIfCurrentApp(ContentApp * app);

    // loads content app identified by vid/pid of client and calls HandleGetSetupPin.
    // Returns 0 if pin cannot be obtained.
    uint32_t GetPincodeFromContentApp(uint16_t vendorId, uint16_t productId, CharSpan rotatingId);

protected:
    // requires vendorApp to be in the catalog of the platform
    ContentApp * LoadContentAppInternal(CatalogVendorApp vendorApp);
    ContentApp * GetContentAppInternal(CatalogVendorApp vendorApp);

    static const int kNoCurrentEndpointId = 0;
    EndpointId mCurrentAppEndpointId      = kNoCurrentEndpointId;

    ContentAppFactory * mContentAppFactory = nullptr;
    EndpointId mCurrentEndpointId;
    EndpointId mFirstDynamicEndpointId;
    ContentApp * mContentApps[CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT];
};

} // namespace AppPlatform
} // namespace chip

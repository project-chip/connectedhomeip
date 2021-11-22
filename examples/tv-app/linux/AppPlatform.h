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
#include <app/util/attribute-storage.h>

#include <stdbool.h>
#include <stdint.h>

#include <functional>

namespace chip {
namespace AppPlatform {

class ContentApp
{
public:
    static const int kVendorNameSize         = 32;
    static const int kApplicationNameSize    = 32;
    static const int kApplicationVersionSize = 32;

    ContentApp(const char * szVendorName, uint16_t vendorId, const char * szApplicationName, uint16_t productId,
               const char * szApplicationVersion);
    virtual ~ContentApp() {}

    inline void SetEndpointId(chip::EndpointId id) { mEndpointId = id; };
    inline chip::EndpointId GetEndpointId() { return mEndpointId; };

    // Start: App Basic Cluster
    void SetVendorName(const char * szVendorName);
    inline void SetVendorId(uint16_t vendorId) { mVendorId = vendorId; };
    void SetApplicationName(const char * szApplicationName);
    inline void SetProductId(uint16_t productId) { mProductId = productId; };
    inline void SetApplicationStatus(EmberAfApplicationBasicStatus applicationStatus) { mApplicationStatus = applicationStatus; };
    void SetApplicationVersion(const char * szApplicationVersion);

    inline const char * GetVendorName() { return mVendorName; };
    inline uint16_t GetVendorId() { return mVendorId; };
    inline const char * GetApplicationName() { return mApplicationName; };
    inline uint16_t GetProductId() { return mProductId; };
    inline EmberAfApplicationBasicStatus GetApplicationStatus() { return mApplicationStatus; };
    inline const char * GetApplicationVersion() { return mApplicationVersion; };

    // Start: App Login Cluster
    inline void AccountLogin_SetSetupPIN(uint32_t setupPIN) { mAccountLogin_SetupPIN = setupPIN; };

    uint32_t AccountLogin_GetSetupPIN(const char * tempAccountId);

    bool AccountLogin_Login(const char * tempAccountId, uint32_t setupPin);

protected:
    chip::EndpointId mEndpointId = 0;

    // Start: App Basic Cluster
    char mVendorName[kVendorNameSize];
    uint16_t mVendorId;
    char mApplicationName[kApplicationNameSize];
    uint16_t mProductId;
    EmberAfApplicationBasicStatus mApplicationStatus = EMBER_ZCL_APPLICATION_BASIC_STATUS_STOPPED;
    char mApplicationVersion[kApplicationVersionSize];

    // Start: App Login Cluster
    uint32_t mAccountLogin_SetupPIN;
};

void SetupAppPlatform();
ContentApp * GetLoadContentAppByVendorId(uint16_t vendorId);
ContentApp * GetContentAppByEndpointId(chip::EndpointId id);

} // namespace AppPlatform
} // namespace chip

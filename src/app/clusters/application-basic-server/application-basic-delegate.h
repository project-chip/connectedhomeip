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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>

#include <app/AttributeValueEncoder.h>
#include <lib/support/CHIPMemString.h>

#include <list>
#include <string>

namespace chip {
namespace app {
namespace Clusters {
namespace ApplicationBasic {

using ApplicationBasicApplicationType = chip::app::Clusters::ApplicationBasic::Structs::ApplicationStruct::Type;

class DLL_EXPORT CatalogVendorApp
{
public:
    CatalogVendorApp(){};
    CatalogVendorApp(CatalogVendorApp * app)
    {
        catalogVendorId = app->catalogVendorId;
        Platform::CopyString(applicationId, sizeof(applicationId), app->applicationId);
    };
    CatalogVendorApp(uint16_t vendorId, const char * appId) { Set(vendorId, appId); };

    bool Matches(const CatalogVendorApp & app)
    {
        std::string appId1(applicationId);
        std::string appId2(app.applicationId);

        return catalogVendorId == app.catalogVendorId && appId1 == appId2;
    }

    void Set(uint16_t vendorId, const char * appId)
    {
        catalogVendorId = vendorId;
        Platform::CopyString(applicationId, sizeof(applicationId), appId);
    }

    const char * GetApplicationId() { return applicationId; }

    static const int kApplicationIdSize = 32;
    char applicationId[kApplicationIdSize];
    uint16_t catalogVendorId;
};

/** @brief
 *    Defines methods for implementing application-specific logic for the Application Basic Cluster.
 */
class Delegate
{
public:
    Delegate() : Delegate(123, "applicationId"){};
    Delegate(uint16_t szCatalogVendorId, const char * szApplicationId) : mCatalogVendorApp(szCatalogVendorId, szApplicationId){};

    virtual CHIP_ERROR HandleGetVendorName(app::AttributeValueEncoder & aEncoder)      = 0;
    virtual uint16_t HandleGetVendorId()                                               = 0;
    virtual CHIP_ERROR HandleGetApplicationName(app::AttributeValueEncoder & aEncoder) = 0;
    virtual uint16_t HandleGetProductId()                                              = 0;
    CHIP_ERROR HandleGetApplication(app::AttributeValueEncoder & aEncoder);
    inline ApplicationStatusEnum HandleGetStatus() { return mApplicationStatus; }
    virtual CHIP_ERROR HandleGetApplicationVersion(app::AttributeValueEncoder & aEncoder) = 0;
    virtual CHIP_ERROR HandleGetAllowedVendorList(app::AttributeValueEncoder & aEncoder)  = 0;

    inline void SetApplicationStatus(ApplicationStatusEnum status) { mApplicationStatus = status; }
    bool Matches(const ApplicationBasicApplicationType & match);

    inline CatalogVendorApp * GetCatalogVendorApp() { return &mCatalogVendorApp; }
    virtual std::list<uint16_t> GetAllowedVendorList() = 0;

    virtual ~Delegate() = default;

protected:
    CatalogVendorApp mCatalogVendorApp;
    ApplicationStatusEnum mApplicationStatus = ApplicationStatusEnum::kStopped;
};

} // namespace ApplicationBasic
} // namespace Clusters
} // namespace app
} // namespace chip

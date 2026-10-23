/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/application-basic-server/application-basic-delegate.h>
#include <app/util/config.h>

#if MATTER_DM_APPLICATION_BASIC_CLUSTER_SERVER_ENDPOINT_COUNT > 0

namespace chip::app::Clusters::ApplicationBasic::Chef {

class ChefDelegate : public Delegate
{
public:
    ChefDelegate(EndpointId endpointId, const char * vendorName, uint16_t catalogVendorId, const char * applicationId,
                 const char * applicationName, const char * applicationVersion, const Span<const uint16_t> allowedVendorList,
                 const uint16_t productId);

    CHIP_ERROR HandleGetApplicationName(app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetApplicationVersion(app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetAllowedVendorList(app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetVendorName(app::AttributeValueEncoder & aEncoder) override;
    uint16_t HandleGetVendorId() override;
    uint16_t HandleGetProductId() override;
    std::list<uint16_t> GetAllowedVendorList() override;

    EndpointId GetEndpointId() { return mEndpointId; }

    void Register();

    ApplicationStatusEnum GetApplicationStatus() const { return mApplicationStatus; }

private:
    EndpointId mEndpointId;
    const char * mApplicationName;
    const char * mVendorName;
    const uint16_t mProductId;
    const char * mApplicationVersion;
    const Span<const uint16_t> mAllowedVendorList;
};

} // namespace chip::app::Clusters::ApplicationBasic::Chef

#endif // MATTER_DM_APPLICATION_BASIC_CLUSTER_SERVER_ENDPOINT_COUNT

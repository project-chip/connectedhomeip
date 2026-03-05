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

#include "chef-application-basic-delegate.h"

#include <app/clusters/application-basic-server/application-basic-server.h>
#include <app/util/config.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>

#if MATTER_DM_APPLICATION_BASIC_CLUSTER_SERVER_ENDPOINT_COUNT > 0

namespace chip::app::Clusters::ApplicationBasic::Chef {

ChefDelegate::ChefDelegate(EndpointId endpointId, const char * vendorName, uint16_t catalogVendorId, const char * applicationId,
                           const char * applicationName, const char * applicationVersion,
                           const Span<const uint16_t> allowedVendorList, const uint16_t productId) :
    Delegate(catalogVendorId, applicationId),
    mEndpointId(endpointId), mApplicationName(applicationName), mVendorName(vendorName), mProductId(productId),
    mApplicationVersion(applicationVersion), mAllowedVendorList(allowedVendorList)
{}

CHIP_ERROR ChefDelegate::HandleGetApplicationName(app::AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(Zcl, "ApplicationBasic::Chef::ChefDelegate::HandleGetApplicationName %s", mApplicationName);
    return aEncoder.Encode(chip::CharSpan(mApplicationName, strlen(mApplicationName)));
}

CHIP_ERROR ChefDelegate::HandleGetApplicationVersion(app::AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(Zcl, "ApplicationBasic::Chef::ChefDelegate::HandleGetApplicationVersion %s", mApplicationVersion);
    return aEncoder.Encode(chip::CharSpan(mApplicationVersion, strlen(mApplicationVersion)));
}

CHIP_ERROR ChefDelegate::HandleGetAllowedVendorList(app::AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(Zcl, "ApplicationBasic::Chef::ChefDelegate::HandleGetAllowedVendorList");
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (const auto & vendorId : mAllowedVendorList)
        {
            ReturnErrorOnFailure(encoder.Encode(vendorId));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR ChefDelegate::HandleGetVendorName(app::AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(Zcl, "ApplicationBasic::Chef::ChefDelegate::HandleGetVendorName %s", mVendorName);
    return aEncoder.Encode(chip::CharSpan(mVendorName, strlen(mVendorName)));
}

uint16_t ChefDelegate::HandleGetVendorId()
{
    ChipLogProgress(Zcl, "ApplicationBasic::Chef::ChefDelegate::HandleGetVendorId %d", mCatalogVendorApp.catalogVendorId);
    return mCatalogVendorApp.catalogVendorId;
}

uint16_t ChefDelegate::HandleGetProductId()
{
    ChipLogProgress(Zcl, "ApplicationBasic::Chef::ChefDelegate::HandleGetProductId %d", mProductId);
    return mProductId;
}

std::list<uint16_t> ChefDelegate::GetAllowedVendorList()
{
    ChipLogProgress(Zcl, "ApplicationBasic::Chef::ChefDelegate::GetAllowedVendorList");
    return std::list<uint16_t>(mAllowedVendorList.begin(), mAllowedVendorList.end());
}

void ChefDelegate::Register()
{
    ChipLogProgress(Zcl, "ApplicationBasic::Chef::ChefDelegate::Register");
    SetDefaultDelegate(mEndpointId, this);
}

} // namespace chip::app::Clusters::ApplicationBasic::Chef

#endif // MATTER_DM_APPLICATION_BASIC_CLUSTER_SERVER_ENDPOINT_COUNT

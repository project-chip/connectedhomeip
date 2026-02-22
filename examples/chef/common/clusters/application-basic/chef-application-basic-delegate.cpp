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

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ApplicationBasic;

namespace chip {
namespace app {
namespace Clusters {
namespace ApplicationBasic {
namespace Chef {

ChefDelegate::ChefDelegate(EndpointId endpointId, uint16_t catalogVendorId, const char * applicationId,
                           const char * applicationName, const char * applicationVersion,
                           const Span<const uint16_t> allowedVendorList) :
    Delegate(catalogVendorId, applicationId), mEndpointId(endpointId), mApplicationName(applicationName),
    mApplicationVersion(applicationVersion), mAllowedVendorList(allowedVendorList)
{}

CHIP_ERROR ChefDelegate::HandleGetApplicationName(app::AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(chip::CharSpan(mApplicationName, strlen(mApplicationName)));
}

CHIP_ERROR ChefDelegate::HandleGetApplicationVersion(app::AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(chip::CharSpan(mApplicationVersion.c_str(), mApplicationVersion.length()));
}

CHIP_ERROR ChefDelegate::HandleGetAllowedVendorList(app::AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (const auto & vendorId : mAllowedVendorList)
        {
            ReturnErrorOnFailure(encoder.Encode(vendorId));
        }
        return CHIP_NO_ERROR;
    });
}

void ChefDelegate::Register()
{
    SetDefaultDelegate(mEndpointId, this);
}

} // namespace Chef
} // namespace ApplicationBasic
} // namespace Clusters
} // namespace app
} // namespace chip

#endif // MATTER_DM_APPLICATION_BASIC_CLUSTER_SERVER_ENDPOINT_COUNT
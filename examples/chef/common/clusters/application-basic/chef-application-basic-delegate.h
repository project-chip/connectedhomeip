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
#include <lib/support/IntrusiveList.h>
#include <string>

namespace chip {
namespace app {
namespace Clusters {
namespace ApplicationBasic {
namespace Chef {

class ChefDelegate : public Delegate, public chip::IntrusiveListNodeBase<>
{
public:
    ChefDelegate(EndpointId endpointId, uint16_t catalogVendorId, const char * applicationId, const char * applicationName,
                 const char * applicationVersion, const Span<const uint16_t> allowedVendorList);

    CHIP_ERROR HandleGetApplicationName(app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetApplicationVersion(app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetAllowedVendorList(app::AttributeValueEncoder & aEncoder) override;

    EndpointId GetEndpointId() { return mEndpointId; }
    const char * GetApplicationId() { return mApplicationName.c_str(); }

private:
    EndpointId mEndpointId;
    std::string mApplicationName;
    uint16_t mVendorId;
    std::string mVendorName;
    uint16_t mProductId;
    std::string mApplicationVersion;
};

/**
 * @brief Adds an Application Basic delegate for the given endpoint.
 *
 * @param endpoint The endpoint to add the delegate for.
 */
void AddApplicationBasicDelegateForEndpoint(EndpointId endpoint);

} // namespace Chef
} // namespace ApplicationBasic
} // namespace Clusters
} // namespace app
} // namespace chip

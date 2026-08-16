/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/DeviceProxy.h>
#include <app/ReadClient.h>
#include <controller/AutoCommissioner.h>
#include <controller/CHIPDeviceController.h>
#include <credentials/jcm/TrustVerification.h>
#include <lib/core/CHIPCallback.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPError.h>
#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

namespace chip {

namespace Controller {

namespace JCM {

/*
 * AutoCommissioner is a class that handles the Joint Commissioning Management (JCM) process
 * for commissioning Joint Fabric Administrator devices in a CHIP network. It extends the AutoCommissioner class and
 * helps setup for the JCM trust verification process.
 */
class AutoCommissioner : public chip::Controller::AutoCommissioner
{
public:
    AutoCommissioner(){};
    ~AutoCommissioner(){};

    CHIP_ERROR SetCommissioningParameters(const CommissioningParameters & params) override;
    void CleanupCommissioning() override;

private:
    // Joint Fabric Management: all attributes
    const std::vector<app::AttributePathParams> mExtraReadPaths = {
        app::AttributePathParams(app::Clusters::JointFabricAdministrator::Id,
                                 app::Clusters::JointFabricAdministrator::Attributes::AdministratorFabricIndex::Id),
        app::AttributePathParams(kRootEndpointId, app::Clusters::OperationalCredentials::Id,
                                 app::Clusters::OperationalCredentials::Attributes::Fabrics::Id),
        app::AttributePathParams(kRootEndpointId, app::Clusters::OperationalCredentials::Id,
                                 app::Clusters::OperationalCredentials::Attributes::NOCs::Id),
        app::AttributePathParams(kRootEndpointId, app::Clusters::OperationalCredentials::Id,
                                 app::Clusters::OperationalCredentials::Attributes::TrustedRootCertificates::Id)
    };
    std::vector<app::AttributePathParams> mTempReadPaths;
};

} // namespace JCM
} // namespace Controller
} // namespace chip

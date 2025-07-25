/**
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "AccessControl.h"

#include <access/AccessControl.h>
#include <access/Privilege.h>
#include <access/ProviderDeviceTypeResolver.h>
#include <access/RequestPath.h>
#include <access/SubjectDescriptor.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/InteractionModelEngine.h>
#include <lib/core/CHIPError.h>
#include <lib/core/Global.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::Access;
using namespace chip::app::Clusters;

namespace {

class DeviceTypeResolver : public chip::Access::DynamicProviderDeviceTypeResolver
{
public:
    DeviceTypeResolver() :
        chip::Access::DynamicProviderDeviceTypeResolver(
            [] { return chip::app::InteractionModelEngine::GetInstance()->GetDataModelProvider(); })
    {}
};

class AccessControlDelegate : public Access::AccessControl::Delegate
{
    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath,
                     Privilege requestPrivilege) override
    {
        // Check for WebRTC Transport Requestor endpoint
        bool isWebRtcEndpoint =
            (requestPath.endpoint == kWebRTCRequesterDynamicEndpointId && requestPath.cluster == WebRTCTransportRequestor::Id);

        // Only allow these specific endpoints
        if (!isWebRtcEndpoint)
        {
            return CHIP_ERROR_ACCESS_DENIED;
        }

        // Allow kOperate and below (kView, kProxyView, etc.)
        // This covers reading attributes (kView) and invoking commands (kOperate)
        if (requestPrivilege > Privilege::kOperate)
        {
            return CHIP_ERROR_ACCESS_DENIED;
        }

        if (subjectDescriptor.authMode != AuthMode::kCase)
        {
            // Restrict to Case
            return CHIP_ERROR_ACCESS_DENIED;
        }

        return CHIP_NO_ERROR;
    }
};

struct ControllerAccessControl
{
    DeviceTypeResolver mDeviceTypeResolver;
    AccessControlDelegate mDelegate;
    bool mInitialized = false;

    // Remove constructor initialization
    ControllerAccessControl() = default;

    // Add explicit Init method
    CHIP_ERROR Init()
    {
        if (mInitialized)
        {
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR err = GetAccessControl().Init(&mDelegate, mDeviceTypeResolver);
        if (err == CHIP_NO_ERROR)
        {
            mInitialized = true;
        }
        return err;
    }
};

Global<ControllerAccessControl> gControllerAccessControl;

} // anonymous namespace

namespace chip {
namespace app {
namespace AccessControl {

void InitAccessControl()
{
    CHIP_ERROR err = gControllerAccessControl.get().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to initialize access control: %" CHIP_ERROR_FORMAT, err.Format());
    }
    else
    {
        ChipLogProgress(AppServer, "Access control initialized successfully");
    }
}

} // namespace AccessControl
} // namespace app
} // namespace chip

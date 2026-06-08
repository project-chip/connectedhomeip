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

#include "WebRTCAccessControl.h"

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
public:
    void SetWebRTCEndpointId(EndpointId endpointId) { mWebRtcEndpointId = endpointId; }

    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath,
                     Privilege requestPrivilege) override
    {
        // Check for WebRTC Transport Requestor endpoint and cluster
        bool isWebRtcTransportRequestor =
            (requestPath.endpoint == mWebRtcEndpointId && requestPath.cluster == WebRTCTransportRequestor::Id);

        // If the request is not for WebRTC Transport Requestor, deny access
        if (!isWebRtcTransportRequestor)
        {
            return CHIP_ERROR_ACCESS_DENIED;
        }

        // Allow kOperate and below (kView, kProxyView, etc.)
        // This covers reading attributes (kView) and invoking commands (kOperate)
        if (requestPrivilege > Privilege::kOperate)
        {
            return CHIP_ERROR_ACCESS_DENIED;
        }

        return CHIP_NO_ERROR;
    }

private:
    EndpointId mWebRtcEndpointId = kInvalidEndpointId;
};

struct ControllerAccessControl
{
    DeviceTypeResolver mDeviceTypeResolver;
    AccessControlDelegate mDelegate;
    bool mInitialized = false;

    ControllerAccessControl() = default;

    CHIP_ERROR Init(EndpointId endpointId)
    {
        if (mInitialized)
        {
            return CHIP_NO_ERROR;
        }

        mDelegate.SetWebRTCEndpointId(endpointId);

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
namespace Controller {
namespace AccessControl {

void InitAccessControl(EndpointId endpointId)
{
    CHIP_ERROR err = gControllerAccessControl.get().Init(endpointId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to initialize access control: %" CHIP_ERROR_FORMAT, err.Format());
    }
    else
    {
        ChipLogProgress(AppServer, "Access control initialized successfully for endpoint %u", endpointId);
    }
}

} // namespace AccessControl
} // namespace Controller
} // namespace chip

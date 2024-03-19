/**
 *    Copyright (c) 2022-2023 Project CHIP Authors
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
#include <access/RequestPath.h>
#include <access/SubjectDescriptor.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/InteractionModelEngine.h>
#include <lib/core/CHIPError.h>
#include <lib/core/Global.h>

// TODO: this include is unclear as dynamic server should NOT link those.
//       we should probably have some separate includes here for dynamic
//       server
#include <app/util/ember-compatibility-functions.h>

using namespace chip;
using namespace chip::Access;
using namespace chip::app::Clusters;

namespace {
// TODO: Maybe consider making this configurable?  See also
// DynamicDispatch.cpp.
constexpr EndpointId kSupportedEndpoint = 0;

class DeviceTypeResolver : public Access::AccessControl::DeviceTypeResolver
{
public:
    bool IsDeviceTypeOnEndpoint(DeviceTypeId deviceType, EndpointId endpoint) override
    {
        return app::IsDeviceTypeOnEndpoint(deviceType, endpoint);
    }
};

// TODO: Make the policy more configurable by consumers.
class AccessControlDelegate : public Access::AccessControl::Delegate
{
    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath,
                     Privilege requestPrivilege) override
    {
        if (requestPath.endpoint != kSupportedEndpoint || requestPath.cluster != OtaSoftwareUpdateProvider::Id)
        {
            // We only allow access to OTA software update provider.
            return CHIP_ERROR_ACCESS_DENIED;
        }

        if (requestPrivilege != Privilege::kOperate)
        {
            // The commands on OtaSoftwareUpdateProvider all require
            // Operate; we should not be asked for anything else.
            return CHIP_ERROR_ACCESS_DENIED;
        }

        if (subjectDescriptor.authMode != AuthMode::kCase && subjectDescriptor.authMode != AuthMode::kPase)
        {
            // No idea who is asking; deny for now.
            return CHIP_ERROR_ACCESS_DENIED;
        }

        // TODO do we care about the fabric index here?  Probably not.

        return CHIP_NO_ERROR;
    }
};

struct ControllerAccessControl
{
    DeviceTypeResolver mDeviceTypeResolver;
    AccessControlDelegate mDelegate;
    ControllerAccessControl() { GetAccessControl().Init(&mDelegate, mDeviceTypeResolver); }
};

Global<ControllerAccessControl> gControllerAccessControl;

} // anonymous namespace

namespace chip {
namespace app {
namespace dynamic_server {
void InitAccessControl()
{
    gControllerAccessControl.get(); // force initialization
}
} // namespace dynamic_server
} // namespace app
} // namespace chip

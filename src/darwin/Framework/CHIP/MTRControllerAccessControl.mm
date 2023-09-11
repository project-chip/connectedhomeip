/**
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRControllerAccessControl.h"

#include <access/AccessControl.h>
#include <access/Privilege.h>
#include <access/RequestPath.h>
#include <access/SubjectDescriptor.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/InteractionModelEngine.h>
#include <lib/core/CHIPError.h>

using namespace chip;
using namespace chip::Access;
using namespace chip::app::Clusters;

namespace {
// TODO: Maybe consider making this configurable?  See also
// CHIPIMDispatch.mm.
constexpr EndpointId kSupportedEndpoint = 0;

class DeviceTypeResolver : public Access::AccessControl::DeviceTypeResolver {
public:
    bool IsDeviceTypeOnEndpoint(DeviceTypeId deviceType, EndpointId endpoint) override
    {
        return app::IsDeviceTypeOnEndpoint(deviceType, endpoint);
    }
} gDeviceTypeResolver;

// TODO: Make the policy more configurable by consumers.
class AccessControlDelegate : public Access::AccessControl::Delegate {
    CHIP_ERROR Check(
        const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath, Privilege requestPrivilege) override
    {
        if (requestPath.endpoint != kSupportedEndpoint || requestPath.cluster != OtaSoftwareUpdateProvider::Id) {
            // We only allow access to OTA software update provider.
            return CHIP_ERROR_ACCESS_DENIED;
        }

        if (requestPrivilege != Privilege::kOperate) {
            // The commands on OtaSoftwareUpdateProvider all require
            // Operate; we should not be asked for anything else.
            return CHIP_ERROR_ACCESS_DENIED;
        }

        if (subjectDescriptor.authMode != AuthMode::kCase && subjectDescriptor.authMode != AuthMode::kPase) {
            // No idea who is asking; deny for now.
            return CHIP_ERROR_ACCESS_DENIED;
        }

        // TODO do we care about the fabric index here?  Probably not.

        return CHIP_NO_ERROR;
    }
};

AccessControlDelegate gDelegate;
} // anonymous namespace

@implementation MTRControllerAccessControl

+ (void)init
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        GetAccessControl().Init(&gDelegate, gDeviceTypeResolver);
    });
}

@end

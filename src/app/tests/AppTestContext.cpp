/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <app/tests/AppTestContext.h>

#include <access/AccessControl.h>
#include <access/examples/PermissiveAccessControlDelegate.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/tests/MockReportScheduler.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>

namespace {

class TestDeviceTypeResolver : public chip::Access::AccessControl::DeviceTypeResolver
{
public:
    bool IsDeviceTypeOnEndpoint(chip::DeviceTypeId deviceType, chip::EndpointId endpoint) override { return false; }
} gDeviceTypeResolver;

chip::Access::AccessControl gPermissiveAccessControl;

} // namespace

namespace chip {
namespace Test {

CHIP_ERROR AppContext::Init()
{
    ReturnErrorOnFailure(Super::Init());
    ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->Init(&GetExchangeManager(), &GetFabricTable(),
                                                                                app::reporting::GetDefaultReportScheduler()));

    Access::SetAccessControl(gPermissiveAccessControl);
    ReturnErrorOnFailure(
        Access::GetAccessControl().Init(chip::Access::Examples::GetPermissiveAccessControlDelegate(), gDeviceTypeResolver));

    return CHIP_NO_ERROR;
}

void AppContext::Shutdown()
{
    Access::GetAccessControl().Finish();
    Access::ResetAccessControlToDefault();

    chip::app::InteractionModelEngine::GetInstance()->Shutdown();
    Super::Shutdown();
}

} // namespace Test
} // namespace chip

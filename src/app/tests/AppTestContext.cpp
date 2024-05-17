/*
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "AppTestContext.h"

#include <access/AccessControl.h>
#include <access/examples/PermissiveAccessControlDelegate.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/tests/MockReportScheduler.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/CodeUtils.h>

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

void AppContext::SetUpTestSuite()
{
    LoopbackMessagingContext::SetUpTestSuite();
    VerifyOrReturn(!HasFailure()); // Stop if parent had a failure.

    ASSERT_EQ(chip::DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
}

void AppContext::TearDownTestSuite()
{
    chip::DeviceLayer::PlatformMgr().Shutdown();
    LoopbackMessagingContext::TearDownTestSuite();
}

void AppContext::SetUp()
{
    LoopbackMessagingContext::SetUp();
    VerifyOrReturn(!HasFailure()); // Stop if parent had a failure.

    ASSERT_EQ(app::InteractionModelEngine::GetInstance()->Init(&GetExchangeManager(), &GetFabricTable(),
                                                               app::reporting::GetDefaultReportScheduler()),
              CHIP_NO_ERROR);
    Access::SetAccessControl(gPermissiveAccessControl);
    ASSERT_EQ(Access::GetAccessControl().Init(chip::Access::Examples::GetPermissiveAccessControlDelegate(), gDeviceTypeResolver),
              CHIP_NO_ERROR);
}

void AppContext::TearDown()
{
    Access::GetAccessControl().Finish();
    Access::ResetAccessControlToDefault();
    chip::app::InteractionModelEngine::GetInstance()->Shutdown();

    LoopbackMessagingContext::TearDown();
}

} // namespace Test
} // namespace chip

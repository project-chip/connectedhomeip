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

#include <app/tests/AppTestContext.h>

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
    CHIP_ERROR err = CHIP_NO_ERROR;
    LoopbackMessagingContext::SetUpTestSuite();
    // TODO: use ASSERT_EQ, once transition to pw_unit_test is complete
    VerifyOrDieWithMsg((err = chip::DeviceLayer::PlatformMgr().InitChipStack()) == CHIP_NO_ERROR, AppServer,
                       "Init CHIP stack failed: %" CHIP_ERROR_FORMAT, err.Format());
}

void AppContext::TearDownTestSuite()
{
    // Some test suites finish with unprocessed work left in the platform manager event queue.
    // This can particularly be a problem when this unprocessed work involves reporting engine runs,
    // since those can take a while and cause later tests to not reach their queued work before
    // their timeouts hit.  This is only an issue in setups where all unit tests are compiled into
    // a single file (e.g. nRF CI (Zephyr native_posix)).
    //
    // Work around this issue by doing a DrainAndServiceIO() here to attempt to flush out any queued-up work.
    //
    // TODO: Solve the underlying issue where test suites leave unprocessed work.  Or is this actually
    // the right solution?
    LoopbackMessagingContext::DrainAndServiceIO();

    chip::DeviceLayer::PlatformMgr().Shutdown();
    LoopbackMessagingContext::TearDownTestSuite();
}

void AppContext::SetUp()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    LoopbackMessagingContext::SetUp();
    // TODO: use ASSERT_EQ, once transition to pw_unit_test is complete
    VerifyOrDieWithMsg((err = app::InteractionModelEngine::GetInstance()->Init(&GetExchangeManager(), &GetFabricTable(),
                                                                               app::reporting::GetDefaultReportScheduler())) ==
                           CHIP_NO_ERROR,
                       AppServer, "Init InteractionModelEngine failed: %" CHIP_ERROR_FORMAT, err.Format());
    Access::SetAccessControl(gPermissiveAccessControl);
    VerifyOrDieWithMsg((err = Access::GetAccessControl().Init(chip::Access::Examples::GetPermissiveAccessControlDelegate(),
                                                              gDeviceTypeResolver)) == CHIP_NO_ERROR,
                       AppServer, "Init AccessControl failed: %" CHIP_ERROR_FORMAT, err.Format());
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

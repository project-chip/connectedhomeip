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

CHIP_ERROR AppContext::SetUpTestSuite()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit((err = chip::Platform::MemoryInit()) == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Init CHIP memory failed: %" CHIP_ERROR_FORMAT, err.Format()));
    VerifyOrExit((err = LoopbackTransportManager::Init()) == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Init LoopbackTransportManager failed: %" CHIP_ERROR_FORMAT, err.Format()));
    VerifyOrExit((err = chip::DeviceLayer::PlatformMgr().InitChipStack()) == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Init CHIP stack failed: %" CHIP_ERROR_FORMAT, err.Format()));
exit:
    return err;
}

void AppContext::TearDownTestSuite()
{
    chip::DeviceLayer::PlatformMgr().Shutdown();
    LoopbackTransportManager::Shutdown();
    chip::Platform::MemoryShutdown();
}

CHIP_ERROR AppContext::SetUp()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit((err = MessagingContext::Init(&GetTransportMgr(), &GetIOContext())) == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Init MessagingContext failed: %" CHIP_ERROR_FORMAT, err.Format()));
    VerifyOrExit((err = app::InteractionModelEngine::GetInstance()->Init(
                      &GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler())) == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Init InteractionModelEngine failed: %" CHIP_ERROR_FORMAT, err.Format()));
    Access::SetAccessControl(gPermissiveAccessControl);
    VerifyOrExit((err = Access::GetAccessControl().Init(chip::Access::Examples::GetPermissiveAccessControlDelegate(),
                                                        gDeviceTypeResolver)) == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Init AccessControl failed: %" CHIP_ERROR_FORMAT, err.Format()));
exit:
    return err;
}

void AppContext::TearDown()
{
    Access::GetAccessControl().Finish();
    Access::ResetAccessControlToDefault();
    chip::app::InteractionModelEngine::GetInstance()->Shutdown();
    MessagingContext::Shutdown();
}

} // namespace Test
} // namespace chip

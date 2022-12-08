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
    ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->Init(&GetExchangeManager(), &GetFabricTable()));

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

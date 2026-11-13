/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

// Linux implementation of the JFAPlatform hooks declared in
// examples/jf-admin-app/jfa-common/include/JFAPlatform.h.
//
// This file is the single place the shared jf-admin-app code touches the Linux
// example platform (CommissionerMain.h / LinuxDeviceOptions). All other shared
// files route through the JFAPlatform.h interface.

#include "JFAPlatform.h"

#include <AppMain.h>
#include <CommissionerMain.h>

namespace chip {

CHIP_ERROR JFAInitCommissioner(FabricId fabricId, FabricIndex fabricIndex)
{
    return ::InitCommissioner(LinuxDeviceOptions::GetInstance().securedCommissionerPort,
                              LinuxDeviceOptions::GetInstance().unsecuredCommissionerPort, fabricId, fabricIndex);
}

Controller::DeviceCommissioner * JFAGetDeviceCommissioner()
{
    return ::GetDeviceCommissioner();
}

} // namespace chip

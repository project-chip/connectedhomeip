/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <platform/CHIPDeviceLayer.h>
// #include <support/CHIPMem.h>
// #include <support/CodeUtils.h>
// #include <support/ErrorStr.h>
#include <support/UnitTestRegistration.h>
#include <support/logging/CHIPLogging.h>

using namespace ::chip;
using namespace ::chip::Inet;
// using namespace ::chip::DeviceLayer;

int main()
{
    ChipLogProgress(NotSpecified, "Starting CHIP tests!");
    int status = RunRegisteredUnitTests();
    ChipLogProgress(NotSpecified, "CHIP test status: %d", status);

    return status;
}

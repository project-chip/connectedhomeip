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
#include "netsocket/WiFiInterface.h"
#include <support/UnitTestRegistration.h>
#include <support/logging/CHIPLogging.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;
WiFiInterface * _interface;
#define concat_(x, y) x##y
#define concat(x, y) concat_(x, y)
#define SECURITY concat(NSAPI_SECURITY_, MBED_CONF_NSAPI_DEFAULT_WIFI_SECURITY)

int main()
{
    ChipLogProgress(NotSpecified, "Starting CHIP tests!");
    _interface = WiFiInterface::get_default_instance();
    _interface->connect(MBED_CONF_NSAPI_DEFAULT_WIFI_SSID, MBED_CONF_NSAPI_DEFAULT_WIFI_PASSWORD, SECURITY);
    _interface->get_connection_status();

    int status = RunRegisteredUnitTests();
    ChipLogProgress(NotSpecified, "CHIP test status: %d", status);

    return status;
}

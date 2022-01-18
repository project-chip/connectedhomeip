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

#include "netsocket/WiFiInterface.h"
#include <platform/mbed/Logging.h>
#include <support/UnitTestRegistration.h>
#include <support/logging/CHIPLogging.h>

using namespace ::chip;
using namespace ::chip::Logging::Platform;

#define concat_(x, y) x##y
#define concat(x, y) concat_(x, y)
#define SECURITY concat(NSAPI_SECURITY_, MBED_CONF_NSAPI_DEFAULT_WIFI_SECURITY)

int main()
{
    mbed_logging_init();

    ChipLogProgress(NotSpecified, "Mbed unit-tests application start");

    ChipLogProgress(NotSpecified, "Connect to the network...");
    auto interface = WiFiInterface::get_default_instance();
    if (interface != nullptr)
    {
        auto err = interface->connect(MBED_CONF_NSAPI_DEFAULT_WIFI_SSID, MBED_CONF_NSAPI_DEFAULT_WIFI_PASSWORD, SECURITY);
        if (err != NSAPI_ERROR_OK)
        {
            ChipLogProgress(NotSpecified, "WARNING: WiFi network connection failed [%d]", err);
        }
        else
        {
            auto connection_status = interface->get_connection_status();
            if (connection_status != NSAPI_STATUS_LOCAL_UP && connection_status != NSAPI_STATUS_GLOBAL_UP)
            {
                ChipLogProgress(NotSpecified, "WARNING: WiFi network connection failed. Connection status [%d]", connection_status);
            }
            else
            {
                ChipLogProgress(NotSpecified, "WiFi network connection success");
            }
        }
    }
    else
    {
        ChipLogProgress(NotSpecified, "WARNING: WiFi interface not available");
    }

    ChipLogProgress(NotSpecified, "Mbed unit-tests application run");

    int status = RunRegisteredUnitTests();
    ChipLogProgress(NotSpecified, "CHIP test status: %d", status);

    return status;
}

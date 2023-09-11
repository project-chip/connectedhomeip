/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "netsocket/WiFiInterface.h"
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/mbed/Logging.h>

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

/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>
#include <platform/CHIPDeviceLayer.h>

#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;

LOG_MODULE_REGISTER(runner, CONFIG_MATTER_LOG_LEVEL);

extern "C" void main(void)
{
    VerifyOrDie(settings_subsys_init() == 0);

    LOG_INF("Starting CHIP tests!");
    int status = RunRegisteredUnitTests();
    LOG_INF("CHIP test status: %d", status);

    exit(status);
}

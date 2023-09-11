/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <app/util/af.h>

#include <app/CommandHandler.h>
#include <app/util/attribute-storage.h>

using namespace chip;

void emberAfThermostatClusterClientInitCallback()
{
    // TODO
}
bool emberAfThermostatClusterCurrentWeeklyScheduleCallback(app::CommandHandler * commandObj, uint8_t, uint8_t, uint8_t, uint8_t *)
{
    // TODO
    return false;
}

bool emberAfThermostatClusterRelayStatusLogCallback(app::CommandHandler * commandObj, uint16_t, uint16_t, int16_t, uint8_t, int16_t,
                                                    uint16_t)
{
    // TODO
    return false;
}

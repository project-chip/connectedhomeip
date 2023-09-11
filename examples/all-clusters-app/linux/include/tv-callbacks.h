/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/****************************************************************************
 * @file
 * @brief Routines for TV stubs
 *server stub implementation of TV cluster code.
 *******************************************************************************
 ******************************************************************************/

#include <app/clusters/low-power-server/low-power-server.h>

class LowPowerManager : public chip::app::Clusters::LowPower::Delegate
{
public:
    bool HandleSleep() override;
};

/*
 *
 * SPDX-FileCopyrightText: 2022-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AppTask.h"
#include "binding-handler.h"

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init(void)
{
    InitCommonParts();

    // Configure Bindings
    CHIP_ERROR err = InitBindingHandlers();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("InitBindingHandlers fail");
        return err;
    }

    err = ConnectivityMgr().SetBLEDeviceName("TelinkApp");
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("SetBLEDeviceName fail");
        return err;
    }

    return CHIP_NO_ERROR;
}

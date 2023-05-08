/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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

#include "AppTask.h"

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init(void)
{
    InitCommonParts();

    CHIP_ERROR err = SensorMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("SensorMgr Init fail");
        return err;
    }
    err = TempMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("TempMgr Init fail");
        return err;
    }

    err = ConnectivityMgr().SetBLEDeviceName("TelinkThermo");
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("SetBLEDeviceName fail");
        return err;
    }

    return CHIP_NO_ERROR;
}

void AppTask::UpdateThermoStatUI(void)
{
    LOG_INF("Thermostat Status - M:%d T:%d'C H:%d'C C:%d'C", TempMgr().GetMode(), TempMgr().GetCurrentTemp(),
            TempMgr().GetHeatingSetPoint(), TempMgr().GetCoolingSetPoint());
}

void AppTask::UpdateClusterState() {}

/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "ElectricalGridConditionsMain.h"
#include "ElectricalGridConditionsDelegate.h"
/*
 *  @brief  Creates a Delegate and Instance for ElectricalGridConditions cluster
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalGridConditions;

static std::unique_ptr<ElectricalGridConditionsDelegate> gElectricalGridConditionsDelegate;
static std::unique_ptr<ElectricalGridConditionsInstance> gElectricalGridConditionsInstance;

ElectricalGridConditionsInstance * ElectricalGridConditions::GetElectricalGridConditionsInstance()
{
    return gElectricalGridConditionsInstance.get();
}

CHIP_ERROR ElectricalGridConditionsInit(EndpointId endpointId)
{
    CHIP_ERROR err;

    if (gElectricalGridConditionsDelegate || gElectricalGridConditionsInstance)
    {
        ChipLogError(AppServer, "Electrical Grid Conditions Instance or Delegate already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    gElectricalGridConditionsDelegate = std::make_unique<ElectricalGridConditionsDelegate>();
    if (!gElectricalGridConditionsDelegate)
    {
        ChipLogError(AppServer, "Failed to allocate memory for ElectricalGridConditionsDelegate");
        return CHIP_ERROR_NO_MEMORY;
    }
    /* Manufacturer may optionally not support all features, commands & attributes */
    gElectricalGridConditionsInstance = std::make_unique<ElectricalGridConditionsInstance>(
        EndpointId(endpointId), *gElectricalGridConditionsDelegate,
        BitMask<ElectricalGridConditions::Feature, uint32_t>(ElectricalGridConditions::Feature::kForecasting));

    if (!gElectricalGridConditionsInstance)
    {
        ChipLogError(AppServer, "Failed to allocate memory for ElectricalGridConditionsManager");
        gElectricalGridConditionsDelegate.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    err = gElectricalGridConditionsInstance->Init(); /* Register Attribute handlers */
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gElectricalGridConditionsInstance");
        gElectricalGridConditionsInstance.reset();
        gElectricalGridConditionsDelegate.reset();
        return err;
    }

    // Call example application initialisation to fill in required data
    err = gElectricalGridConditionsInstance->AppInit();

    return err;
}

CHIP_ERROR ElectricalGridConditionsShutdown()
{
    /* Do this in the order Instance first, then delegate
     * Ensure we call the Instance->Shutdown to free attribute & command handlers first
     */
    if (gElectricalGridConditionsInstance)
    {
        /* deregister attribute & command handlers */
        gElectricalGridConditionsInstance->Shutdown();
        gElectricalGridConditionsInstance.reset();
    }

    if (gElectricalGridConditionsDelegate)
    {
        gElectricalGridConditionsDelegate.reset();
    }

    return CHIP_NO_ERROR;
}

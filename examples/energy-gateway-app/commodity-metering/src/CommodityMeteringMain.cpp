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

#include "CommodityMeteringMain.h"
/*
 *  @brief  Creates an Instance for the Commodity Metering cluster
 *
 * The Instance is created and its Init() method is called to register
 * the attribute and command handlers.
 */

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityMetering;

static std::unique_ptr<Instance> gCommodityMeteringCluster;

Instance * CommodityMetering::GetCommodityMeteringInstance()
{
    return gCommodityMeteringCluster.get();
}

CHIP_ERROR CommodityMeteringInit(EndpointId endpointId)
{
    if (gCommodityMeteringCluster)
    {
        ChipLogError(AppServer, "Commodity Metering Instance already exists.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    /* Manufacturer may optionally not support all features, commands & attributes */
    gCommodityMeteringCluster = std::make_unique<Instance>(EndpointId(endpointId));

    if (!gCommodityMeteringCluster)
    {
        ChipLogError(AppServer, "Failed to allocate memory for Commodity Metering Instance");
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR err = gCommodityMeteringCluster->Init(); /* Register Attribute & Command handlers */
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gCommodityMeteringCluster");
        gCommodityMeteringCluster.reset();
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommodityMeteringShutdown()
{
    /* Do this in the order Instance first, then delegate
     * Ensure we call the Instance->Shutdown to free attribute & command handlers first
     */
    if (gCommodityMeteringCluster)
    {
        /* deregister attribute & command handlers */
        gCommodityMeteringCluster->Shutdown();
        gCommodityMeteringCluster.reset();
    }

    return CHIP_NO_ERROR;
}

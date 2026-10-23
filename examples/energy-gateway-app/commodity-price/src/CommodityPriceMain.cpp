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

#include "CommodityPriceMain.h"
#include "CommodityPriceDelegate.h"
/*
 *  @brief  Creates a Delegate and Instance for EVSE cluster
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityPrice;

static std::unique_ptr<CommodityPriceDelegate> gCommodityPriceDelegate;
static std::unique_ptr<CommodityPriceInstance> gCommodityPriceInstance;

CommodityPriceInstance * CommodityPrice::GetCommodityPriceInstance()
{
    return gCommodityPriceInstance.get();
}

CHIP_ERROR CommodityPriceInit(EndpointId endpointId)
{
    CHIP_ERROR err;

    if (gCommodityPriceDelegate || gCommodityPriceInstance)
    {
        ChipLogError(AppServer, "Commodity Price Instance or Delegate already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    gCommodityPriceDelegate = std::make_unique<CommodityPriceDelegate>();
    if (!gCommodityPriceDelegate)
    {
        ChipLogError(AppServer, "Failed to allocate memory for CommodityPriceDelegate");
        return CHIP_ERROR_NO_MEMORY;
    }
    /* Manufacturer may optionally not support all features, commands & attributes */
    gCommodityPriceInstance =
        std::make_unique<CommodityPriceInstance>(EndpointId(endpointId), *gCommodityPriceDelegate,
                                                 BitMask<CommodityPrice::Feature, uint32_t>(CommodityPrice::Feature::kForecasting));

    if (!gCommodityPriceInstance)
    {
        ChipLogError(AppServer, "Failed to allocate memory for CommodityPriceManager");
        gCommodityPriceDelegate.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    err = gCommodityPriceInstance->Init(); /* Register Attribute & Command handlers */
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gCommodityPriceInstance");
        gCommodityPriceInstance.reset();
        gCommodityPriceDelegate.reset();
        return err;
    }

    // Call example application initialisation to fill in required data
    err = gCommodityPriceInstance->AppInit();

    return err;
}

CHIP_ERROR CommodityPriceShutdown()
{
    /* Do this in the order Instance first, then delegate
     * Ensure we call the Instance->Shutdown to free attribute & command handlers first
     */
    if (gCommodityPriceInstance)
    {
        /* deregister attribute & command handlers */
        gCommodityPriceInstance->Shutdown();
        gCommodityPriceInstance.reset();
    }

    if (gCommodityPriceDelegate)
    {
        gCommodityPriceDelegate.reset();
    }

    return CHIP_NO_ERROR;
}

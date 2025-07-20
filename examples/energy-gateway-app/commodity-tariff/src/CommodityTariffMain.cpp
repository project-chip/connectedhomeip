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

#include <CommodityTariffMain.h>
#include <fstream>

/*
 *  @brief  Creates a Delegate and Instance for Commodity Tariff cluster
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityTariff;

static std::unique_ptr<CommodityTariffInstance> gCommodityTariffInstance;
static std::unique_ptr<CommodityTariffDelegate> gCommodityTariffDelegate;

CommodityTariffInstance * CommodityTariff::GetCommodityTariffInstance()
{
    return gCommodityTariffInstance.get();
}

CommodityTariffDelegate * CommodityTariff::GetCommodityTariffDelegate()
{
    CommodityTariffInstance * mInst = GetCommodityTariffInstance();
    VerifyOrDieWithMsg(mInst != nullptr, AppServer, "CommodityTariffInstance is null");
    CommodityTariffDelegate * dg = mInst->GetDelegate();
    VerifyOrDieWithMsg(dg != nullptr, AppServer, "CommodityTariffDelegate is null");

    return dg;
}

static bool parseFromConstant(const char * aJsonString, Json::Value & aRoot, std::string & aErrorMsg)
{
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;

    if (aJsonString == nullptr)
    {
        aErrorMsg = "Null JSON string provided";
        return false;
    }

    std::istringstream jsonStream(aJsonString);
    bool success = Json::parseFromStream(builder, jsonStream, &aRoot, &errs);
    if (!success)
    {
        aErrorMsg = "Failed to parse JSON: " + errs;
    }

    return success;
}

void LoadTariffFromJSONString(const char * aJsonStringPreset, CommodityTariffDelegate * dg)
{
    Json::Value json_root;
    std::string errs;
    if (parseFromConstant(aJsonStringPreset, json_root, errs))
    {
        ChipLogProgress(NotSpecified, "The tariff data loaded successfully. Tariff name: %s",
                        json_root["TariffInfo"]["TariffLabel"].asString().c_str());
        if (CHIP_NO_ERROR == dg->LoadTariffData(json_root))
        {
            dg->TariffDataUpdate();
        }
    }
    else
    {
        ChipLogError(NotSpecified, "Unable to load tariff data, err: %s", errs.c_str());
    }
}

CHIP_ERROR CommodityTariffInit(EndpointId endpointId)
{
    CHIP_ERROR err;

    if (gCommodityTariffInstance || gCommodityTariffDelegate)
    {
        ChipLogError(AppServer, "Commodity Tariff Instance or Delegate already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    gCommodityTariffDelegate = std::make_unique<CommodityTariffDelegate>();
    if (!gCommodityTariffDelegate)
    {
        ChipLogError(AppServer, "Failed to allocate memory for CommodityTariffInstance");
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Manufacturer may optionally not support all features, commands & attributes */
    gCommodityTariffInstance = std::make_unique<CommodityTariffInstance>(
        EndpointId(endpointId), *gCommodityTariffDelegate,
        BitMask<CommodityTariff::Feature, uint32_t>(CommodityTariff::Feature::kPricing, CommodityTariff::Feature::kFriendlyCredit,
                                                    CommodityTariff::Feature::kAuxiliaryLoad, CommodityTariff::Feature::kPeakPeriod,
                                                    CommodityTariff::Feature::kPowerThreshold,
                                                    CommodityTariff::Feature::kRandomization));

    if (!gCommodityTariffInstance)
    {
        ChipLogError(AppServer, "Failed to allocate memory for CommodityTariffInstance");
        gCommodityTariffInstance.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    err = gCommodityTariffInstance->Init(); /* Register Attribute & Command handlers */
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gCommodityTariffInstance");
        gCommodityTariffInstance.reset();
        gCommodityTariffDelegate.reset();
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommodityTariffShutdown()
{
    /* Do this in the order Instance first, then delegate
     * Ensure we call the Instance->Shutdown to free attribute & command handlers first
     */
    if (gCommodityTariffInstance)
    {
        /* deregister attribute & command handlers */
        gCommodityTariffInstance->Shutdown();
        gCommodityTariffInstance.reset();
    }

    if (gCommodityTariffDelegate)
    {
        gCommodityTariffDelegate.reset();
    }

    return CHIP_NO_ERROR;
}

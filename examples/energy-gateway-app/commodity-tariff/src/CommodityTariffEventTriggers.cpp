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
#include <app/clusters/commodity-tariff-server/CommodityTariffTestEventTriggerHandler.h>
#include <app/util/af-types.h>

#include <fstream>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityTariff;
using namespace chip::app::Clusters::CommodityTariff::Structs;

static constexpr const char * default_tariff_data = "./DefaultTariff.json";
//static constexpr const char * tariff_data_1 ="./full_complex_tariff_1.json";
//static constexpr const char * tariff_data_2 ="./full_complex_tariff_2.json";

static bool LoadJsonFile(const char * aFname, Json::Value &jsonValue)
{
    bool is_ok = false;
    std::ifstream ifs;    
    Json::CharReaderBuilder builder;
    Json::String errs;

    ifs.open(aFname);

    if (!ifs.good())
    {
        ChipLogError(NotSpecified,
             "AllClusters App: Error open file %s", aFname);
        goto exit;
    }

    if (!parseFromStream(builder, ifs, &jsonValue, &errs)) {
        ChipLogError(NotSpecified,
             "AllClusters App: Error parsing JSON file %s with error %s:", aFname, errs.c_str());
        goto exit;
    }

    if (jsonValue.empty() || !jsonValue.isObject())
    {
        ChipLogError(NotSpecified, "Invalid file format %s", aFname);
        goto exit;
    }

    is_ok = true;

exit:
    return is_ok;
}

void SetTestEventTrigger_TariffDataUpdated()
{
   CommodityTariffDelegate * dg = GetCommodityTariffDelegate();
   Json::Value json_root;
   if ( LoadJsonFile(default_tariff_data, json_root) )
   {
       ChipLogProgress(NotSpecified, "The default tariff file opened successfully");
       dg->LoadTariffData(json_root);     
   }
   else
   {
       ChipLogError(NotSpecified, "Unable to load default tariff file");
   }
}

void SetTestEventTrigger_TariffDataClear()
{
    // TODO
}
void SetTestEventTrigger_ForcedOneDayForward()
{
    // TODO
}

bool HandleCommodityTariffTestEventTrigger(uint64_t eventTrigger)
{
    CommodityTariffTrigger trigger = static_cast<CommodityTariffTrigger>(eventTrigger);

    switch (trigger)
    {
    case CommodityTariffTrigger::kTariffDataUpdated:
        ChipLogProgress(Support, "[CommodityTariff-Test-Event] => Tariff Data Updated");
        SetTestEventTrigger_TariffDataUpdated();
        break;
   case CommodityTariffTrigger::kTariffDataClear:
        ChipLogProgress(Support, "[CommodityTariff-Test-Event] => Tariff Data Clear");
        SetTestEventTrigger_TariffDataClear();
        break;
    case CommodityTariffTrigger::kForcedOneDayForward:
        ChipLogProgress(Support, "[CommodityTariff-Test-Event] => Forced OneDay Forward");
        SetTestEventTrigger_ForcedOneDayForward();
        break;

    default:
        return false;
    }

    return true;
}

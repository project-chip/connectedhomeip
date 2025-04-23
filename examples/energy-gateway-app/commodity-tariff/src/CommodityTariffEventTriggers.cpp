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
 
 using namespace chip;
 using namespace chip::app;
 using namespace chip::app::DataModel;
 using namespace chip::app::Clusters;
 using namespace chip::app::Clusters::CommodityTariff;
 using namespace chip::app::Clusters::CommodityTariff::Structs;
 
 /*CommodityTariffDelegate * GetCommodityTariffDelegate()
 {
     CommodityTariffInstance * mInst = GetCommodityTariffInstance();
     VerifyOrDieWithMsg(mInst != nullptr, AppServer, "CommodityTariffInstance is null");
     CommodityTariffDelegate * dg = mInst->GetDelegate();
     VerifyOrDieWithMsg(dg != nullptr, AppServer, "CommodityTariffDelegate is null");
 
     return dg;
 }*/
 
 void SetTestEventTrigger_TariffDataUpdated()
 {
     // TODO
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
 
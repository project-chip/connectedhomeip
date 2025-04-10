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

 #include <CommodityTariffDelegate.h>
 #include <app/EventLogging.h>
 #include <app/reporting/reporting.h>
 
 #include <app/clusters/commodity-tariff-server/commodity-tariff-server.h>
 
 using namespace chip;
 using namespace chip::app;
 using namespace chip::app::DataModel;
 using namespace chip::app::Clusters;
 using namespace chip::app::Clusters::Globals;
 using namespace chip::app::Clusters::Globals::Structs;
 using namespace chip::app::Clusters::CommodityTariff;
 using namespace chip::app::Clusters::CommodityTariff::Attributes;
 using namespace chip::app::Clusters::CommodityTariff::Structs;
 
 using chip::Protocols::InteractionModel::Status;
 
 CHIP_ERROR CommodityTariffInstance::Init()
 {
     return Instance::Init();
 }
 
 void CommodityTariffInstance::Shutdown()
 {
     Instance::Shutdown();
 }
 /*
 CommodityTariffDelegate::CommodityTariffDelegate()
 {
    // TODO - set default values for attributes 
 }
 
 CommodityTariffDelegate::~CommodityTariffDelegate()
 {
    // TODO - free allocated space
 }
*/

Status CommodityTariffDelegate::GetDayEntryById(DataModel::Nullable<uint32_t> aDayEntryId, Structs::DayEntryStruct::Type & aDayEntry)
{
    return Status::Success;
}

Status CommodityTariffDelegate::GetTariffComponentInfoById(DataModel::Nullable<uint32_t>  aTariffComponentId,
                                                           DataModel::Nullable<chip::CharSpan> & label,
                                                           DataModel::List<const uint32_t> & dayEntryIDs,
                                                           Structs::TariffComponentStruct::Type & aTariffComponent)
{
    return Status::Success;
}

 // --------------- Internal Attribute Set APIs
 /*
 CHIP_ERROR SetTariffInfo(const DataModel::Nullable<Structs::TariffInformationStruct::Type>& newValue)
{
    bool is_changed = false;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO - some value change logic



    if ( (err == CHIP_NO_ERROR) && is_changed)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, TariffInfo::Id;);
    }

    return err;
}


CHIP_ERROR SetTariffUnit(Globals::TariffUnitEnum newValue);
{
    bool is_changed = false;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO - some value change logic

    if ( (err == CHIP_NO_ERROR) && is_changed)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, TariffInfo::Id);
    }

    return err;
}

CHIP_ERROR SetStartDate(const DataModel::Nullable<epoch_s> newValue)
{
    bool is_changed = false;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO - some value change logic

    if ( (err == CHIP_NO_ERROR) && is_changed)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, StartDate::Id);
    }

    return err;
}

CHIP_ERROR SetDefaultRandomizationOffset(const DataModel::Nullable<int16_t> newValue)
{
    bool is_changed = false;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO - some value change logic

    if ( (err == CHIP_NO_ERROR) && is_changed)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, DefaultRandomizationOffset::Id);
    }

    return err;
}

CHIP_ERROR SetDefaultRandomizationType(const DataModel::Nullable<DayEntryRandomizationTypeEnum> newValue)
{
    bool is_changed = false;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO - some value change logic

    if ( (err == CHIP_NO_ERROR) && is_changed)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, DefaultRandomizationType::Id);
    }

    return err;
}

// List Setters (replace entire lists)
CHIP_ERROR SetCalendarPeriods(const DataModel::List<Structs::CalendarPeriodStruct::Type>* newList)
{
    bool is_changed = false;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO - some value change logic

    if ( (err == CHIP_NO_ERROR) && is_changed)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, CalendarPeriods::Id);
    }

    return err;
}

CHIP_ERROR SetDayPatterns(const DataModel::List<Structs::DayPatternStruct::Type>*);
CHIP_ERROR SetIndividualDays(const DataModel::List<Structs::DayStruct::Type>*);
CHIP_ERROR SetDayEntries(const DataModel::List<Structs::DayEntryStruct::Type>*);
CHIP_ERROR SetTariffPeriods(const DataModel::List<Structs::TariffPeriodStruct::Type>*);
CHIP_ERROR SetTariffComponents(const DataModel::List<Structs::TariffComponentStruct::Type>*);

// Secondary Attributes
CHIP_ERROR SetCurrentDay(const DataModel::Nullable<Structs::DayStruct::Type>*);
CHIP_ERROR SetNextDay(const DataModel::Nullable<Structs::DayStruct::Type>*);
CHIP_ERROR SetCurrentDayEntry(const DataModel::Nullable<Structs::DayEntryStruct::Type>*);
CHIP_ERROR SetNextDayEntry(const DataModel::Nullable<Structs::DayEntryStruct::Type>*);
CHIP_ERROR SetCurrentDayEntryDate(const DataModel::Nullable<epoch_s>);
CHIP_ERROR SetNextDayEntryDate(const DataModel::Nullable<epoch_s>);
CHIP_ERROR SetCurrentTariffComponents(const DataModel::List<Structs::TariffComponentStruct::Type>*);
CHIP_ERROR SetNextTariffComponents(const DataModel::List<Structs::TariffComponentStruct::Type>*);
*/
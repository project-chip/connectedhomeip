/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "commodity-tariff-server.h"

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityTariff;
using namespace chip::app::Clusters::CommodityTariff::Attributes;

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityTariff {

void LockThreadTask(void)
{

}

void UnlockThreadTask(void)
{

}

bool CommodityTariffDataProvider::TariffDataUpd_Init(const CommodityTariffPrimaryData& aNewData)
{
        CHIP_ERROR err = CHIP_NO_ERROR;

#define X(attrName, attrType) \
        err = attrName##_MgmtObj.UpdateBegin(aNewData.m##attrName, mFeature);
COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X
/*
#define X(attrName, attrType) \
        if (!attrName##_MgmtObj.IsValid()) { \
            ChipLogProgress(NotSpecified, "EGW-CTC: New value for attribute " #attrName " (Id %d) is invalid", Attributes::attrName::Id); \
            allValid = false; \
        }
    COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X
*/
        if (err != CHIP_NO_ERROR)
        {
            return false;
        }

        return true;
}

void CommodityTariffDataProvider::TariffDataUpd_Commit()
{
#define X(attrName, attrType) \
    attrName##_MgmtObj.UpdateCommit();
COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X

#define X(attrName, attrType) \
    if (attrName##_MgmtObj.HasChanged()) { \
        ChipLogProgress(NotSpecified, "EGW-CTC: New value for attribute " #attrName " (Id %d) updated", Attributes::attrName::Id); \
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, Attributes::attrName::Id); \
    }
    COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X
}

void CommodityTariffDataProvider::TariffDataUpd_Abort()
{
#define X(attrName, attrType) \
        attrName##_MgmtObj.UpdateAbort();
COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X
}

bool CommodityTariffDataProvider::TariffDataUpd_Validator()
{
    if (!TariffInfo_MgmtObj.GetValue().IsNull())
    {
        return false;
    }

    //CheckDayPatterns
    //CheckDayEntries
    //CheckTariffPeriods    
    //CheckTariffComponents

    //CheckIndividualDays
    //CalendarPeriods

    return true;
}

void CommodityTariffDataProvider::TariffDataUpdate(const CommodityTariffPrimaryData& newData)
{
    if (!TariffDataUpd_Init(newData))
    {
        ChipLogError(NotSpecified, "EGW-CTC: New tariff data rejected due to internal inconsistencies");
    }
    else if (!TariffDataUpd_Validator())
    {
        ChipLogError(NotSpecified, "EGW-CTC: New tariff data rejected due to some cross-fields inconsistencies");
    }
    else
    {
        TariffDataUpd_Commit();
        ChipLogProgress(NotSpecified, "EGW-CTC: Tariff data applied");
        return;
    }
    TariffDataUpd_Abort();
}


CHIP_ERROR CommodityTariffServer::Init()
{
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

void CommodityTariffServer::Shutdown()
{
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

bool CommodityTariffServer::HasFeature(Feature aFeature) const
{
    return mFeature.Has(aFeature);
}

// AttributeAccessInterface
CHIP_ERROR CommodityTariffServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case TariffInfo::Id:
        return aEncoder.Encode(mProvider.GetTariffInfo());
    case TariffUnit::Id:
        return aEncoder.Encode(mProvider.GetTariffUnit());
    case StartDate::Id:
        return aEncoder.Encode(mProvider.GetStartDate());
    case DayEntries::Id:
        return aEncoder.Encode(mProvider.GetDayEntries());
    case DayPatterns::Id:
        return aEncoder.Encode(mProvider.GetDayPatterns());
    case CalendarPeriods::Id:
        return aEncoder.Encode(mProvider.GetCalendarPeriods());
    case IndividualDays::Id:
        return aEncoder.Encode(mProvider.GetIndividualDays());
    case CurrentDay::Id:
        return aEncoder.Encode(mProvider.GetCurrentDay());
    case NextDay::Id:
        return aEncoder.Encode(mProvider.GetNextDay());
    case CurrentDayEntry::Id:
        return aEncoder.Encode(mProvider.GetCurrentDayEntry());
    case CurrentDayEntryDate::Id:
        return aEncoder.Encode(mProvider.GetCurrentDayEntryDate());
    case NextDayEntry::Id:
        return aEncoder.Encode(mProvider.GetNextDayEntry());
    case NextDayEntryDate::Id:
        return aEncoder.Encode(mProvider.GetNextDayEntryDate());
    case TariffComponents::Id:
        return aEncoder.Encode(mProvider.GetTariffComponents());
    case TariffPeriods::Id:
        return aEncoder.Encode(mProvider.GetTariffPeriods());
    case CurrentTariffComponents::Id:
        return aEncoder.Encode(mProvider.GetCurrentTariffComponents());
    case NextTariffComponents::Id:
        return aEncoder.Encode(mProvider.GetNextTariffComponents());
    case DefaultRandomizationOffset::Id:
        if (!HasFeature(Feature::kRandomization))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return aEncoder.Encode(mProvider.GetDefaultRandomizationOffset());
    case DefaultRandomizationType::Id:
        if (!HasFeature(Feature::kRandomization))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return aEncoder.Encode(mProvider.GetDefaultRandomizationType());

    /* FeatureMap - is held locally */
    case FeatureMap::Id:
        return aEncoder.Encode(mFeature);
    }

    /* Allow all other unhandled attributes to fall through to Ember */
    return CHIP_NO_ERROR;
}

void CommodityTariffServer::InvokeCommand(HandlerContext & handlerContext)
{
    using namespace Commands;

    switch (handlerContext.mRequestPath.mCommandId)
    {
    case GetTariffComponent::Id:
        HandleCommand<GetTariffComponent::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleGetTariffComponent(ctx, commandData); });
        return;        
    case GetDayEntry::Id:
        HandleCommand<GetDayEntry::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleGetDayEntry(ctx, commandData); });
        return;
    }
}

void CommodityTariffServer::HandleGetTariffComponent(HandlerContext & ctx, const Commands::GetTariffComponent::DecodableType & commandData)
{
    Commands::GetTariffComponentResponse::Type response;

    Status status = mProvider.GetTariffComponentInfoById(commandData.tariffComponentID, 
                                                         response.label,
                                                         response.dayEntryIDs,
                                                         response.tariffComponent);
    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void CommodityTariffServer::HandleGetDayEntry(HandlerContext & ctx, const Commands::GetDayEntry::DecodableType & commandData)
{
    Commands::GetDayEntryResponse::Type response;

    Status status = mProvider.GetDayEntryById(commandData.dayEntryID, response.dayEntry);
    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterCommodityTariffPluginServerInitCallback() {}

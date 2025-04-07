/*
 *    Copyright (c) 2023 Project CHIP Authors
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

CHIP_ERROR Instance::Init()
{
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

bool Instance::HasFeature(Feature aFeature) const
{
    return mFeature.Has(aFeature);
}

// AttributeAccessInterface
CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case TariffInfo::Id:
        return aEncoder.Encode(mDelegate.GetTariffInfo());
    case TariffUnit::Id:
        return aEncoder.Encode(mDelegate.GetTariffUnit());
    case StartDate::Id:
        return aEncoder.Encode(mDelegate.GetStartDate());
    case DayEntries::Id:
        return aEncoder.Encode(mDelegate.GetDayEntries());
    case DayPatterns::Id:
        return aEncoder.Encode(mDelegate.GetDayPatterns());
    case CalendarPeriods::Id:
        return aEncoder.Encode(mDelegate.GetCalendarPeriods());
    case IndividualDays::Id:
        return aEncoder.Encode(mDelegate.GetIndividualDays());
    case CurrentDay::Id:
        return aEncoder.Encode(mDelegate.GetCurrentDay());
    case NextDay::Id:
        return aEncoder.Encode(mDelegate.GetNextDay());
    case CurrentDayEntry::Id:
        return aEncoder.Encode(mDelegate.GetCurrentDayEntry());
    case CurrentDayEntryDate::Id:
        return aEncoder.Encode(mDelegate.GetCurrentDayEntryDate());
    case NextDayEntry::Id:
        return aEncoder.Encode(mDelegate.GetNextDayEntry());
    case NextDayEntryDate::Id:
        return aEncoder.Encode(mDelegate.GetNextDayEntryDate());
    case TariffComponents::Id:
        return aEncoder.Encode(mDelegate.GetTariffComponents());
    case TariffPeriods::Id:
        return aEncoder.Encode(mDelegate.GetTariffPeriods());
    case CurrentTariffComponents::Id:
        return aEncoder.Encode(mDelegate.GetCurrentTariffComponents());
    case NextTariffComponents::Id:
        return aEncoder.Encode(mDelegate.GetNextTariffComponents());
    case DefaultRandomizationOffset::Id:
        if (!HasFeature(Feature::kRandomization))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return aEncoder.Encode(mDelegate.GetDefaultRandomizationOffset());
    case DefaultRandomizationType::Id:
        if (!HasFeature(Feature::kRandomization))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return aEncoder.Encode(mDelegate.GetDefaultRandomizationType());

    /* FeatureMap - is held locally */
    case FeatureMap::Id:
        return aEncoder.Encode(mFeature);
    }

    /* Allow all other unhandled attributes to fall through to Ember */
    return CHIP_NO_ERROR;
}

void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    using namespace Commands;

    switch (handlerContext.mRequestPath.mCommandId)
    {
    case GetDayEntry::Id:
        HandleCommand<GetDayEntry::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleGetDayEntry(ctx, commandData); });
        return;
    case GetTariffComponent::Id:
        HandleCommand<GetTariffComponent::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleGetTariffComponent(ctx, commandData); });
        return;
    }
}

void Instance::HandleGetDayEntry(HandlerContext & ctx, const Commands::GetDayEntry::DecodableType & commandData)
{
    Commands::GetDayEntryResponse::Type response;

    Status status = mDelegate.GetDayEntryById(commandData.dayEntryID, response.dayEntry);
    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}

void Instance::HandleGetTariffComponent(HandlerContext & ctx, const Commands::GetTariffComponent::DecodableType & commandData)
{
    Commands::GetTariffComponentResponse::Type response;

    Status status = mDelegate.GetTariffComponentInfoById(commandData.tariffComponentID, 
                                                         response.label,
                                                         response.dayEntryIDs,
                                                         response.tariffComponent);
    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterCommodityTariffPluginServerInitCallback() {}

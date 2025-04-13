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

#include "commodity-price-server.h"

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityPrice;
using namespace chip::app::Clusters::CommodityPrice::Attributes;

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityPrice {

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
    case TariffUnit::Id:
        return aEncoder.Encode(mDelegate.GetTariffUnit());
    case Currency::Id:
        return aEncoder.Encode(mDelegate.GetCurrency());
    case CurrentPrice::Id:
        return aEncoder.Encode(mDelegate.GetCurrentPrice());
    case PriceForecast::Id:
        /* FORE - Forecasting */
        if (!HasFeature(Feature::kForecasting))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return aEncoder.Encode(mDelegate.GetPriceForecast(static_cast<CommodityPriceDetailBitmap>(0x0))); // We don't want the description or the

    /* FeatureMap - is held locally */
    case FeatureMap::Id:
        return aEncoder.Encode(mFeature);
    }

    /* Allow all other unhandled attributes to fall through to Ember */
    return CHIP_NO_ERROR;
}

// CommandHandlerInterface
CHIP_ERROR Instance::EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)
{
    using namespace Commands;

    VerifyOrExit(callback(GetDetailedPriceRequest::Id, context) == Loop::Continue, /**/);

    if (HasFeature(Feature::kForecasting))
    {
        VerifyOrExit(callback(GetDetailedForecastRequest::Id, context) == Loop::Continue, /**/);
    }

exit:
    return CHIP_NO_ERROR;
}

void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    using namespace Commands;

    switch (handlerContext.mRequestPath.mCommandId)
    {
    case GetDetailedPriceRequest::Id:
        HandleCommand<GetDetailedPriceRequest::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleGetDetailedPriceRequest(ctx, commandData); });
        return;
    case GetDetailedForecastRequest::Id:
        if (!HasFeature(Feature::kForecasting))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<GetDetailedForecastRequest::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleGetDetailedForecastRequest(ctx, commandData); });
        }
        return;
    }
}

void Instance::HandleGetDetailedPriceRequest(HandlerContext & ctx,
                                             const Commands::GetDetailedPriceRequest::DecodableType & commandData)
{
    //    bool validArgs = false;

    //    map16 details                       = commandData.details;

    // TODO finish off
}

void Instance::HandleGetDetailedForecastRequest(HandlerContext & ctx,
                                                const Commands::GetDetailedForecastRequest::DecodableType & commandData)
{
    if (!HasFeature(Feature::kForecasting))
    {
        ChipLogError(Zcl, "GetDetailedForecastRequest not supported");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    // TODO finish off
}

} // namespace CommodityPrice
} // namespace Clusters
} // namespace app
} // namespace chip

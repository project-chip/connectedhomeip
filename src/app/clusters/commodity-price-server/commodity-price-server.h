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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/StatusIB.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityPrice {

class Delegate
{
public:
    virtual ~Delegate() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }

    // ------------------------------------------------------------------
    // Get attribute methods
    virtual Globals::TariffUnitEnum GetTariffUnit()                                            = 0;
    virtual Globals::Structs::CurrencyStruct::Type GetCurrency()                               = 0;
    virtual const DataModel::Nullable<Structs::CommodityPriceStruct::Type> & GetCurrentPrice() = 0;
    virtual int64_t GetPriceForecast()                                                         = 0;
    // TODO GetPriceForecast returns a List of CommodityPriceStruct with simple details, unlike the command which can ask for
    // specific details.

    /**
     * @brief Returns the current Forecast object
     *
     * The reference returned from GetForecast() is only valid until the next Matter event
     * is processed.  Callers must not hold on to that reference for any asynchronous processing.
     *
     * Once another Matter event has had a chance to run, the memory associated with the
     * ForecastStruct is likely to change or be re-allocated, so would become invalid.
     *
     * @return  The current Forecast object
     */
    // virtual const DataModel::Nullable<Structs::CommodityPriceStruct::Type> & GetDetailedForecast() = 0;

protected:
    EndpointId mEndpointId = 0;
};

class Instance : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    Instance(EndpointId aEndpointId, Delegate & aDelegate, Feature aFeature) :
        AttributeAccessInterface(MakeOptional(aEndpointId), Id), CommandHandlerInterface(MakeOptional(aEndpointId), Id),
        mDelegate(aDelegate), mFeature(aFeature)
    {
        /* set the base class delegates endpointId */
        mDelegate.SetEndpointId(aEndpointId);
    }

    ~Instance() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;

private:
    Protocols::InteractionModel::Status GetMatterEpochTimeFromUnixTime(uint32_t & currentUtcTime) const;

private:
    Delegate & mDelegate;
    BitMask<Feature> mFeature;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    // NOTE there are no writable attributes

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & handlerContext) override;
    CHIP_ERROR EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context) override;

    void HandleGetDetailedPriceRequest(HandlerContext & ctx, const Commands::GetDetailedPriceRequest::DecodableType & commandData);
    void HandleGetDetailedForecastRequest(HandlerContext & ctx,
                                          const Commands::GetDetailedForecastRequest::DecodableType & commandData);
};

} // namespace CommodityPrice
} // namespace Clusters
} // namespace app
} // namespace chip

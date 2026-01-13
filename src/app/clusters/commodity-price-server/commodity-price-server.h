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

// Spec-defined constraints
constexpr uint8_t kMaxForecastEntries         = 56;
constexpr uint8_t kMaxDescriptionLength       = 32;
constexpr uint8_t kMaxComponentsPerPriceEntry = 10;

constexpr uint16_t kMaxCurrencyValue = 999; // From spec
// From ISO 4217 (non exhaustive selection)
constexpr uint16_t kCurrencyCHF  = 756;
constexpr uint16_t kCurrencyEURO = 978;
constexpr uint16_t kCurrencyGBP  = 826;
constexpr uint16_t kCurrencyNOK  = 578;
constexpr uint16_t kCurrencySEK  = 752;
constexpr uint16_t kCurrencyUSD  = 840;

constexpr bool operator!=(const Globals::Structs::CurrencyStruct::Type & lhs, const Globals::Structs::CurrencyStruct::Type & rhs)
{
    return ((lhs.currency != rhs.currency) || (lhs.decimalPoints != rhs.decimalPoints));
}

class Delegate
{
public:
    virtual ~Delegate() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }

    // For now this is a place holder and the Delegate could be removed
    // There are no delegated methods since these are largely implemented in the
    // commodity-price-server.cpp Instance class

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
        mEndpointId = aEndpointId;
    }

    ~Instance() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;

    // Set attribute methods
    CHIP_ERROR SetTariffUnit(Globals::TariffUnitEnum);
    CHIP_ERROR SetCurrency(Globals::Structs::CurrencyStruct::Type);
    CHIP_ERROR SetCurrentPrice(const DataModel::Nullable<Structs::CommodityPriceStruct::Type>);
    CHIP_ERROR SetForecast(const DataModel::List<const Structs::CommodityPriceStruct::Type> &);

    // Send Price Change events
    Protocols::InteractionModel::Status GeneratePriceChangeEvent();

private:
    Delegate & mDelegate;
    BitMask<Feature> mFeature;

    EndpointId mEndpointId;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    // NOTE there are no writable attributes

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & handlerContext) override;
    CHIP_ERROR RetrieveAcceptedCommands(const ConcreteClusterPath & cluster,
                                        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    void HandleGetDetailedPriceRequest(HandlerContext & ctx, const Commands::GetDetailedPriceRequest::DecodableType & commandData);
    void HandleGetDetailedForecastRequest(HandlerContext & ctx,
                                          const Commands::GetDetailedForecastRequest::DecodableType & commandData);

    // Helper function to create a copy of the data for sending to client with the .description or .components knocked out
    CHIP_ERROR GetDetailedPriceRequest(BitMask<CommodityPriceDetailBitmap> details,
                                       DataModel::Nullable<Structs::CommodityPriceStruct::Type> &);

    CHIP_ERROR GetDetailedForecastRequest(BitMask<CommodityPriceDetailBitmap> details,
                                          Platform::ScopedMemoryBuffer<Structs::CommodityPriceStruct::Type> & forecastBuffer,
                                          DataModel::List<const Structs::CommodityPriceStruct::Type> & forecastList,
                                          bool isCommand);

    // Attribute storage
    Globals::TariffUnitEnum mTariffUnit;
    Globals::Structs::CurrencyStruct::Type mCurrency;

    // NOTE the CurrentPrice and PriceForecast are stored here with description and components
    // When read as an attribute the description and components should not be included
    // When the GetDetailedPriceRequest or GetDetailedForecastRequest are called these may need
    // munging to remove one or other elements
    DataModel::Nullable<Structs::CommodityPriceStruct::Type> mCurrentPrice;
    DataModel::List<const Structs::CommodityPriceStruct::Type> mPriceForecast;

    // This is the cluster server's backing store for mCurrentPrice .components and .descriptions
    Platform::ScopedMemoryBuffer<Structs::CommodityPriceStruct::Type> mOwnedCurrentPriceStructBuffer;
    Platform::ScopedMemoryBuffer<Structs::CommodityPriceComponentStruct::Type> mOwnedCurrentPriceComponentBuffer;
    // each component has an optional Description
    Platform::ScopedMemoryBuffer<char> mOwnedCurrentPriceComponentDescriptionBuffer[kMaxComponentsPerPriceEntry];
    Platform::ScopedMemoryBuffer<char> mOwnedCurrentPriceDescriptionBuffer;

    // Helper function that makes a copy of a string into a span
    CHIP_ERROR CopyCharSpan(const CharSpan src, Platform::ScopedMemoryBuffer<char> & bufferOut, CharSpan & spanOut);

    // This performs a deep copy into mCurrentPrice so that the caller of the SetCurrentPrice can free its memory
    CHIP_ERROR CopyPrice(const DataModel::Nullable<Structs::CommodityPriceStruct::Type> & src);

    // This performs a deep copy into mPriceForecast so that the caller of SetPriceForecast can free its memory
    CHIP_ERROR CopyPriceForecast(const DataModel::List<const Structs::CommodityPriceStruct::Type> & src);

    // Helper function to clear buffer storage
    void CheckAndFreeForecastBuffers(void);
    CHIP_ERROR CopyPriceStructWithinForecast(
        Structs::CommodityPriceStruct::Type & destPriceStruct, Platform::ScopedMemoryBuffer<char> & dest_descriptionBuffer,
        Platform::ScopedMemoryBuffer<Structs::CommodityPriceComponentStruct::Type> & dest_componentsBuffer,
        Platform::ScopedMemoryBuffer<char> * dest_componentsDescriptionBuffer, const Structs::CommodityPriceStruct::Type & src);

    // This is the cluster server's backing store for mForecast (list of CommodityPriceStructs) each with .components and
    // .descriptions
    Platform::ScopedMemoryBuffer<Structs::CommodityPriceStruct::Type> mOwnedForecastPriceStructBuffer;
    Platform::ScopedMemoryBuffer<Structs::CommodityPriceComponentStruct::Type>
        mOwnedForecastPriceComponentBuffer[kMaxForecastEntries];
    // each component has an optional Description
    Platform::ScopedMemoryBuffer<char> mOwnedForecastPriceComponentDescriptionBuffer[kMaxForecastEntries]
                                                                                    [kMaxComponentsPerPriceEntry];
    Platform::ScopedMemoryBuffer<char> mOwnedForecastPriceDescriptionBuffer[kMaxForecastEntries];
};

} // namespace CommodityPrice
} // namespace Clusters
} // namespace app
} // namespace chip

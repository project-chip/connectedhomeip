/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "PowerSourceCluster.h"

#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/PowerSource/Metadata.h>

using namespace chip;
using namespace app;
using namespace app::Clusters;

namespace {
template <typename T>
inline CHIP_ERROR EncodeValue(AttributeValueEncoder & encoder, const T & value, CHIP_ERROR err)
{
    if (err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        return encoder.EncodeNull();
    }
    if (err != CHIP_NO_ERROR)
    {
        return encoder.EncodeNull();
    }
    return encoder.Encode(value);
}
template <typename T>
inline CHIP_ERROR EncodeOptional(AttributeValueEncoder & encoder, const ConstexprIntegralOptional<T> & value, CHIP_ERROR err = CHIP_NO_ERROR)
{
    if (err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        return encoder.EncodeNull();
    }
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    if (!value.HasValue())
    {
        return encoder.EncodeNull();
    }
    return encoder.Encode(value.value);
}

template <typename T>
inline CHIP_ERROR EncodeListOfValues(AttributeValueEncoder & encoder, const T & valueList, CHIP_ERROR err = CHIP_NO_ERROR)
{
    if (err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        return encoder.EncodeEmptyList();
    }
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    return encoder.EncodeList([&valueList](const auto & enc) -> CHIP_ERROR {
        for (const auto & value : valueList)
        {
            if constexpr (std::is_enum_v<std::remove_cv_t<std::remove_reference_t<decltype(value)>>>)
            {
                ReturnErrorOnFailure(enc.Encode(to_underlying(value)));
            }
            else
            {
                ReturnErrorOnFailure(enc.Encode(value));
            }
        }

        return CHIP_NO_ERROR;
    });
}
} // anonymous namespace

namespace chip {
namespace app {
namespace Clusters {

using namespace PowerSource;
using namespace PowerSource::Attributes;
using namespace PowerSource::Events;

CHIP_ERROR WiredPowerSourceCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    // the `Order` attribute is marked as `Persistent`.
    VerifyOrReturnValue(mConfig.orderFetchFromPersistentStorageDuringStartup, CHIP_NO_ERROR);

    AttributePersistence attributePersistence(context.attributeStorage);
    attributePersistence.LoadNativeEndianValue<uint8_t>({ mPath.mEndpointId, mPath.mClusterId, Order::Id }, mConfig.order, mConfig.order);

    // if getting the value from persistent storage fails, continue with our lives.
    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus WiredPowerSourceCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                AttributeValueEncoder & encoder)
{
    using namespace PowerSource::Attributes;
    // `ReadAttribute` is guaranteed to only be called for attributes that are supported by the cluster, so the code below is valid.
    switch (request.path.mAttributeId)
    {
    case Status::Id:
        return encoder.Encode(to_underlying(mConfig.status));
    case Order::Id:
        return encoder.Encode(mConfig.order);
    case Description::Id:
        return encoder.Encode(GetDescription());
    case WiredAssessedInputVoltage::Id:
        return EncodeOptional(encoder, mConfig.assessedInputVoltage);
    case WiredAssessedInputFrequency::Id:
        return EncodeOptional(encoder, mConfig.assessedInputFrequency);
    case WiredCurrentType::Id:
        return encoder.Encode(to_underlying(mConfig.currentType));
    case WiredAssessedCurrent::Id:
        return EncodeOptional(encoder, mConfig.assessedCurrent);
    case WiredNominalVoltage::Id:
        return encoder.Encode(mConfig.nominalVoltage);
    case WiredMaximumCurrent::Id:
        return encoder.Encode(mConfig.maximumCurrent);
    case WiredPresent::Id:
        return encoder.Encode(mConfig.isPresent);
    case ActiveWiredFaults::Id:
        return EncodeListOfValues(encoder, Span(mConfig.activeFaultsBuf, mConfig.activeFaultsCount));
    case EndpointList::Id:
        return EncodeListOfValues(encoder, GetEndpointList());
    case Globals::Attributes::FeatureMap::Id:
        return encoder.Encode(BitFlags(Feature::kWired));
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(kRevision);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR WiredPowerSourceCluster::Attributes(const ConcreteClusterPath & path,
                                          ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    ReturnErrorOnFailure(builder.AppendElements(Span(kMandatoryMetadata)));
    return builder.AppendElements(
        {
            WiredAssessedInputVoltage::kMetadataEntry,
            WiredAssessedInputFrequency::kMetadataEntry,
            WiredCurrentType::kMetadataEntry,
            WiredAssessedCurrent::kMetadataEntry,
            WiredNominalVoltage::kMetadataEntry,
            WiredMaximumCurrent::kMetadataEntry,
            WiredPresent::kMetadataEntry,
            ActiveWiredFaults::kMetadataEntry
        });
}

CHIP_ERROR WiredPowerSourceCluster::SetOrder(uint8_t val)
{
    // This attribute is marked as `Persistent`.
    if (mContext != nullptr && val != mConfig.order)
    {
        AttributePersistence attributePersistence(mContext->attributeStorage);

        attributePersistence.StoreNativeEndianValue({ mPath.mEndpointId, mPath.mClusterId, Order::Id }, val);
    }

    SetAttributeValue(mConfig.order, val, Order::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WiredPowerSourceCluster::SetActiveWiredFaults(Span<const WiredFaultEnum> val)
{
    auto bitset = SpanToBitSet<uint8_t>(ConvertSpanType<uint8_t>(val));
    if (mConfig.activeFaultsBitSet == bitset)
    {
        return CHIP_NO_ERROR; // no-op if equal
    }

    GenerateEventAndSetAndNotify(bitset);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WiredPowerSourceCluster::AddActiveWiredFault(WiredFaultEnum val)
{
    if (mConfig.activeFaultsBitSet & (1 << to_underlying(val)))
    {
        return CHIP_NO_ERROR; // no-op if already present
    }

    GenerateEventAndSetAndNotify(mConfig.activeFaultsBitSet | (1 << to_underlying(val)));

    return CHIP_NO_ERROR;
}

CHIP_ERROR WiredPowerSourceCluster::RemoveActiveWiredFault(WiredFaultEnum val)
{
    if (!(mConfig.activeFaultsBitSet & (1 << to_underlying(val))))
    {
        return CHIP_NO_ERROR; // no-op if not present
    }

    GenerateEventAndSetAndNotify(mConfig.activeFaultsBitSet & ~(1 << to_underlying(val)));

    return CHIP_NO_ERROR;
}

void WiredPowerSourceCluster::GenerateEventAndSetAndNotify(uint8_t newBitSet)
{
    uint8_t newBuf[to_underlying(WiredFaultEnum::kUnknownEnumValue)];
    auto oldSpan = ConvertSpanType<WiredFaultEnum>(BitSetToSpan(mConfig.activeFaultsBitSet, mConfig.activeFaultsBuf));
    auto newSpanNonConverted = BitSetToSpan(newBitSet, newBuf);
    auto newSpan = ConvertSpanType<WiredFaultEnum>(newSpanNonConverted);

    WiredFaultChange::Type event_data{ oldSpan, newSpan };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event_data, mPath.mEndpointId);

    mConfig.activeFaultsBitSet = newBitSet;
    mConfig.activeFaultsCount = newSpan.size();
    std::copy(newSpanNonConverted.begin(), newSpanNonConverted.end(), mConfig.activeFaultsBuf);

    NotifyAttributeChanged(ActiveWiredFaults::Id);
}

BatteryPowerSourceCluster::BatteryPowerSourceCluster(EndpointId endpointId, System::Layer & systemLayer, const Configuration & config) :
    DefaultServerCluster({ endpointId, PowerSource::Id }), mConfig(config), mSystemLayer(systemLayer)
{}

CHIP_ERROR BatteryPowerSourceCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    // the `Order` attribute is marked as `Persistent`.
    VerifyOrReturnValue(mConfig.orderFetchFromPersistentStorageDuringStartup, CHIP_NO_ERROR);

    AttributePersistence attributePersistence(context.attributeStorage);
    attributePersistence.LoadNativeEndianValue<uint8_t>({ mPath.mEndpointId, mPath.mClusterId, Order::Id }, mConfig.order, mConfig.order);

    // if getting the value from persistent storage fails, continue with our lives.
    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus BatteryPowerSourceCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                       AttributeValueEncoder & encoder)
{
    using namespace PowerSource::Attributes;
    // `ReadAttribute` is guaranteed to only be called for attributes that are supported by the cluster, so the code below is valid.
    switch (request.path.mAttributeId)
    {
    case Status::Id:
        return encoder.Encode(to_underlying(mConfig.status));
    case Order::Id:
        return encoder.Encode(mConfig.order);
    case Description::Id:
        return encoder.Encode(GetDescription());
    case BatVoltage::Id:
        return EncodeOptional(encoder, mConfig.voltage);
    case BatPercentRemaining::Id:
        return EncodeOptional(encoder, mConfig.percentRemaining);
    case BatTimeRemaining::Id:
        return EncodeOptional(encoder, mConfig.timeRemaining);
    case BatChargeLevel::Id:
        return encoder.Encode(to_underlying(mConfig.chargeLevel));
    case BatReplacementNeeded::Id:
        return encoder.Encode(mConfig.replacementNeeded);
    case BatReplaceability::Id:
        return encoder.Encode(to_underlying(mConfig.replaceability));
    case BatPresent::Id:
        return encoder.Encode(mConfig.isPresent);
    case ActiveBatFaults::Id:
        return EncodeListOfValues(encoder, Span(mConfig.activeFaultsBuf, mConfig.activeFaultsCount));
    case BatReplacementDescription::Id:
        return encoder.Encode(GetBatReplacementDescription());
    case BatCommonDesignation::Id:
        return encoder.Encode(to_underlying(mConfig.commonDesignation));
    case BatANSIDesignation::Id:
        return encoder.Encode(GetBatANSIDesignation());
    case BatIECDesignation::Id:
        return encoder.Encode(GetBatIECDesignation());
    case BatApprovedChemistry::Id:
        return encoder.Encode(to_underlying(mConfig.approvedChemistry));
    case BatCapacity::Id:
        return encoder.Encode(mConfig.capacity);
    case BatQuantity::Id:
        return encoder.Encode(mConfig.quantity);
    case BatChargeState::Id:
        return encoder.Encode(to_underlying(mConfig.chargeState));
    case BatTimeToFullCharge::Id:
        return EncodeOptional(encoder, mConfig.timeToFullCharge);
    case BatFunctionalWhileCharging::Id:
        return encoder.Encode(mConfig.functionalWhileCharging);
    case BatChargingCurrent::Id:
        return EncodeOptional(encoder, mConfig.chargingCurrent);
    case ActiveBatChargeFaults::Id:
        return EncodeListOfValues(encoder, Span(mConfig.activeChargeFaultsBuf, mConfig.activeChargeFaultsCount));
    case EndpointList::Id:
        return EncodeListOfValues(encoder, GetEndpointList());
    case Globals::Attributes::FeatureMap::Id:
        return encoder.Encode(
            BitFlags<PowerSource::Feature>(PowerSource::Feature::kBattery)
                .Set(PowerSource::Feature::kReplaceable,  mConfig.replaceable)
                .Set(PowerSource::Feature::kRechargeable, mConfig.rechargeable));
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(kRevision);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR BatteryPowerSourceCluster::Attributes(const ConcreteClusterPath & path,
                                                 ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    // Workaround to save flash.
    constexpr uint32_t batteryAttributeBits =
        OptionalAttributeSet<BatVoltage::Id, BatPercentRemaining::Id, BatTimeRemaining::Id, BatChargeLevel::Id,
                             BatReplacementNeeded::Id, BatReplaceability::Id, BatPresent::Id, ActiveBatFaults::Id>::All();

    constexpr uint32_t replaceableBatteryAttributeBits =
        OptionalAttributeSet<BatReplacementDescription::Id, BatCommonDesignation::Id, BatANSIDesignation::Id, BatIECDesignation::Id,
                             BatApprovedChemistry::Id, BatQuantity::Id>::All();

    constexpr uint32_t rechargeableBatteryAttributeBits =
        OptionalAttributeSet<BatChargeState::Id, BatTimeToFullCharge::Id, BatFunctionalWhileCharging::Id, BatChargingCurrent::Id,
                             ActiveBatChargeFaults::Id>::All();

    constexpr uint32_t capacityAttributeBit = OptionalAttributeSet<BatCapacity::Id>::All();

    constexpr DataModel::AttributeEntry kOptionalAttributes[] = { BatVoltage::kMetadataEntry,
                                                                  BatPercentRemaining::kMetadataEntry,
                                                                  BatTimeRemaining::kMetadataEntry,
                                                                  BatChargeLevel::kMetadataEntry,
                                                                  BatReplacementNeeded::kMetadataEntry,
                                                                  BatReplaceability::kMetadataEntry,
                                                                  BatPresent::kMetadataEntry,
                                                                  ActiveBatFaults::kMetadataEntry,
                                                                  BatReplacementDescription::kMetadataEntry,
                                                                  BatCommonDesignation::kMetadataEntry,
                                                                  BatANSIDesignation::kMetadataEntry,
                                                                  BatIECDesignation::kMetadataEntry,
                                                                  BatApprovedChemistry::kMetadataEntry,
                                                                  BatCapacity::kMetadataEntry,
                                                                  BatQuantity::kMetadataEntry,
                                                                  BatChargeState::kMetadataEntry,
                                                                  BatTimeToFullCharge::kMetadataEntry,
                                                                  BatFunctionalWhileCharging::kMetadataEntry,
                                                                  BatChargingCurrent::kMetadataEntry,
                                                                  ActiveBatChargeFaults::kMetadataEntry };

    AttributeSet optAttributeSet(batteryAttributeBits |
                                 mConfig.replaceable * replaceableBatteryAttributeBits |
                                 mConfig.rechargeable * rechargeableBatteryAttributeBits |
                                 (mConfig.replaceable || mConfig.rechargeable) * capacityAttributeBit);

    AttributeListBuilder attributeListBuilder(builder);

    return attributeListBuilder.Append(Span(kMandatoryMetadata), Span(kOptionalAttributes), optAttributeSet);
}

CHIP_ERROR BatteryPowerSourceCluster::SetOrder(uint8_t val)
{
    // This attribute is marked as `Persistent`.
    if (mContext != nullptr && val != mConfig.order)
    {
        AttributePersistence attributePersistence(mContext->attributeStorage);

        attributePersistence.StoreNativeEndianValue({ mPath.mEndpointId, mPath.mClusterId, Order::Id }, val);
    }

    SetAttributeValue(mConfig.order, val, Order::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BatteryPowerSourceCluster::SetActiveBatFaults(Span<const BatFaultEnum> val)
{
    uint8_t newBitSet = SpanToBitSet<uint8_t>(ConvertSpanType<uint8_t>(val));
    if (mConfig.activeFaultsBitSet == newBitSet)
    {
        return CHIP_NO_ERROR;
    }

    GenerateBatFaultEventAndSetAndNotify(newBitSet);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BatteryPowerSourceCluster::AddActiveBatFault(BatFaultEnum val)
{
    uint8_t newBitSet = mConfig.activeFaultsBitSet | (1 << to_underlying(val));
    if (mConfig.activeFaultsBitSet == newBitSet)
    {
        return CHIP_NO_ERROR;
    }

    GenerateBatFaultEventAndSetAndNotify(newBitSet);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BatteryPowerSourceCluster::RemoveActiveBatFault(BatFaultEnum val)
{
    uint8_t newBitSet = mConfig.activeFaultsBitSet & ~(1 << to_underlying(val));
    if (mConfig.activeFaultsBitSet == newBitSet)
    {
        return CHIP_NO_ERROR;
    }

    GenerateBatFaultEventAndSetAndNotify(newBitSet);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BatteryPowerSourceCluster::SetActiveBatChargeFaults(Span<const BatChargeFaultEnum> val)
{
    uint16_t newBitSet = SpanToBitSet<uint16_t>(ConvertSpanType<uint8_t>(val));
    if (mConfig.activeChargeFaultsBitSet == newBitSet)
    {
        return CHIP_NO_ERROR;
    }

    GenerateBatChargeFaultEventAndSetAndNotify(newBitSet);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BatteryPowerSourceCluster::AddActiveBatChargeFault(BatChargeFaultEnum val)
{
    uint16_t newBitSet = mConfig.activeChargeFaultsBitSet | (1 << to_underlying(val));
    if (mConfig.activeChargeFaultsBitSet == newBitSet)
    {
        return CHIP_NO_ERROR;
    }

    GenerateBatChargeFaultEventAndSetAndNotify(newBitSet);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BatteryPowerSourceCluster::RemoveActiveBatChargeFault(BatChargeFaultEnum val)
{
    uint16_t newBitSet = mConfig.activeChargeFaultsBitSet & ~(1 << to_underlying(val));
    if (mConfig.activeChargeFaultsBitSet == newBitSet)
    {
        return CHIP_NO_ERROR;
    }

    GenerateBatChargeFaultEventAndSetAndNotify(newBitSet);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BatteryPowerSourceCluster::SetBatPercentRemaining(Optional<uint8_t> val)
{
    if (mConfig.percentRemaining == val)
    {
        return CHIP_NO_ERROR;
    }

    mConfig.percentRemaining = val;

    if (mBatPercentRemainingNotifyTimerExpired.exchange(false))
    {
        mSystemLayer.CancelTimer(SetTimerExpired, &mBatPercentRemainingNotifyTimerExpired);
    }

    CHIP_ERROR err = mSystemLayer.StartTimer(notifyTimerDuration, SetTimerExpired, &mBatPercentRemainingNotifyTimerExpired);
    if (err != CHIP_NO_ERROR)
    {
        // If we can't start the timer, notify immediately
        NotifyAttributeChanged(BatPercentRemaining::Id);
        mBatPercentRemainingNotifyTimerExpired = true;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BatteryPowerSourceCluster::SetBatTimeRemaining(Optional<uint32_t> val)
{
    if (mConfig.timeRemaining == val)
    {
        return CHIP_NO_ERROR;
    }

    mConfig.timeRemaining = val;

    if (mBatTimeRemainingNotifyTimerExpired.exchange(false))
    {
        mSystemLayer.CancelTimer(SetTimerExpired, &mBatTimeRemainingNotifyTimerExpired);
    }

    CHIP_ERROR err = mSystemLayer.StartTimer(notifyTimerDuration, SetTimerExpired, &mBatTimeRemainingNotifyTimerExpired);
    if (err != CHIP_NO_ERROR)
    {
        // If we can't start the timer, notify immediately
        NotifyAttributeChanged(BatTimeRemaining::Id);
        mBatTimeRemainingNotifyTimerExpired = true;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BatteryPowerSourceCluster::SetBatTimeToFullCharge(Optional<uint32_t> val)
{
    if (mConfig.timeToFullCharge == val)
    {
        return CHIP_NO_ERROR;
    }

    mConfig.timeToFullCharge = val;

    if (mBatTimeToFullChargeNotifyTimerExpired.exchange(false))
    {
        mSystemLayer.CancelTimer(SetTimerExpired, &mBatTimeToFullChargeNotifyTimerExpired);
    }

    CHIP_ERROR err = mSystemLayer.StartTimer(notifyTimerDuration, SetTimerExpired, &mBatTimeToFullChargeNotifyTimerExpired);
    if (err != CHIP_NO_ERROR)
    {
        // If we can't start the timer, notify immediately
        NotifyAttributeChanged(BatTimeToFullCharge::Id);
        mBatTimeToFullChargeNotifyTimerExpired = true;
    }

    return CHIP_NO_ERROR;
}

void BatteryPowerSourceCluster::GenerateBatFaultEventAndSetAndNotify(uint8_t newBitSet)
{
    uint8_t newBuf[to_underlying(BatFaultEnum::kUnknownEnumValue)];
    auto oldSpan = ConvertSpanType<BatFaultEnum>(BitSetToSpan(mConfig.activeFaultsBitSet, mConfig.activeFaultsBuf));
    auto newSpanNonConverted = BitSetToSpan(newBitSet, newBuf);
    auto newSpan = ConvertSpanType<BatFaultEnum>(newSpanNonConverted);

    BatFaultChange::Type event_data{ oldSpan, newSpan };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event_data, mPath.mEndpointId);

    mConfig.activeFaultsBitSet = newBitSet;
    mConfig.activeFaultsCount = newSpan.size();
    std::copy(newSpanNonConverted.begin(), newSpanNonConverted.end(), mConfig.activeFaultsBuf);

    NotifyAttributeChanged(ActiveBatFaults::Id);
}

void BatteryPowerSourceCluster::GenerateBatChargeFaultEventAndSetAndNotify(uint16_t newBitSet)
{
    uint8_t newBuf[to_underlying(BatChargeFaultEnum::kUnknownEnumValue)];
    auto oldSpan = ConvertSpanType<BatChargeFaultEnum>(BitSetToSpan(mConfig.activeChargeFaultsBitSet, mConfig.activeChargeFaultsBuf));
    auto newSpanNonConverted = BitSetToSpan(newBitSet, newBuf);
    auto newSpan = ConvertSpanType<BatChargeFaultEnum>(newSpanNonConverted);

    BatChargeFaultChange::Type event_data{ oldSpan, newSpan };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event_data, mPath.mEndpointId);

    mConfig.activeChargeFaultsBitSet = newBitSet;
    mConfig.activeChargeFaultsCount = newSpan.size();
    std::copy(newSpanNonConverted.begin(), newSpanNonConverted.end(), mConfig.activeChargeFaultsBuf);

    NotifyAttributeChanged(ActiveBatChargeFaults::Id);
}

} // namespace Clusters
} // namespace app
} // namespace chip

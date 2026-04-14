/*
 *
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

#pragma once

#include <algorithm>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/PowerSource/Attributes.h>
#include <clusters/PowerSource/Commands.h>
#include <clusters/PowerSource/Enums.h>
#include <clusters/PowerSource/Events.h>
#include <lib/support/ScopedMemoryBuffer.h>

#include <limits>

namespace chip {
namespace app {
namespace Clusters {

/// Uses std::numeric_limits<T>::max() as null value, saving sizeof(T) space
/// C++ rules make that one bool will make the struct two times bigger.
template<class T, class = std::enable_if_t<std::is_integral_v<T>, void>>
struct ConstexprIntegralOptional
{
    constexpr ConstexprIntegralOptional(): value(std::numeric_limits<T>::max()) {}
    constexpr ConstexprIntegralOptional(const T& val) : value(val) {}
    constexpr ConstexprIntegralOptional(const ConstexprIntegralOptional & other) = default;
    constexpr ConstexprIntegralOptional & operator=(const T & val)
    {
        value    = val;
        return *this;
    }

    constexpr ConstexprIntegralOptional & operator=(const ConstexprIntegralOptional & other) = default;

    ConstexprIntegralOptional & operator=(Optional<T> other)
    {
        if (other.HasValue())
        {
            value = other.Value();
        }
        else
        {
            reset();
        }
        return *this;
    }

    bool operator==(Optional<T> other)
    {
        if (HasValue() && other.HasValue())
        {
            return value == other.Value();
        }

        return !HasValue() && !other.HasValue();
    }

    constexpr void reset()
    {
        value = std::numeric_limits<T>::max();
    }

    constexpr bool HasValue() const
    {
        return value != std::numeric_limits<T>::max();
    }

    operator Optional<T>() const
    {
        if (value != std::numeric_limits<T>::max())
        {
            return MakeOptional(value);
        }
        return NullOptional;
    }

    T value;
};

template<class T, class = std::enable_if_t<std::is_unsigned_v<T> && std::is_integral_v<T>, void>>
constexpr static T SpanToBitSet(Span<const uint8_t> span)
{
    T val{};
    for (uint8_t el : span)
    {
        val |= (1 << el);
    }
    return val;
}

template<class T, class = std::enable_if_t<std::is_unsigned_v<T> && std::is_integral_v<T>, void>>
constexpr static Span<const uint8_t> BitSetToSpan(T bitset, uint8_t * buffer)
{
    size_t bufInd = 0;
    for (uint8_t i = 0; i < sizeof(T) * 8; i++)
    {
        if (bitset & (1 << i))
        {
            buffer[bufInd++] = i;
        }
    }

    return Span(buffer, bufInd);
}

template<class To, class From, class = std::enable_if_t<sizeof(From) == sizeof(To), void>>
constexpr static Span<const To> ConvertSpanType(Span<const From> span)
{
    return Span(reinterpret_cast<const To *>(span.data()), span.size());
}

class WiredPowerSourceCluster : public DefaultServerCluster
{
public:
    using PowerSourceStatusEnum    = PowerSource::PowerSourceStatusEnum;
    using WiredCurrentTypeEnum     = PowerSource::WiredCurrentTypeEnum;
    using WiredFaultEnum           = PowerSource::WiredFaultEnum;

    // Comments for each attriubute/member mean
    // - fixed? if the attribute is fixed, it means that it can ONLY be set in the constructor and CANNOT be changed afterwards.
    //
    // EndpointList attribute cannot be set from the constructor, use the `WiredPowerSourceCluster::SetEndpointList` function for them.
    //
    // If it is possible (user is able to support it), all attributes are expected to have valid (and meaningfull) values.
    // The cluster currently doesn't have a way to express if some optional attributes cannot be supported (for flash usage reasons),
    // so in that case the cluster will report a default value (0 for numbers, empty for strings, and other defaults you can see below).

    struct Configuration
    {
        uint32_t nominalVoltage{}; // fixed
        uint32_t maximumCurrent{}; // fixed
        ConstexprIntegralOptional<uint32_t> assessedInputVoltage{};
        ConstexprIntegralOptional<uint32_t> assessedCurrent{};
        ConstexprIntegralOptional<uint16_t> assessedInputFrequency{};
        WiredCurrentTypeEnum currentType{}; // fixed
        bool isPresent{};
        PowerSourceStatusEnum status = PowerSourceStatusEnum::kUnspecified;
        uint8_t order{};
        bool orderFetchFromPersistentStorageDuringStartup{ true }; // this is not an attribute, can be set only here

        // - fixed (see class comment)
        // - will truncate the input if too big, max length is `kMaxDescriptionLen`
        constexpr void SetDescription(CharSpan val)
        {
            MutableCharSpan outSpan(description, kMaxDescriptionLen);
            CopyCharSpanToMutableCharSpanWithTruncation(val, outSpan);
            descriptionLen = static_cast<uint8_t>(outSpan.size());
        }

        constexpr void SetActiveFaults(Span<const WiredFaultEnum> val)
        {
            activeFaultsBitSet = SpanToBitSet<uint8_t>(ConvertSpanType<uint8_t>(val));
            auto span = BitSetToSpan(activeFaultsBitSet, activeFaultsBuf);
            activeFaultsCount = static_cast<uint8_t>(span.size());
        }

        // To force the user to specify mandatory fixed attributes (taking the corresponding feature into account).
        constexpr Configuration(CharSpan desc, WiredCurrentTypeEnum currType) : currentType(currType)
        {
            SetDescription(desc);
        }

    private:

        constexpr static uint8_t kMaxDescriptionLen = PowerSource::Attributes::Description::TypeInfo::MaxLength();
        constexpr static uint8_t kMaxActiveFaultsCount = to_underlying(WiredFaultEnum::kUnknownEnumValue);

        char description[kMaxDescriptionLen]{};
        uint8_t descriptionLen = 0;
        uint8_t activeFaultsBuf[kMaxActiveFaultsCount]{};
        uint8_t activeFaultsBitSet{}; // WiredFault::Enum::kUnknownEnumValue = 3 possible faults only, so uint8_t is enough.
        uint8_t activeFaultsCount{}; // to avoid counting bits in the bitset every time we need to get the faults span.

        friend class WiredPowerSourceCluster;
    };

    WiredPowerSourceCluster(EndpointId endpointId, const Configuration & config) :
        DefaultServerCluster({ endpointId, PowerSource::Id }), mConfig(config)
    {}

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    // Getters

    PowerSourceStatusEnum GetStatus() const
    {
        return mConfig.status;
    }
    uint8_t GetOrder() const
    {
        return mConfig.order;
    }
    CharSpan GetDescription() const
    {
        return Span(mConfig.description, mConfig.descriptionLen);
    }
    Optional<uint32_t> GetWiredAssessedInputVoltage() const
    {
        return mConfig.assessedInputVoltage;
    }
    Optional<uint16_t> GetWiredAssessedInputFrequency() const
    {
        return mConfig.assessedInputFrequency;
    }
    WiredCurrentTypeEnum GetWiredCurrentType() const
    {
        return mConfig.currentType;
    }
    Optional<uint32_t> GetWiredAssessedCurrent() const
    {
        return mConfig.assessedCurrent;
    }
    uint32_t GetWiredNominalVoltage() const
    {
        return mConfig.nominalVoltage;
    }
    uint32_t GetWiredMaximumCurrent() const
    {
        return mConfig.maximumCurrent;
    }
    bool GetWiredPresent() const
    {
        return mConfig.isPresent;
    }
    Span<const WiredFaultEnum> GetActiveWiredFaults() const
    {
        return ConvertSpanType<WiredFaultEnum>(Span(mConfig.activeFaultsBuf, mConfig.activeFaultsCount));
    }
    Span<const EndpointId> GetEndpointList() const
    {
        return Span(mEndpointList.Get(), mEndpointListCount);
    }

    // Setters

    /// Attributes marked with the `Fixed` quality do not have setters.
    /// They can be only set during construction using the `WiredPowerSourceCluster::Configuration` class.
    /// `Fixed` attributes are `Description`, `WiredCurrentType`, `WiredNominalVoltage`, `WiredMaximumCurrent`

    CHIP_ERROR SetStatus(PowerSourceStatusEnum val)
    {
        SetAttributeValue(mConfig.status, val, PowerSource::Attributes::Status::Id);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetOrder(uint8_t val);
    CHIP_ERROR SetWiredAssessedInputVoltage(Optional<uint32_t> val)
    {
        mConfig.assessedInputVoltage = val; // no notifying because attribute marked with 'Changes Omitted' quality
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetWiredAssessedInputFrequency(Optional<uint16_t> val)
    {
        mConfig.assessedInputFrequency = val; // no notifying because attribute marked with 'Changes Omitted' quality
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetWiredAssessedCurrent(Optional<uint32_t> val)
    {
        mConfig.assessedCurrent = val; // no notifying because attribute marked with 'Changes Omitted' quality
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetWiredPresent(bool val)
    {
        SetAttributeValue(mConfig.isPresent, val, PowerSource::Attributes::WiredPresent::Id);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetActiveWiredFaults(Span<const WiredFaultEnum> val);
    CHIP_ERROR AddActiveWiredFault(WiredFaultEnum val);
    CHIP_ERROR RemoveActiveWiredFault(WiredFaultEnum val);
    CHIP_ERROR SetEndpointList(Span<const EndpointId> val);

private:

    Configuration mConfig;

    Platform::ScopedMemoryBuffer<EndpointId> mEndpointList;
    uint16_t mEndpointListCount = 0;

    void GenerateEventAndSetAndNotify(uint8_t newBitSet);
};

class BatteryPowerSourceCluster : public DefaultServerCluster
{
public:
    using PowerSourceStatusEnum    = PowerSource::PowerSourceStatusEnum;
    using BatChargeLevelEnum       = PowerSource::BatChargeLevelEnum;
    using BatReplaceabilityEnum    = PowerSource::BatReplaceabilityEnum;
    using BatFaultEnum             = PowerSource::BatFaultEnum;
    using BatCommonDesignationEnum = PowerSource::BatCommonDesignationEnum;
    using BatApprovedChemistryEnum = PowerSource::BatApprovedChemistryEnum;
    using BatChargeStateEnum       = PowerSource::BatChargeStateEnum;
    using BatChargeFaultEnum       = PowerSource::BatChargeFaultEnum;

    // Comments for each member/attribute mean
    // - corresponding feature, if you do not want to support the feature, you can ignore the member, it won't be used.
    // - fixed? if the attribute is fixed, it means that it can ONLY be set in the constructor and CANNOT be changed afterwards.
    //
    // EndpointList attribute cannot be set from the constructor, use the `BatteryPowerSourceCluster::SetEndpointList` function for them.
    //
    // If it is possible (user is able to support it), all attributes are expected to have valid (and meaningfull) values.
    // The cluster currently doesn't have a way to express if some optional attributes cannot be supported (for FLASH reasons),
    // so in that case the cluster will report a default value
    // (false for booleans, 0 for numbers, empty for strings, and other defaults you can see below).
    struct Configuration
    {
        // do not change the order of these members, it is to minimize padding.

        ConstexprIntegralOptional<uint32_t> voltage{};
        ConstexprIntegralOptional<uint32_t> timeRemaining{};
        uint32_t capacity{}; // replaceable or rechargeable fixed
        ConstexprIntegralOptional<uint32_t> timeToFullCharge{}; // rechargeable
        ConstexprIntegralOptional<uint32_t> chargingCurrent{}; // rechargeable
        BatCommonDesignationEnum commonDesignation{}; // replaceable fixed
        BatApprovedChemistryEnum approvedChemistry{}; // replaceable fixed
        PowerSourceStatusEnum status = PowerSourceStatusEnum::kUnspecified;
        uint8_t order{};
        bool orderFetchFromPersistentStorageDuringStartup{ true }; // this is not an attribute, can be set only here
        ConstexprIntegralOptional<uint8_t> percentRemaining{}; // (max 200, 0 <-> 0%, 200 <-> 100%)
        BatChargeLevelEnum chargeLevel = BatChargeLevelEnum::kOk;
        bool replacementNeeded{};
        BatReplaceabilityEnum replaceability = BatReplaceabilityEnum::kUnspecified; // fixed
        bool isPresent{};
        uint8_t quantity{}; // replaceable fixed
        BatChargeStateEnum chargeState = BatChargeStateEnum::kUnknown; // rechargeable
        bool functionalWhileCharging{}; // rechargeable

        // To force the user to specify mandatory fixed attributes (taking the corresponding features into account).

        constexpr Configuration(CharSpan desc, BatReplaceabilityEnum replability) :
            replaceability(replability), replaceable(false), rechargeable(false)
        {
            SetDescription(desc);
        }

        constexpr void MakeReplaceable(CharSpan replDescription, uint8_t quan)
        {
            replaceable = true;
            SetReplacementDescription(replDescription);
            quantity = quan;
        }

        constexpr void MakeRechargeable() { rechargeable = true; }

        constexpr bool isReplaceable() const { return replaceable; }
        constexpr bool isRechargeable() const { return rechargeable; }

        // Setters for strings
        constexpr void SetDescription(CharSpan val)
        {
            MutableCharSpan outSpan(description, kMaxDescriptionLen);
            CopyCharSpanToMutableCharSpanWithTruncation(val, outSpan);
            descriptionLen = static_cast<uint8_t>(outSpan.size());
        }

        constexpr void SetReplacementDescription(CharSpan val)
        {
            MutableCharSpan outSpan(replacementDescription, kMaxReplacementDescriptionLen);
            CopyCharSpanToMutableCharSpanWithTruncation(val, outSpan);
            replacementDescriptionLen = static_cast<uint8_t>(outSpan.size());
        }

        constexpr void SetANSIDesignation(CharSpan val)
        {
            MutableCharSpan outSpan(ansiDesignation, kMaxANSIDesignationLen);
            CopyCharSpanToMutableCharSpanWithTruncation(val, outSpan);
            ansiDesignationLen = static_cast<uint8_t>(outSpan.size());
        }

        constexpr void SetIECDesignation(CharSpan val)
        {
            MutableCharSpan outSpan(iecDesignation, kMaxIECDesignationLen);
            CopyCharSpanToMutableCharSpanWithTruncation(val, outSpan);
            iecDesignationLen = static_cast<uint8_t>(outSpan.size());
        }

        // Setters for lists
        constexpr void SetActiveFaults(Span<const BatFaultEnum> val)
        {
            activeFaultsBitSet = SpanToBitSet<uint8_t>(ConvertSpanType<uint8_t>(val));
            auto span = BitSetToSpan(activeFaultsBitSet, activeFaultsBuf);
            activeFaultsCount = static_cast<uint8_t>(span.size());
        }

        constexpr void SetActiveChargeFaults(Span<const BatChargeFaultEnum> val)
        {
            activeChargeFaultsBitSet = SpanToBitSet<uint16_t>(ConvertSpanType<uint8_t>(val));
            auto span = BitSetToSpan(activeChargeFaultsBitSet, activeChargeFaultsBuf);
            activeChargeFaultsCount = static_cast<uint8_t>(span.size());
        }

    private:
        bool replaceable;
        bool rechargeable;

        constexpr static uint8_t kMaxDescriptionLen = PowerSource::Attributes::Description::TypeInfo::MaxLength();
        constexpr static uint8_t kMaxReplacementDescriptionLen = PowerSource::Attributes::BatReplacementDescription::TypeInfo::MaxLength();
        constexpr static uint8_t kMaxANSIDesignationLen = PowerSource::Attributes::BatANSIDesignation::TypeInfo::MaxLength();
        constexpr static uint8_t kMaxIECDesignationLen = PowerSource::Attributes::BatIECDesignation::TypeInfo::MaxLength();
        constexpr static uint8_t kMaxActiveFaultsCount = to_underlying(BatFaultEnum::kUnknownEnumValue);
        constexpr static uint8_t kMaxActiveChargeFaultsCount = to_underlying(BatChargeFaultEnum::kUnknownEnumValue);

        char description[kMaxDescriptionLen]{};
        uint8_t descriptionLen = 0;
        char replacementDescription[kMaxReplacementDescriptionLen]{};
        uint8_t replacementDescriptionLen = 0;
        char ansiDesignation[kMaxANSIDesignationLen]{};
        uint8_t ansiDesignationLen = 0;
        char iecDesignation[kMaxIECDesignationLen]{};
        uint8_t iecDesignationLen = 0;
        uint8_t activeFaultsBuf[kMaxActiveFaultsCount]{};
        uint8_t activeFaultsBitSet{};
        uint8_t activeFaultsCount{};
        uint8_t activeChargeFaultsBuf[kMaxActiveChargeFaultsCount]{};
        uint16_t activeChargeFaultsBitSet{};
        uint8_t activeChargeFaultsCount{};

        friend class BatteryPowerSourceCluster;
    };

    BatteryPowerSourceCluster(EndpointId endpointId, System::Layer & systemLayer, const Configuration & config);

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    // Getters

    PowerSourceStatusEnum GetStatus() const
    {
        return mConfig.status;
    }
    uint8_t GetOrder() const
    {
        return mConfig.order;
    }
    CharSpan GetDescription() const
    {
        return Span(mConfig.description, mConfig.descriptionLen);
    }
    Optional<uint32_t> GetBatVoltage() const
    {
        return mConfig.voltage;
    }
    Optional<uint8_t> GetBatPercentRemaining() const
    {
        return mConfig.percentRemaining;
    }
    Optional<uint32_t> GetBatTimeRemaining() const
    {
        return mConfig.timeRemaining;
    }
    BatChargeLevelEnum GetBatChargeLevel() const
    {
        return mConfig.chargeLevel;
    }
    bool GetBatReplacementNeeded() const
    {
        return mConfig.replacementNeeded;
    }
    BatReplaceabilityEnum GetBatReplaceability() const
    {
        return mConfig.replaceability;
    }
    bool GetBatPresent() const
    {
        return mConfig.isPresent;
    }
    Span<const BatFaultEnum> GetActiveBatFaults() const
    {
        return ConvertSpanType<BatFaultEnum>(Span(mConfig.activeFaultsBuf, mConfig.activeFaultsCount));
    }
    CharSpan GetBatReplacementDescription() const
    {
        return Span(mConfig.replacementDescription, mConfig.replacementDescriptionLen);
    }
    BatCommonDesignationEnum GetBatCommonDesignation() const
    {
        return mConfig.commonDesignation;
    }
    CharSpan GetBatANSIDesignation() const
    {
        return Span(mConfig.ansiDesignation, mConfig.ansiDesignationLen);
    }
    CharSpan GetBatIECDesignation() const
    {
        return Span(mConfig.iecDesignation, mConfig.iecDesignationLen);
    }
    BatApprovedChemistryEnum GetBatApprovedChemistry() const
    {
        return mConfig.approvedChemistry;
    }
    uint32_t GetBatCapacity() const
    {
        return mConfig.capacity;
    }
    uint8_t GetBatQuantity() const
    {
        return mConfig.quantity;
    }
    BatChargeStateEnum GetBatChargeState() const
    {
        return mConfig.chargeState;
    }
    Optional<uint32_t> GetBatTimeToFullCharge() const
    {
        return mConfig.timeToFullCharge;
    }
    bool GetBatFunctionalWhileCharging() const
    {
        return mConfig.functionalWhileCharging;
    }
    Optional<uint32_t> GetBatChargingCurrent() const
    {
        return mConfig.chargingCurrent;
    }
    Span<const BatChargeFaultEnum> GetActiveBatChargeFaults() const
    {
        return ConvertSpanType<BatChargeFaultEnum>(Span(mConfig.activeChargeFaultsBuf, mConfig.activeChargeFaultsCount));
    }
    Span<const EndpointId> GetEndpointList() const
    {
        return Span(mEndpointList.Get(), mEndpointListCount);
    }

    // Setters

    /// Attributes marked with the `Fixed` quality do not have setters.
    /// They can be only set during construction using the `BatteryPowerSourceCluster::Configuration` class.
    /// `Fixed` attributes are `Description`, `BatReplaceability`,
    /// `BatReplacementDescription`, `BatCommonDesignation`, `BatANSIDesignation`, `BatIECDesignation`, `BatApprovedChemistry`,
    /// `BatCapacity` and `BatQuantity`

    CHIP_ERROR SetStatus(PowerSourceStatusEnum val)
    {
        SetAttributeValue(mConfig.status, val, PowerSource::Attributes::Status::Id);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetOrder(uint8_t val);
    CHIP_ERROR SetBatVoltage(Optional<uint32_t> val)
    {
        mConfig.voltage = val; // no notifying because attribute marked with 'Changes Omitted' quality
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetBatPercentRemaining(Optional<uint8_t> val);
    CHIP_ERROR SetBatTimeRemaining(Optional<uint32_t> val);
    CHIP_ERROR SetBatChargeLevel(BatChargeLevelEnum val)
    {
        SetAttributeValue(mConfig.chargeLevel, val, PowerSource::Attributes::BatChargeLevel::Id);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetBatReplacementNeeded(bool val)
    {
        SetAttributeValue(mConfig.replacementNeeded, val, PowerSource::Attributes::BatReplacementNeeded::Id);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetBatPresent(bool val)
    {
        SetAttributeValue(mConfig.isPresent, val, PowerSource::Attributes::BatPresent::Id);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetActiveBatFaults(Span<const BatFaultEnum> val);
    CHIP_ERROR AddActiveBatFault(BatFaultEnum val);
    CHIP_ERROR RemoveActiveBatFault(BatFaultEnum val);
    CHIP_ERROR SetBatChargeState(BatChargeStateEnum val)
    {
        SetAttributeValue(mConfig.chargeState, val, PowerSource::Attributes::BatChargeState::Id);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetBatTimeToFullCharge(Optional<uint32_t> val);
    CHIP_ERROR SetBatFunctionalWhileCharging(bool val)
    {
        SetAttributeValue(mConfig.functionalWhileCharging, val, PowerSource::Attributes::BatFunctionalWhileCharging::Id);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetBatChargingCurrent(Optional<uint32_t> val)
    {
        mConfig.chargingCurrent = val; // no notifying because attribute marked with 'Changes Omitted' quality
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetActiveBatChargeFaults(Span<const BatChargeFaultEnum> val);
    CHIP_ERROR AddActiveBatChargeFault(BatChargeFaultEnum val);
    CHIP_ERROR RemoveActiveBatChargeFault(BatChargeFaultEnum val);
    CHIP_ERROR SetEndpointList(Span<const EndpointId> val);

private:

    Configuration mConfig;

    Platform::ScopedMemoryBuffer<EndpointId> mEndpointList;
    uint16_t mEndpointListCount = 0;

    void GenerateBatFaultEventAndSetAndNotify(uint8_t newBitSet);
    void GenerateBatChargeFaultEventAndSetAndNotify(uint16_t newBitSet);

    // context
    System::Layer & mSystemLayer;

    // notify timer stuff
    std::atomic_bool mBatPercentRemainingNotifyTimerExpired{ true };
    std::atomic_bool mBatTimeRemainingNotifyTimerExpired{ true };
    std::atomic_bool mBatTimeToFullChargeNotifyTimerExpired{ true };

    static constexpr System::Clock::Timeout notifyTimerDuration = System::Clock::Seconds16(10);

    static void SetTimerExpired(System::Layer *, void * pAtomicBool)
    {
        std::atomic_bool * p = reinterpret_cast<std::atomic_bool *>(pAtomicBool);
        *p                   = true;
    }
};

} // namespace Clusters
} // namespace app
} // namespace chip

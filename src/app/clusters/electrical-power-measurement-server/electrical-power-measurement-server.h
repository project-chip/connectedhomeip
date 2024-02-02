/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <lib/core/Optional.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <lib/support/CommonIterator.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalPowerMeasurement {

using namespace chip::app::Clusters::ElectricalPowerMeasurement::Attributes;
using namespace chip::app::Clusters::ElectricalPowerMeasurement::Structs;

using chip::Protocols::InteractionModel::Status;

class Delegate
{
public:
    virtual ~Delegate() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }

    using AccuracyIterator            = CommonIterator<Structs::MeasurementAccuracyStruct::Type>;
    using RangeIterator               = CommonIterator<Structs::MeasurementRangeStruct::Type>;
    using HarmonicMeasurementIterator = CommonIterator<Structs::HarmonicMeasurementStruct::Type>;

    virtual PowerModeEnum GetPowerMode()                            = 0;
    virtual uint8_t GetNumberOfMeasurementTypes()                   = 0;
    virtual AccuracyIterator * IterateAccuracy()                    = 0;
    virtual RangeIterator * IterateRanges()                         = 0;
    virtual DataModel::Nullable<int64_t> GetVoltage()               = 0;
    virtual DataModel::Nullable<int64_t> GetActiveCurrent()         = 0;
    virtual DataModel::Nullable<int64_t> GetReactiveCurrent()       = 0;
    virtual DataModel::Nullable<int64_t> GetApparentCurrent()       = 0;
    virtual DataModel::Nullable<int64_t> GetActivePower()           = 0;
    virtual DataModel::Nullable<int64_t> GetReactivePower()         = 0;
    virtual DataModel::Nullable<int64_t> GetApparentPower()         = 0;
    virtual DataModel::Nullable<int64_t> GetRMSVoltage()            = 0;
    virtual DataModel::Nullable<int64_t> GetRMSCurrent()            = 0;
    virtual DataModel::Nullable<int64_t> GetRMSPower()              = 0;
    virtual DataModel::Nullable<int64_t> GetFrequency()             = 0;
    virtual HarmonicMeasurementIterator * IterateHarmonicCurrents() = 0;
    virtual HarmonicMeasurementIterator * IterateHarmonicPhases()   = 0;
    virtual DataModel::Nullable<int64_t> GetPowerFactor()           = 0;
    virtual DataModel::Nullable<int64_t> GetNeutralCurrent()        = 0;

protected:
    EndpointId mEndpointId = 0;
};

enum class OptionalAttributes : uint32_t
{
    kOptionalAttributeRanges          = 0x1,
    kOptionalAttributeVoltage         = 0x2,
    kOptionalAttributeActiveCurrent   = 0x4,
    kOptionalAttributeReactiveCurrent = 0x8,
    kOptionalAttributeApparentCurrent = 0x10,
    kOptionalAttributeReactivePower   = 0x20,
    kOptionalAttributeApparentPower   = 0x40,
    kOptionalAttributeRMSVoltage      = 0x80,
    kOptionalAttributeRMSCurrent      = 0x100,
    kOptionalAttributeRMSPower        = 0x200,
    kOptionalAttributeFrequency       = 0x400,
    kOptionalAttributePowerFactor     = 0x800,
    kOptionalAttributeNeutralCurrent  = 0x1000,
};

class Instance : public AttributeAccessInterface
{
public:
    Instance(EndpointId aEndpointId, Delegate & aDelegate, BitMask<Feature> aFeature,
             BitMask<OptionalAttributes> aOptionalAttributes) :
        AttributeAccessInterface(MakeOptional(aEndpointId), Id),
        mDelegate(aDelegate), mFeature(aFeature), mOptionalAttrs(aOptionalAttributes)
    {
        /* set the base class delegates endpointId */
        mDelegate.SetEndpointId(aEndpointId);
    }
    ~Instance() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;
    bool SupportsOptAttr(OptionalAttributes aOptionalAttrs) const;

private:
    Delegate & mDelegate;
    BitMask<Feature> mFeature;
    BitMask<OptionalAttributes> mOptionalAttrs;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    CHIP_ERROR ReadAccuracy(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadRanges(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadHarmonicCurrents(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadHarmonicPhases(AttributeValueEncoder & aEncoder);
};

} // namespace ElectricalPowerMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip

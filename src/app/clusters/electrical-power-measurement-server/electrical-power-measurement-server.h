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
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalPowerMeasurement {

class Delegate
{
public:
    virtual ~Delegate() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }

    virtual PowerModeEnum GetPowerMode()          = 0;
    virtual uint8_t GetNumberOfMeasurementTypes() = 0;

    /* These functions are called by the ReadAttribute handler to iterate through lists
     * The cluster server will call Start<Type>Read to allow the delegate to create a temporary
     * lock on the data.
     * The delegate is expected to not change these values once Start<Type>Read has been called
     * until the End<Type>Read() has been called (e.g. releasing a lock on the data)
     */
    virtual CHIP_ERROR StartAccuracyRead()                                                     = 0;
    virtual CHIP_ERROR GetAccuracyByIndex(uint8_t, Structs::MeasurementAccuracyStruct::Type &) = 0;
    virtual CHIP_ERROR EndAccuracyRead()                                                       = 0;

    virtual CHIP_ERROR StartRangesRead()                                                 = 0;
    virtual CHIP_ERROR GetRangeByIndex(uint8_t, Structs::MeasurementRangeStruct::Type &) = 0;
    virtual CHIP_ERROR EndRangesRead()                                                   = 0;

    virtual CHIP_ERROR StartHarmonicCurrentsRead()                                                     = 0;
    virtual CHIP_ERROR GetHarmonicCurrentsByIndex(uint8_t, Structs::HarmonicMeasurementStruct::Type &) = 0;
    virtual CHIP_ERROR EndHarmonicCurrentsRead()                                                       = 0;

    virtual CHIP_ERROR StartHarmonicPhasesRead()                                                     = 0;
    virtual CHIP_ERROR GetHarmonicPhasesByIndex(uint8_t, Structs::HarmonicMeasurementStruct::Type &) = 0;
    virtual CHIP_ERROR EndHarmonicPhasesRead()                                                       = 0;

    virtual DataModel::Nullable<int64_t> GetVoltage()         = 0;
    virtual DataModel::Nullable<int64_t> GetActiveCurrent()   = 0;
    virtual DataModel::Nullable<int64_t> GetReactiveCurrent() = 0;
    virtual DataModel::Nullable<int64_t> GetApparentCurrent() = 0;
    virtual DataModel::Nullable<int64_t> GetActivePower()     = 0;
    virtual DataModel::Nullable<int64_t> GetReactivePower()   = 0;
    virtual DataModel::Nullable<int64_t> GetApparentPower()   = 0;
    virtual DataModel::Nullable<int64_t> GetRMSVoltage()      = 0;
    virtual DataModel::Nullable<int64_t> GetRMSCurrent()      = 0;
    virtual DataModel::Nullable<int64_t> GetRMSPower()        = 0;
    virtual DataModel::Nullable<int64_t> GetFrequency()       = 0;
    virtual DataModel::Nullable<int64_t> GetPowerFactor()     = 0;
    virtual DataModel::Nullable<int64_t> GetNeutralCurrent()  = 0;

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

    CHIP_ERROR EncodeAccuracy(const AttributeValueEncoder::ListEncodeHelper & aEncoder);
    CHIP_ERROR EncodeRanges(const AttributeValueEncoder::ListEncodeHelper & aEncoder);
    CHIP_ERROR EncodeHarmonicCurrents(const AttributeValueEncoder::ListEncodeHelper & aEncoder);
    CHIP_ERROR EncodeHarmonicPhases(const AttributeValueEncoder::ListEncodeHelper & aEncoder);
};

} // namespace ElectricalPowerMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip

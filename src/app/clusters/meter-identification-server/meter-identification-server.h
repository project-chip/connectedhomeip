/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <stddef.h>

#include <lib/core/Optional.h>

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPError.h>

using chip::app::Clusters::MeterIdentification::MeterTypeEnum;
using chip::app::Clusters::MeterIdentification::PowerThresholdSourceEnum;
using Feature = chip::app::Clusters::MeterIdentification::Feature;

namespace chip {
namespace app {
namespace Clusters {
namespace MeterIdentification {

struct Delegate
{
public:
    virtual ~Delegate() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }

    virtual DataModel::Nullable<MeterTypeEnum> GetMeterType()                       = 0;
    virtual CharSpan GetCustomerName()                                              = 0;
    virtual CharSpan GetUtilityName()                                               = 0;
    virtual CharSpan GetPointOfDelivery()                                           = 0;
    virtual DataModel::Nullable<uint64_t> GetPowerThreshold()                       = 0;
    virtual DataModel::Nullable<PowerThresholdSourceEnum> GetPowerThresholdSource() = 0;

    virtual CHIP_ERROR SetCustomerName(CharSpan & value) = 0;

protected:
    EndpointId mEndpointId = 0;
};

class Instance : public AttributeAccessInterface
{
public:
    Instance(EndpointId aEndpointId, Delegate & aDelegate, BitMask<Feature> aFeature) :
        AttributeAccessInterface(MakeOptional(aEndpointId), Id), mDelegate(aDelegate), mFeature(aFeature)
    {
        /* set the base class delegates endpointId */
        mDelegate.SetEndpointId(aEndpointId);
    }
    ~Instance() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;

private:
    Delegate & mDelegate;
    BitMask<Feature> mFeature;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;
};

} // namespace MeterIdentification
} // namespace Clusters
} // namespace app
} // namespace chip

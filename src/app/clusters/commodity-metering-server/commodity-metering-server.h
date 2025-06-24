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

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityMetering {

class Instance : public AttributeAccessInterface
{
public:
    Instance(const EndpointId & aEndpointId, const BitMask<Feature> & aFeature) :
        AttributeAccessInterface(MakeOptional(aEndpointId), Id), mEndpointId(aEndpointId), mFeature(aFeature)
    {}
    ~Instance() override;

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(const Feature & aFeature) const;

    // Attribute Accessors
    const DataModel::Nullable<DataModel::List<Structs::MeteredQuantityStruct::Type>> & GetMeteredQuantity() const { return mMeteredQuantity; }
    const DataModel::<uint32_t> & GetMeteredQuantityTimestamp() const { return mMeteredQuantityTimestamp; }    
    const DataModel::Nullable<Globals::TariffUnitEnum> & GetTariffUnit() const { return mTariffUnit; }

    // Internal Application API to set attribute values
    CHIP_ERROR SetMeteredQuantity(const DataModel::Nullable<DataModel::List<Structs::MeteredQuantityStruct::Type>> & value);
    CHIP_ERROR SetMeteredQuantityTimestamp(const DataModel::<uint32_t> & value);
    CHIP_ERROR SetTariffUnit(const DataModel::Nullable<Globals::TariffUnitEnum> & value);

private:
    // Attribute storage
    DataModel::Nullable<Globals::TariffUnitEnum> mTariffUnit;
    DataModel::Nullable<uint32_t> mMeteredQuantityTimestamp;
    DataModel::Nullable<DataModel::List<Structs::MeteredQuantityStruct::Type>> mMeteredQuantity;
    EndpointId mEndpointId = 0;
    BitMask<Feature> mFeature;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

} // namespace CommodityMetering
} // namespace Clusters
} // namespace app
} // namespace chip

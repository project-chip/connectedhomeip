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

#include <cstdint>
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
    Instance(const EndpointId & aEndpointId) : AttributeAccessInterface(MakeOptional(aEndpointId), Id), mEndpointId(aEndpointId) {}
    ~Instance() override;

    CHIP_ERROR Init();
    void Shutdown();

    // Attribute Accessors

    /**
     * Gets the current metered quantity data.
     *
     * @warning Returned references are only valid until:
     *          - Next call to SetMeteredQuantity()
     *          - Call to Shutdown()
     *          - Instance destruction
     *
     * @warning Not thread-safe - caller must ensure external synchronization
     *
     * @return Reference to internal state. Do not store long-term.
     */
    const DataModel::Nullable<DataModel::List<Structs::MeteredQuantityStruct::Type>> & GetMeteredQuantity() const
    {
        return mMeteredQuantity;
    }
    const DataModel::Nullable<uint32_t> & GetMeteredQuantityTimestamp() const { return mMeteredQuantityTimestamp; }
    const DataModel::Nullable<Globals::TariffUnitEnum> & GetTariffUnit() const { return mTariffUnit; }
    const DataModel::Nullable<uint16_t> & GetMaximumMeteredQuantities() const { return mMaximumMeteredQuantities; }
    // Internal Application API to set attribute values
    CHIP_ERROR SetMeteredQuantity(const DataModel::Nullable<DataModel::List<Structs::MeteredQuantityStruct::Type>> & value);
    CHIP_ERROR SetMeteredQuantityTimestamp(DataModel::Nullable<uint32_t>);
    CHIP_ERROR SetTariffUnit(DataModel::Nullable<Globals::TariffUnitEnum>);
    CHIP_ERROR SetMaximumMeteredQuantities(DataModel::Nullable<uint16_t>);

private:
    // Attribute storage
    DataModel::Nullable<DataModel::List<Structs::MeteredQuantityStruct::Type>> mMeteredQuantity;
    DataModel::Nullable<uint32_t> mMeteredQuantityTimestamp;
    DataModel::Nullable<Globals::TariffUnitEnum> mTariffUnit;
    DataModel::Nullable<uint16_t> mMaximumMeteredQuantities;

    EndpointId mEndpointId = 0;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

} // namespace CommodityMetering
} // namespace Clusters
} // namespace app
} // namespace chip

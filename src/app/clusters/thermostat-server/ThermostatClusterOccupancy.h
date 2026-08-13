/**
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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeValueEncoder.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/OperationTypes.h>
#include <lib/support/BitMask.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class ThermostatCluster;

class ThermostatOccupancy
{
public:
    class Delegate
    {
    public:
        virtual ~Delegate() = default;

        virtual BitMask<OccupancyBitmap> GetOccupancy() const = 0;
        virtual Protocols::InteractionModel::Status SetOccupancy(BitMask<OccupancyBitmap> occupied) = 0;
    };

    explicit ThermostatOccupancy(ThermostatCluster & cluster){};

    void SetDelegate(Delegate * delegate) { mDelegate = delegate; }

    std::optional<DataModel::ActionReturnStatus> ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder);

    bool IsOccupied() const
    {
        if (mDelegate)
        {
            return mDelegate->GetOccupancy().Has(OccupancyBitmap::kOccupied);
        }
        return true;
    }

    Protocols::InteractionModel::Status SetOccupancy(BitMask<OccupancyBitmap> occupied)
    {
        if (mDelegate)
        {
            return mDelegate->SetOccupancy(occupied);
        }
        return Protocols::InteractionModel::Status::InvalidInState;
    }

private:
    Delegate * mDelegate = nullptr;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

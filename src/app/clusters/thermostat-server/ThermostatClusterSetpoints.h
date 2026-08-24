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

#include "DelegateResolution.h"
#include "Setpoints.h"
#include "Temperature.h"
#include "ThermostatClusterAttributes.h"
#include "ThermostatClusterCoolingSetpoints.h"
#include "ThermostatClusterCore.h"
#include "ThermostatClusterHeatingSetpoints.h"
#include "ThermostatClusterSetpointsBase.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeValueEncoder.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/OperationTypes.h>
#include <lib/support/BitMask.h>
#include <lib/support/ReadOnlyBuffer.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class ThermostatClusterCore;

class ThermostatAutoSetpoints
{
public:
    class Delegate
    {
    public:
        Delegate()          = default;
        virtual ~Delegate() = default;

        virtual Protocols::InteractionModel::Status GetMinDeadband(temperature & minDeadband) const;
    };

    ThermostatAutoSetpoints(Delegate & delegate) : mDelegate(delegate) {}

    Protocols::InteractionModel::Status LoadSetpoints(Setpoints & setpoints);

    std::optional<DataModel::ActionReturnStatus> ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder);
    std::optional<DataModel::ActionReturnStatus> WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                AttributeValueDecoder & decoder);
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder);

private:
    Delegate & mDelegate;

    Protocols::InteractionModel::Status LoadDeadband(temperature & minDeadband);
};

template <typename... Delegates>
class ThermostatSetpoints : public ThermostatSetpointsBase
{
public:
    static constexpr bool kHasCooling = detail::kArgsHasDelegate<ThermostatCoolingSetpoints::Delegate, Delegates...>;
    static constexpr bool kHasHeating = detail::kArgsHasDelegate<ThermostatHeatingSetpoints::Delegate, Delegates...>;
    static constexpr bool kHasAuto    = detail::kArgsHasDelegate<ThermostatAutoSetpoints::Delegate, Delegates...>;

    ThermostatSetpoints(ThermostatClusterCore & cluster, Delegates &... delegates) :
        ThermostatSetpointsBase(cluster),
        mCooling(detail::MakeFeature<kHasCooling, ThermostatCoolingSetpoints>(std::forward_as_tuple(delegates...), *this)),
        mHeating(detail::MakeFeature<kHasHeating, ThermostatHeatingSetpoints>(std::forward_as_tuple(delegates...), *this)),
        mAuto(detail::MakeFeature<kHasAuto, ThermostatAutoSetpoints>(std::forward_as_tuple(delegates...)))
    {}

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
    {
        if constexpr (kHasCooling)
        {
            ReturnErrorOnFailure(mCooling.Attributes(path, builder));
        }
        if constexpr (kHasHeating)
        {
            ReturnErrorOnFailure(mHeating.Attributes(path, builder));
        }
        if constexpr (kHasAuto)
        {
            ReturnErrorOnFailure(mAuto.Attributes(path, builder));
        }
        return CHIP_NO_ERROR;
    }

    std::optional<DataModel::ActionReturnStatus> ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder)
    {
        if constexpr (kHasCooling)
        {
            if (auto status = mCooling.ReadAttribute(request, encoder))
            {
                return *status;
            }
        }

        if constexpr (kHasHeating)
        {
            if (auto status = mHeating.ReadAttribute(request, encoder))
            {
                return *status;
            }
        }

        if constexpr (kHasAuto)
        {
            if (auto status = mAuto.ReadAttribute(request, encoder))
            {
                return *status;
            }
        }
        return std::nullopt;
    }

    std::optional<DataModel::ActionReturnStatus> WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                AttributeValueDecoder & decoder)
    {

        if constexpr (kHasAuto)
        {
            if (auto status = mAuto.WriteAttribute(request, decoder))
            {
                return *status;
            }
        }
        bool hasAttribute = false;
        if constexpr (kHasCooling)
        {
            hasAttribute = mCooling.HasAttribute(request.path.mAttributeId);
        }
        if constexpr (kHasHeating)
        {
            hasAttribute = hasAttribute || mHeating.HasAttribute(request.path.mAttributeId);
        }
        if (!hasAttribute)
        {
            return std::nullopt;
        }

        Setpoints setpoints = GetSetpoints();

        SetpointAttributes changedAttributes;
        std::optional<DataModel::ActionReturnStatus> status;
        if constexpr (kHasCooling)
        {
            status = mCooling.WriteAttribute(request, decoder, setpoints, changedAttributes);
        }
        if constexpr (kHasHeating)
        {
            if (!status)
            {
                status = mHeating.WriteAttribute(request, decoder, setpoints, changedAttributes);
            }
        }

        if (!status)
        {
            return status;
        }
        if (status == Protocols::InteractionModel::Status::Success)
        {
            return SaveSetpoints(setpoints, changedAttributes);
        }
        return status;
    }

    Protocols::InteractionModel::Status LoadSetpoints(Setpoints & setpoints) override
    {
        if constexpr (kHasCooling)
        {
            if (auto status = mCooling.LoadSetpoints(setpoints); status != Protocols::InteractionModel::Status::Success)
            {
                return status;
            }
        }

        if constexpr (kHasHeating)
        {
            if (auto status = mHeating.LoadSetpoints(setpoints); status != Protocols::InteractionModel::Status::Success)
            {
                return status;
            }
        }
        if constexpr (kHasAuto)
        {
            if (auto status = mAuto.LoadSetpoints(setpoints); status != Protocols::InteractionModel::Status::Success)
            {
                return status;
            }
        }
        return Protocols::InteractionModel::Status::Success;
    }

    Setpoints GetSetpoints() override
    {
        Setpoints setpoints = ThermostatSetpointsBase::GetSetpoints();
        if constexpr (kHasCooling)
        {
            mCooling.LoadSetpoints(setpoints);
        }
        if constexpr (kHasHeating)
        {
            mHeating.LoadSetpoints(setpoints);
        }
        if constexpr (kHasAuto)
        {
            mAuto.LoadSetpoints(setpoints);
        }
        return setpoints;
    }

    Protocols::InteractionModel::Status SaveSetpoints(const Setpoints & setpoints, SetpointAttributes changedAttributes) override
    {
        Setpoints currentSetpoints = GetSetpoints();
        if constexpr (kHasCooling)
        {
            if (auto status = mCooling.SaveSetpoints(currentSetpoints, setpoints, changedAttributes);
                status != Protocols::InteractionModel::Status::Success)
            {
                return status.GetStatusCode().GetStatus();
            }
        }
        if constexpr (kHasHeating)
        {
            if (auto status = mHeating.SaveSetpoints(currentSetpoints, setpoints, changedAttributes);
                status != Protocols::InteractionModel::Status::Success)
            {
                return status.GetStatusCode().GetStatus();
            }
        }
        NotifyAttributesChanged(changedAttributes);
        return Protocols::InteractionModel::Status::Success;
    }

private:
    CHIP_NO_UNIQUE_ADDRESS std::conditional_t<kHasCooling, ThermostatCoolingSetpoints, std::monostate> mCooling;
    CHIP_NO_UNIQUE_ADDRESS std::conditional_t<kHasHeating, ThermostatHeatingSetpoints, std::monostate> mHeating;
    CHIP_NO_UNIQUE_ADDRESS std::conditional_t<kHasAuto, ThermostatAutoSetpoints, std::monostate> mAuto;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

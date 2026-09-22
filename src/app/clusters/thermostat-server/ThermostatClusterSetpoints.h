/**
 *    Copyright (c) 2026 Project CHIP Authors
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
#include "ThermostatClusterBase.h"
#include "ThermostatClusterCoolingSetpoints.h"
#include "ThermostatClusterHeatingSetpoints.h"
#include "ThermostatClusterSetpointsBase.h"
#include "lib/core/CHIPError.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/AttributeValueEncoder.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/OperationTypes.h>
#include <clusters/Thermostat/Metadata.h>
#include <lib/support/BitMask.h>
#include <lib/support/ReadOnlyBuffer.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class ThermostatAutoSetpoints
{
public:
    class Delegate
    {
    public:
        Delegate()          = default;
        virtual ~Delegate() = default;

        virtual CHIP_ERROR Startup(ServerClusterContext & context);
        virtual void Shutdown(ClusterShutdownType type);

        virtual Protocols::InteractionModel::Status GetMinDeadband(temperature & minDeadband) const;
    };

    CHIP_ERROR Startup(ServerClusterContext & context);
    void Shutdown(ClusterShutdownType type);

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

    ThermostatSetpoints(ThermostatClusterBase & cluster, Delegates &... delegates) :
        ThermostatSetpointsBase(cluster),
        mCooling(detail::MakeFeature<kHasCooling, ThermostatCoolingSetpoints>(*this, std::forward_as_tuple(delegates...))),
        mHeating(detail::MakeFeature<kHasHeating, ThermostatHeatingSetpoints>(*this, std::forward_as_tuple(delegates...))),
        mAuto(detail::MakeFeature<kHasAuto, ThermostatAutoSetpoints>(std::forward_as_tuple(delegates...)))
    {}

    CHIP_ERROR Startup(ServerClusterContext & context)
    {
        if constexpr (kHasCooling)
        {
            ReturnErrorOnFailure(mCooling.Startup(context));
        }
        if constexpr (kHasHeating)
        {
            ReturnErrorOnFailure(mHeating.Startup(context));
        }
        if constexpr (kHasAuto)
        {
            ReturnErrorOnFailure(mAuto.Startup(context));
        }
        return CHIP_NO_ERROR;
    }

    void Shutdown(ClusterShutdownType type)
    {
        if constexpr (kHasCooling)
        {
            mCooling.Shutdown(type);
        }
        if constexpr (kHasHeating)
        {
            mHeating.Shutdown(type);
        }
        if constexpr (kHasAuto)
        {
            mAuto.Shutdown(type);
        }
    }

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

        const auto & optionalAttributes                              = GetOptionalAttributes();
        const AttributeListBuilder::OptionalAttributeEntry entries[] = {
            { optionalAttributes.SetpointChangeSource, Attributes::SetpointChangeSource::kMetadataEntry },
            { optionalAttributes.SetpointChangeAmount, Attributes::SetpointChangeAmount::kMetadataEntry },
            { optionalAttributes.SetpointChangeSourceTimestamp, Attributes::SetpointChangeSourceTimestamp::kMetadataEntry },
        };
        ReturnErrorOnFailure(AppendOptionalAttributes(builder, Span(entries)));

        return CHIP_NO_ERROR;
    }

    std::optional<DataModel::ActionReturnStatus> ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder)
    {
        switch (request.path.mAttributeId)
        {
        case Attributes::SetpointChangeSource::Id:
            return encoder.Encode(mSetpointChangeSource);
        case Attributes::SetpointChangeAmount::Id:
            return encoder.Encode(mSetpointChangeAmount);
        case Attributes::SetpointChangeSourceTimestamp::Id:
            return encoder.Encode(mSetpointChangeSourceTimestamp);
        default:
            break;
        }

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
            hasAttribute = mCooling.HandlesAttribute(request.path.mAttributeId);
        }
        if constexpr (kHasHeating)
        {
            hasAttribute = hasAttribute || mHeating.HandlesAttribute(request.path.mAttributeId);
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
            bool isOperationalSetpointWrite = IsOperationalSetpointAttribute(request.path.mAttributeId);
            return SaveSetpoints(setpoints, changedAttributes, isOperationalSetpointWrite,
                                 isOperationalSetpointWrite ? std::make_optional(request.path.mAttributeId) : std::nullopt);
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

    Protocols::InteractionModel::Status SaveSetpoints(const Setpoints & setpoints, SetpointAttributes changedAttributes,
                                                      bool initiatedByOperationalSetpointWrite,
                                                      std::optional<AttributeId> initiatingAttributeId) override
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
        UpdateSetpointChangeAttributes(currentSetpoints, setpoints, changedAttributes, initiatedByOperationalSetpointWrite,
                                       initiatingAttributeId);
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

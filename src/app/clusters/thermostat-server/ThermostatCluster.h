/**
 *    Copyright (c) 2024-2026 Project CHIP Authors
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
#include "ThermostatClusterAtomic.h"
#include "ThermostatClusterBase.h"
#include "ThermostatClusterHold.h"
#include "ThermostatClusterOccupancy.h"
#include "ThermostatClusterPresets.h"
#include "ThermostatClusterSetpoints.h"
#include "ThermostatClusterSuggestions.h"
#include <clusters/Thermostat/Metadata.h>
#include <type_traits>
#include <variant>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

/**
 * @brief Code-driven implementation of the Matter Thermostat Server Cluster.
 *
 * `ThermostatCluster` is a variadic class template that configures cluster features,
 * internal state, and storage at compile time based on the delegate types passed to it.
 *
 * ### How the Template Works:
 * - **Delegate-Driven Feature Selection**:
 *   Supported features (Heating, Cooling, Presets, Hold, Suggestions, Occupancy) are enabled
 *   if and only if a corresponding delegate interface (e.g. `ThermostatPresets::Delegate`) is
 *   included in the `Delegates...` parameter pack. The order of delegates is not important, but
 *   the delegate list must include `ThermostatDelegate`.
 *
 * ### Usage Example:
 * @code
 *   // Class Template Argument Deduction (CTAD) infers `Delegates...` automatically:
 *   ThermostatCluster cluster(
 *       endpointId,
 *       features,
 *       config,
 *       thermostatDelegate,
 *       heatingDelegate,
 *       coolingDelegate,
 *       presetsDelegate
 *   );
 * @endcode
 *
 * @tparam Delegates Parameter pack of delegate references implementing cluster feature interfaces.
 *                   Must include `Thermostat::Delegate` and at least one setpoint delegate.
 */
template <typename... Delegates>
class ThermostatCluster : public ThermostatClusterBase, public AtomicWriteSession::Delegate
{
public:
    static constexpr bool kHasHeating          = detail::kArgsHasDelegate<ThermostatHeatingSetpoints::Delegate, Delegates...>;
    static constexpr bool kHasCooling          = detail::kArgsHasDelegate<ThermostatCoolingSetpoints::Delegate, Delegates...>;
    static constexpr bool kHasPresets          = detail::kArgsHasDelegate<ThermostatPresets::Delegate, Delegates...>;
    static constexpr bool kHasHold             = detail::kArgsHasDelegate<ThermostatHold::Delegate, Delegates...>;
    static constexpr bool kHasSuggestions      = detail::kArgsHasDelegate<ThermostatSuggestions::Delegate, Delegates...>;
    static constexpr bool kHasOccupancy        = detail::kArgsHasDelegate<ThermostatOccupancy::Delegate, Delegates...>;
    static constexpr bool kRequiresAtomicWrite = kHasPresets;

    static_assert(!kHasSuggestions || kHasPresets, "Suggestions feature requires Presets feature");
    static_assert(kHasHeating || kHasCooling, "Thermostat cluster must implement either heating or cooling");

    ThermostatCluster(EndpointId aEndpointId, BitFlags<Thermostat::Feature> features, const Config & config,
                      Delegates &... delegates) :
        ThermostatClusterBase(aEndpointId, features, config, detail::FindDelegate<Thermostat::Delegate>(delegates...)),
        mSetpoints(*this, delegates...), mAtomicWriteSession(detail::MakeAtomicWriteSession<kRequiresAtomicWrite>(
                                             *this, config.mTimerDelegate, mDelegate.GetFabricTable())),
        mHold(detail::MakeFeature<kHasHold, ThermostatHold>(*this, std::forward_as_tuple(delegates...))),
        mPresets(
            detail::MakeFeature<kHasPresets, ThermostatPresets>(*this, mAtomicWriteSession, std::forward_as_tuple(delegates...))),
        mSuggestions(
            detail::MakeFeature<kHasSuggestions, ThermostatSuggestions>(*this, mPresets, std::forward_as_tuple(delegates...))),
        mOccupancy(detail::MakeFeature<kHasOccupancy, ThermostatOccupancy>(*this, std::forward_as_tuple(delegates...)))
    {
        static_assert(sizeof...(Delegates) > 0, "ThermostatCluster requires at least one delegate");
        static_assert(detail::kArgsHasDelegate<Thermostat::Delegate, Delegates...>,
                      "Missing Thermostat::Delegate in constructor arguments");
    }

    CHIP_ERROR Startup(ServerClusterContext & context) override
    {
        ReturnErrorOnFailure(ThermostatClusterBase::Startup(context));
        ReturnErrorOnFailure(mDelegate.Startup(context));
        if constexpr (kRequiresAtomicWrite)
        {
            mAtomicWriteSession.Startup();
        }
        ReturnErrorOnFailure(mSetpoints.Startup(context));
        if constexpr (kHasHold)
        {
            ReturnErrorOnFailure(mHold.Startup(context));
        }
        return CHIP_NO_ERROR;
    }

    void Shutdown(ClusterShutdownType type) override
    {
        mDelegate.Shutdown(type);
        if constexpr (kRequiresAtomicWrite)
        {
            mAtomicWriteSession.Shutdown();
        }
        ThermostatClusterBase::Shutdown(type);
    }

    bool IsOccupied() const override
    {
        if constexpr (kHasOccupancy)
        {
            return mOccupancy.IsOccupied();
        }
        return ThermostatClusterBase::IsOccupied();
    }

    template <bool Cond = kHasOccupancy, typename std::enable_if_t<Cond, int> = 0>
    Protocols::InteractionModel::Status SetOccupancy(BitMask<OccupancyBitmap> occupied)
    {
        return mOccupancy.SetOccupancy(occupied);
    }

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        if (auto status = mSetpoints.ReadAttribute(request, encoder))
        {
            return *status;
        }
        if constexpr (kHasHold)
        {
            if (auto status = mHold.ReadAttribute(request, encoder))
            {
                return *status;
            }
        }
        if constexpr (kHasOccupancy)
        {
            if (auto status = mOccupancy.ReadAttribute(request, encoder))
            {
                return *status;
            }
        }
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.ReadAttribute(request, encoder))
            {
                return *status;
            }
        }
        if constexpr (kHasSuggestions)
        {
            if (auto status = mSuggestions.ReadAttribute(request, encoder))
            {
                return *status;
            }
        }
        return ThermostatClusterBase::ReadAttribute(request, encoder);
    }

    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override
    {
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.WriteAttribute(request, decoder))
            {
                return *status;
            }
        }
        if constexpr (kRequiresAtomicWrite)
        {
            auto & subjectDescriptor = decoder.GetSubjectDescriptor();
            if (mAtomicWriteSession.InAtomicWrite(subjectDescriptor))
            {
                ChipLogError(Zcl, "Can not write to non-atomic attribute " ChipLogFormatMEI " during atomic write",
                             ChipLogValueMEI(request.path.mAttributeId));
                return Protocols::InteractionModel::Status::InvalidInState;
            }
        }
        if (auto status = mSetpoints.WriteAttribute(request, decoder))
        {
            if constexpr (kHasPresets)
            {
                if (status->IsSuccess() && IsActiveSetpoint(request.path.mAttributeId))
                {
                    ChipLogProgress(Zcl, "Setting active preset to null");
                    mPresets.SetActivePreset(DataModel::NullNullable);
                }
            }
            return *status;
        }
        if constexpr (kHasHold)
        {
            if (auto status = mHold.WriteAttribute(request, decoder))
            {
                return *status;
            }
        }
        return ThermostatClusterBase::WriteAttribute(request, decoder);
    }

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override
    {
        if (auto status = mSetpoints.InvokeCommand(request, input_arguments, handler))
        {
            return status;
        }
        if constexpr (kRequiresAtomicWrite)
        {
            bool handled = false;
            if (auto status = mAtomicWriteSession.InvokeCommand(request, input_arguments, handler, handled))
            {
                return status;
            }
            if (handled)
            {
                return std::nullopt;
            }
        }
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.InvokeCommand(request, input_arguments, handler))
            {
                return status;
            }
        }
        if constexpr (kHasSuggestions)
        {
            bool handled = false;
            if (auto status = mSuggestions.InvokeCommand(request, input_arguments, handler, handled))
            {
                return status;
            }
            if (handled)
            {
                return std::nullopt;
            }
        }
        return ThermostatClusterBase::InvokeCommand(request, input_arguments, handler);
    }

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override
    {
        if constexpr (kRequiresAtomicWrite)
        {
            ReturnErrorOnFailure(builder.AppendElements({ Commands::AtomicRequest::kMetadataEntry }));
        }
        if constexpr (kHasPresets)
        {
            ReturnErrorOnFailure(builder.AppendElements({ Commands::SetActivePresetRequest::kMetadataEntry }));
        }
        if constexpr (kHasSuggestions)
        {
            ReturnErrorOnFailure(builder.AppendElements(
                { Commands::AddThermostatSuggestion::kMetadataEntry, Commands::RemoveThermostatSuggestion::kMetadataEntry }));
        }
        return ThermostatClusterBase::AcceptedCommands(path, builder);
    }

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override
    {
        if constexpr (kRequiresAtomicWrite)
        {
            ReturnErrorOnFailure(builder.AppendElements({ Commands::AtomicResponse::Id }));
        }
        if constexpr (kHasSuggestions)
        {
            ReturnErrorOnFailure(builder.AppendElements({ Commands::AddThermostatSuggestionResponse::Id }));
        }
        return ThermostatClusterBase::GeneratedCommands(path, builder);
    }

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override
    {
        ReturnErrorOnFailure(ThermostatClusterBase::Attributes(path, builder));
        ReturnErrorOnFailure(mSetpoints.Attributes(path, builder));
        if constexpr (kHasHold)
        {
            ReturnErrorOnFailure(mHold.Attributes(path, builder));
        }
        if constexpr (kHasPresets)
        {
            ReturnErrorOnFailure(mPresets.Attributes(path, builder));
        }
        if constexpr (kHasSuggestions)
        {
            ReturnErrorOnFailure(mSuggestions.Attributes(path, builder));
        }
        if constexpr (kHasOccupancy)
        {
            ReturnErrorOnFailure(mOccupancy.Attributes(path, builder));
        }
        return CHIP_NO_ERROR;
    }

    Setpoints GetSetpoints() override
    {
        Setpoints setpoints;
        setpoints.autoSupported      = mFeatures.Has(Feature::kAutoMode);
        setpoints.heatSupported      = mFeatures.Has(Feature::kHeating);
        setpoints.coolSupported      = mFeatures.Has(Feature::kCooling);
        setpoints.occupancySupported = mFeatures.Has(Feature::kOccupancy);
        mSetpoints.LoadSetpoints(setpoints);
        return setpoints;
    }

    Protocols::InteractionModel::Status OnAtomicWriteBegin(AttributeId attributeId) override
    {
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.OnAtomicWriteBegin(attributeId))
            {
                return *status;
            }
        }
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status OnAtomicWritePrecommit(AttributeId attributeId) override
    {
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.OnAtomicWritePrecommit(attributeId))
            {
                return *status;
            }
        }
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status OnAtomicWriteCommit(AttributeId attributeId) override
    {
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.OnAtomicWriteCommit(attributeId))
            {
                return *status;
            }
        }
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status OnAtomicWriteRollback(AttributeId attributeId) override
    {
        if constexpr (kHasPresets)
        {
            if (auto status = mPresets.OnAtomicWriteRollback(attributeId))
            {
                return *status;
            }
        }
        return Protocols::InteractionModel::Status::Success;
    }

    std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(chip::AttributeId attributeId) override
    {
        if constexpr (kHasPresets)
        {
            if (auto timeout = mPresets.GetMaxAtomicWriteTimeout(attributeId))
            {
                return timeout;
            }
        }
        return std::nullopt;
    }

    bool HasAttribute(chip::AttributeId attributeId) override
    {
        switch (attributeId)
        {
        case Attributes::PresetTypes::Id:
        case Attributes::NumberOfPresets::Id:
        case Attributes::ActivePresetHandle::Id:
        case Attributes::Presets::Id:
            return mFeatures.Has(Feature::kPresets);
        case Attributes::ScheduleTypes::Id:
        case Attributes::NumberOfSchedules::Id:
        case Attributes::NumberOfScheduleTransitions::Id:
        case Attributes::NumberOfScheduleTransitionPerDay::Id:
        case Attributes::ActiveScheduleHandle::Id:
        case Attributes::Schedules::Id:
            return mFeatures.Has(Feature::kMatterScheduleConfiguration);
        case Attributes::MaxThermostatSuggestions::Id:
        case Attributes::ThermostatSuggestions::Id:
        case Attributes::CurrentThermostatSuggestion::Id:
        case Attributes::ThermostatSuggestionNotFollowingReason::Id:
            return mFeatures.Has(Feature::kThermostatSuggestions);
        default:
            return ThermostatClusterBase::HasAttribute(attributeId);
        }
    }

private:
    ThermostatSetpoints<Delegates...> mSetpoints;
    CHIP_NO_UNIQUE_ADDRESS std::conditional_t<kRequiresAtomicWrite, AtomicWriteSession, std::monostate> mAtomicWriteSession;
    CHIP_NO_UNIQUE_ADDRESS std::conditional_t<kHasHold, ThermostatHold, std::monostate> mHold;
    CHIP_NO_UNIQUE_ADDRESS std::conditional_t<kHasPresets, ThermostatPresets, std::monostate> mPresets;
    CHIP_NO_UNIQUE_ADDRESS std::conditional_t<kHasSuggestions, ThermostatSuggestions, std::monostate> mSuggestions;
    CHIP_NO_UNIQUE_ADDRESS std::conditional_t<kHasOccupancy, ThermostatOccupancy, std::monostate> mOccupancy;
};

/**
 * Deduce the template parameters for ThermostatCluster from the arguments.
 */
template <typename... DelegateArgs>
ThermostatCluster(EndpointId, BitFlags<Thermostat::Feature>, const ThermostatClusterBase::Config &, DelegateArgs &...)
    -> ThermostatCluster<std::decay_t<DelegateArgs>...>;

/**
 * An alias for a ThermostatCluster with all features enabled and all delegate types implemented.
 */
using FullFeaturedThermostatCluster =
    ThermostatCluster<Thermostat::Delegate, ThermostatHeatingSetpoints::Delegate, ThermostatCoolingSetpoints::Delegate,
                      ThermostatAutoSetpoints::Delegate, ThermostatHold::Delegate, ThermostatPresets::Delegate,
                      ThermostatSuggestions::Delegate, ThermostatOccupancy::Delegate>;

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

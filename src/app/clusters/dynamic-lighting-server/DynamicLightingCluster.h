/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/data-model/Nullable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/DynamicLighting/Structs.h>
#include <lib/support/Span.h>

namespace chip::app::Clusters {

class DynamicLightingDelegate
{
public:
    virtual ~DynamicLightingDelegate() = default;

    /**
     * Returns the effects supported on the endpoint. The list and the label spans it refers to must stay
     * valid and unchanged while the cluster is started.
     *
     * Startup() fails if an entry violates the EffectStruct constraints or if two entries share an
     * EffectID or a Label. An empty list is accepted so that a cluster can be registered before an
     * application provides effects.
     */
    virtual Span<const DynamicLighting::Structs::EffectStruct::Type> GetAvailableEffects() = 0;

    /**
     * Starts the effect. The cluster has already checked that 1 <= speed <= effect.maxSpeed.
     * The cluster calls StopEffect() before this call if another effect is running.
     */
    virtual CHIP_ERROR StartEffect(const DynamicLighting::Structs::EffectStruct::Type & effect, uint16_t speed) = 0;

    /** Stops the running effect. Called only when an effect is running. */
    virtual void StopEffect() = 0;

    /** Applies a new speed (1 <= speed <= maxSpeed of the running effect) to the running effect. */
    virtual CHIP_ERROR SetEffectSpeed(uint16_t speed) = 0;
};

class DynamicLightingCluster : public DefaultServerCluster
{
public:
    /** The delegate must outlive the cluster. */
    DynamicLightingCluster(EndpointId endpointId, DynamicLightingDelegate & delegate);

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

private:
    DataModel::ActionReturnStatus StartEffect(TLV::TLVReader & input_arguments);
    DataModel::ActionReturnStatus StopEffect();
    const DynamicLighting::Structs::EffectStruct::Type * FindEffect(uint16_t effectId);

    DynamicLightingDelegate & mDelegate;
    DataModel::Nullable<uint16_t> mCurrentEffectID;
    DataModel::Nullable<uint16_t> mCurrentSpeed;
};

} // namespace chip::app::Clusters

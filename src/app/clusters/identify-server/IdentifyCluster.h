/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/TimerDelegates.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/DefaultServerCluster.h>

namespace chip::app::Clusters {

class IdentifyCluster;

class IdentifyDelegate
{
public:
    virtual ~IdentifyDelegate() = default;

    /**
     * @brief Called when identification starts.
     * @param cluster The IdentifyCluster instance.
     */
    virtual void OnIdentifyStart(IdentifyCluster & cluster) = 0;

    /**
     * @brief Called when identification stops.
     * @param cluster The IdentifyCluster instance.
     */
    virtual void OnIdentifyStop(IdentifyCluster & cluster) = 0;

    /**
     * @brief Called to trigger an effect.
     * @param cluster The IdentifyCluster instance from which to get effect information.
     */
    virtual void OnTriggerEffect(IdentifyCluster & cluster) = 0;

    /**
     * @brief Checks if the TriggerEffect command is enabled.
     * @return True if the TriggerEffect command is enabled, false otherwise.
     */
    virtual bool IsTriggerEffectEnabled() const = 0;
};

class IdentifyCluster : public DefaultServerCluster, public reporting::TimerContext
{
public:
    /**
     * @brief Configuration struct for IdentifyCluster.
     *
     * This struct is used to configure an IdentifyCluster instance. It requires mandatory parameters
     * in its constructor and provides fluent-style `With...()` methods for setting optional parameters.
     *
     * Example:
     * @code
     * IdentifyCluster cluster(
     *     IdentifyCluster::Config(kEndpointId, myTimerDelegate)
     *         .WithDelegate(&myIdentifyDelegate)
     * );
     *
     * Use `DefaultTimerDelegate` if you don't need a custom timer delegate implementation.
     * @endcode
     */
    struct Config
    {
        constexpr Config(EndpointId endpoint, reporting::ReportScheduler::TimerDelegate & delegate) :
            endpointId(endpoint), timerDelegate(delegate)
        {}
        constexpr Config & WithIdentifyType(Identify::IdentifyTypeEnum type)
        {
            identifyType = type;
            return *this;
        }
        constexpr Config & WithDelegate(IdentifyDelegate * delegate)
        {
            identifyDelegate = delegate;
            return *this;
        }
        constexpr Config & WithEffectIdentifier(Identify::EffectIdentifierEnum effect)
        {
            effectIdentifier = effect;
            return *this;
        }
        constexpr Config & WithEffectVariant(Identify::EffectVariantEnum variant)
        {
            effectVariant = variant;
            return *this;
        }

        EndpointId endpointId;
        Identify::IdentifyTypeEnum identifyType = Identify::IdentifyTypeEnum::kNone;
        reporting::ReportScheduler::TimerDelegate & timerDelegate;
        IdentifyDelegate * identifyDelegate             = nullptr;
        Identify::EffectIdentifierEnum effectIdentifier = Identify::EffectIdentifierEnum::kBlink;
        Identify::EffectVariantEnum effectVariant       = Identify::EffectVariantEnum::kDefault;
    };

    /**
     * @brief Constructs a new IdentifyCluster object.
     *
     * @param config The configuration object for the cluster.
     */
    IdentifyCluster(const Config & config);

    /**
     * @brief Implementation of ServerClusterInterface methods.
     */
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    /**
     * @brief Called when the identify timer fires.
     */
    void TimerFired() override;

    Identify::EffectIdentifierEnum GetEffectIdentifier() const { return mEffectIdentifier; }
    Identify::EffectVariantEnum GetEffectVariant() const { return mEffectVariant; }
    Identify::IdentifyTypeEnum GetIdentifyType() const { return mIdentifyType; }
    uint16_t GetIdentifyTime() const { return mIdentifyTime; }

private:
    uint16_t mIdentifyTime;
    Identify::IdentifyTypeEnum mIdentifyType;
    IdentifyDelegate * mIdentifyDelegate;
    Identify::EffectIdentifierEnum mEffectIdentifier;
    Identify::EffectVariantEnum mEffectVariant;
    reporting::ReportScheduler::TimerDelegate & mTimerDelegate;

    enum class IdentifyTimeChangeSource
    {
        kClient,
        kTimer,
    };

    /**
     * @brief Helper method to handle changes to the IdentifyTime attribute.
     *
     * This method is called when the IdentifyTime attribute changes, either due to a client write,
     * a command invocation, or a timer countdown. It manages the start/stop of identification
     * callbacks and timer scheduling based on the new IdentifyTime value.
     *
     * @param source The source of the attribute change.
     * @param newTime The new value of the IdentifyTime attribute.
     */
    DataModel::ActionReturnStatus SetIdentifyTime(IdentifyTimeChangeSource source, uint16_t newTime);
};

} // namespace chip::app::Clusters

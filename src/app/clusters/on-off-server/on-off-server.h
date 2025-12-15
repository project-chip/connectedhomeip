/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af-types.h>
#include <app/util/basic-types.h>
#include <platform/CHIPDeviceConfig.h>
#include <protocols/interaction_model/StatusCode.h>

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
#include <app/clusters/scenes-server/SceneTable.h>
#endif

/**********************************************************
 * Defines and Macros
 *********************************************************/

static constexpr chip::System::Clock::Milliseconds32 ON_OFF_UPDATE_TIME_MS = chip::System::Clock::Milliseconds32(100);

static constexpr uint16_t MIN_ON_OFF_TIME_VALUE = 1;
static constexpr uint16_t MAX_ON_OFF_TIME_VALUE = 0xFFFF;

/**
 * @brief
 *
 */
class OnOffServer
{
public:
    using Feature = chip::app::Clusters::OnOff::Feature;

    /**********************************************************
     * Functions Definitions
     *********************************************************/

    static OnOffServer & Instance();

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    chip::scenes::SceneHandler * GetSceneHandler();
#endif

    bool offCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath);
    bool onCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath);
    bool toggleCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath);
    void initOnOffServer(chip::EndpointId endpoint);
    bool offWithEffectCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                              const chip::app::Clusters::OnOff::Commands::OffWithEffect::DecodableType & commandData);
    bool OnWithRecallGlobalSceneCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath);
    bool OnWithTimedOffCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                               const chip::app::Clusters::OnOff::Commands::OnWithTimedOff::DecodableType & commandData);
    void updateOnOffTimeCommand(chip::EndpointId endpoint);
    chip::Protocols::InteractionModel::Status getOnOffValue(chip::EndpointId endpoint, bool * currentOnOffValue);
    chip::Protocols::InteractionModel::Status setOnOffValue(chip::EndpointId endpoint, chip::CommandId command,
                                                            bool initiatedByLevelChange);
    chip::Protocols::InteractionModel::Status getOnOffValueForStartUp(chip::EndpointId endpoint, bool & onOffValueForStartUp);

    bool HasFeature(chip::EndpointId endpoint, Feature feature);
    inline bool SupportsLightingApplications(chip::EndpointId endpointId) { return HasFeature(endpointId, Feature::kLighting); }

    void cancelEndpointTimerCallback(chip::EndpointId endpoint);

private:
    /**********************************************************
     * Functions Definitions
     *********************************************************/

#ifndef IGNORE_ON_OFF_CLUSTER_START_UP_ON_OFF
    bool areStartUpOnOffServerAttributesNonVolatile(chip::EndpointId endpoint);
#endif
    EmberEventControl * getEventControl(chip::EndpointId endpoint, const chip::Span<EmberEventControl> & eventControlArray);
    EmberEventControl * configureEventControl(chip::EndpointId endpoint);

    uint32_t calculateNextWaitTimeMS(void);

    // Matter timer scheduling glue logic
    static void timerCallback(chip::System::Layer *, void * callbackContext);
    void scheduleTimerCallbackMs(EmberEventControl * control, uint32_t delayMs);
    void cancelEndpointTimerCallback(EmberEventControl * control);

    /**********************************************************
     * Attributes Declaration
     *********************************************************/

    static OnOffServer instance;
    chip::System::Clock::Timestamp nextDesiredOnWithTimedOffTimestamp;

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    friend class DefaultOnOffSceneHandler;
#endif
};

struct OnOffEffect
{
    using OffWithEffectTriggerCommand = void (*)(OnOffEffect *);
    using EffectVariantType           = std::underlying_type_t<chip::app::Clusters::OnOff::DelayedAllOffEffectVariantEnum>;
    static_assert(
        std::is_same<EffectVariantType, std::underlying_type_t<chip::app::Clusters::OnOff::DyingLightEffectVariantEnum>>::value,
        "chip::app::Clusters::OnOff::DelayedAllOffEffectVariantEnum and "
        "chip::app::Clusters::OnOff::DyingLightEffectVariantEnum underlying types differ.");

    chip::EndpointId mEndpoint;
    OffWithEffectTriggerCommand mOffWithEffectTrigger = nullptr;
    chip::app::Clusters::OnOff::EffectIdentifierEnum mEffectIdentifier;
    EffectVariantType mEffectVariant;
    OnOffEffect * nextEffect = nullptr;

    OnOffEffect(
        chip::EndpointId endpoint, OffWithEffectTriggerCommand offWithEffectTrigger,
        chip::app::Clusters::OnOff::EffectIdentifierEnum effectIdentifier =
            chip::app::Clusters::OnOff::EffectIdentifierEnum::kDelayedAllOff,

        /*
         * effectVariant's type depends on the effectIdentifier so we don't know the type at compile time.
         * The assertion at the beginning of this method ensures the effect variants share the same base type.
         * Casting to the common base type for more flexibility since the type can be DelayedAllOffEffectVariantEnum or
         * DelayedAllOffEffectVariantEnum
         */
        EffectVariantType = chip::to_underlying(chip::app::Clusters::OnOff::DelayedAllOffEffectVariantEnum::kDelayedOffFastFade));
    ~OnOffEffect();

    bool hasNext() { return this->nextEffect != nullptr; }

    OnOffEffect * next() { return this->nextEffect; }

    void setNext(OnOffEffect * inst) { this->nextEffect = inst; }
};

/**********************************************************
 * Global
 *********************************************************/

/** @brief On/off Cluster Level Control Effect
 *
 * This is called by the framework when the on/off cluster initiates a command
 * that must effect a level control change. The implementation assumes that the
 * client will handle any effect on the On/Off Cluster.
 *
 * @param endpoint   Ver.: always
 * @param newValue   Ver.: always
 */
void emberAfOnOffClusterLevelControlEffectCallback(chip::EndpointId endpoint, bool newValue);

/**********************************************************
 * Callbacks
 *********************************************************/

/** @brief On/off Cluster Server Post Init
 *
 * Following resolution of the On/Off state at startup for this endpoint,
 * perform any additional initialization needed; e.g., synchronize hardware
 * state.
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void emberAfPluginOnOffClusterServerPostInitCallback(chip::EndpointId endpoint);

void onOffWaitTimeOffEventHandler(chip::EndpointId endpoint);

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

using chip::app::Clusters::OnOff::OnOffFeature;

/**********************************************************
 * Defines and Macros
 *********************************************************/

static constexpr chip::System::Clock::Milliseconds32 UPDATE_TIME_MS = chip::System::Clock::Milliseconds32(100);
static constexpr uint16_t TRANSITION_TIME_1S                        = 10;

static constexpr uint16_t MAX_TIME_VALUE = 0xFFFF;
static constexpr uint8_t MIN_TIME_VALUE  = 1;

/**
 * @brief
 *
 */
class OnOffServer
{
public:
    /**********************************************************
     * Functions Definitions
     *********************************************************/

    static OnOffServer & Instance();

    bool offCommand(const chip::app::ConcreteCommandPath & commandPath);
    bool onCommand(const chip::app::ConcreteCommandPath & commandPath);
    bool toggleCommand(const chip::app::ConcreteCommandPath & commandPath);
    void initOnOffServer(chip::EndpointId endpoint);
    bool offWithEffectCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                              const chip::app::Clusters::OnOff::Commands::OffWithEffect::DecodableType & commandData);
    bool OnWithRecallGlobalSceneCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath);
    bool OnWithTimedOffCommand(const chip::app::ConcreteCommandPath & commandPath,
                               const chip::app::Clusters::OnOff::Commands::OnWithTimedOff::DecodableType & commandData);
    void updateOnOffTimeCommand(chip::EndpointId endpoint);
    EmberAfStatus getOnOffValue(chip::EndpointId endpoint, bool * currentOnOffValue);
    EmberAfStatus setOnOffValue(chip::EndpointId endpoint, uint8_t command, bool initiatedByLevelChange);
    EmberAfStatus getOnOffValueForStartUp(chip::EndpointId endpoint, bool & onOffValueForStartUp);

    bool HasFeature(chip::EndpointId endpoint, OnOffFeature feature);
    inline bool SupportsLightingApplications(chip::EndpointId endpointId)
    {
        return HasFeature(endpointId, OnOffFeature::kLighting);
    }

private:
    /**********************************************************
     * Functions Definitions
     *********************************************************/

#ifndef IGNORE_ON_OFF_CLUSTER_START_UP_ON_OFF
    bool areStartUpOnOffServerAttributesNonVolatile(chip::EndpointId endpoint);
#endif
    EmberEventControl * getEventControl(chip::EndpointId endpoint);
    EmberEventControl * configureEventControl(chip::EndpointId endpoint);

    uint32_t calculateNextWaitTimeMS(void);
    /**********************************************************
     * Attributes Declaration
     *********************************************************/

    static OnOffServer instance;
    EmberEventControl eventControls[EMBER_AF_ON_OFF_CLUSTER_SERVER_ENDPOINT_COUNT];
    chip::System::Clock::Timestamp nextDesiredOnWithTimedOffTimestamp;
};

struct OnOffEffect
{
    using OffWithEffectTriggerCommand = void (*)(OnOffEffect *);

    chip::EndpointId mEndpoint;
    OffWithEffectTriggerCommand mOffWithEffectTrigger = nullptr;
    chip::app::Clusters::OnOff::OnOffEffectIdentifier mEffectIdentifier;
    uint8_t mEffectVariant;
    OnOffEffect * nextEffect = nullptr;

    OnOffEffect(
        chip::EndpointId endpoint, OffWithEffectTriggerCommand offWithEffectTrigger,
        chip::app::Clusters::OnOff::OnOffEffectIdentifier effectIdentifier = EMBER_ZCL_ON_OFF_EFFECT_IDENTIFIER_DELAYED_ALL_OFF,

        /*
         * effectVariant's type depends on the effect effectIdentifier so we don't know the type at compile time.
         * Casting to uint8_t for more flexibility since the type can be OnOffDelayedAllOffEffectVariant or
         * OnOffDelayedAllOffEffectVariant
         */
        uint8_t effectVariant = static_cast<uint8_t>(EMBER_ZCL_ON_OFF_DELAYED_ALL_OFF_EFFECT_VARIANT_FADE_TO_OFF_IN_0P8_SECONDS));
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

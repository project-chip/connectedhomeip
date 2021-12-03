/**
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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************/
/**
 * @file
 * @brief Routines for the On-Off plugin, which
 *implements the On-Off server cluster.
 *******************************************************************************
 ******************************************************************************/
#include "on-off-server.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/af-event.h>
#include <app/util/af.h>

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/reporting/reporting.h>

#ifdef EMBER_AF_PLUGIN_SCENES
#include <app/clusters/scenes/scenes.h>
#endif // EMBER_AF_PLUGIN_SCENES

#ifdef EMBER_AF_PLUGIN_ZLL_ON_OFF_SERVER
#include "../zll-on-off-server/zll-on-off-server.h"
#endif

#ifdef EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER
#include "../zll-level-control-server/zll-level-control-server.h"
#endif

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OnOff;

/**********************************************************
 * Attributes Definition
 *********************************************************/

static std::array<OnOffEffect *, EMBER_AF_ON_OFF_CLUSTER_SERVER_ENDPOINT_COUNT> instances = { 0 };
OnOffServer OnOffServer::instance;

/**********************************************************
 * Function definition
 *********************************************************/

static OnOffEffect * inst(EndpointId endpoint);

/**********************************************************
 * OnOff Implementation
 *********************************************************/

OnOffServer & OnOffServer::Instance()
{
    return instance;
}

/** @brief On/off Cluster Set Value
 *
 * This function is called when the on/off value needs to be set, either through
 * normal channels or as a result of a level change.
 *
 * @param endpoint   Ver.: always
 * @param command   Ver.: always
 * @param initiatedByLevelChange   Ver.: always
 */
EmberAfStatus OnOffServer::setOnOffValue(chip::EndpointId endpoint, uint8_t command, bool initiatedByLevelChange)
{
    EmberAfStatus status;
    bool currentValue, newValue;

    emberAfOnOffClusterPrintln("On/Off set value: %x %x", endpoint, command);

    // read current on/off value
    status = Attributes::OnOff::Get(endpoint, &currentValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfOnOffClusterPrintln("ERR: reading on/off %x", status);
        return status;
    }

    // if the value is already what we want to set it to then do nothing
    if ((!currentValue && command == Commands::Off::Id) || (currentValue && command == Commands::On::Id))
    {
        emberAfOnOffClusterPrintln("On/off already set to new value");
        return EMBER_ZCL_STATUS_SUCCESS;
    }

    // we either got a toggle, or an on when off, or an off when on,
    // so we need to swap the value
    newValue = !currentValue;
    emberAfOnOffClusterPrintln("Toggle on/off from %x to %x", currentValue, newValue);

    // the sequence of updating on/off attribute and kick off level change effect should
    // be depend on whether we are turning on or off. If we are turning on the light, we
    // should update the on/off attribute before kicking off level change, if we are
    // turning off the light, we should do the opposite, that is kick off level change
    // before updating the on/off attribute.
    if (newValue) // Set On
    {
        uint16_t onTime = 0;
        Attributes::OnTime::Get(endpoint, &onTime);

        if (onTime == 0)
        {
            emberAfOnOffClusterPrintln("On Command - OffWaitTime :  0");
            Attributes::OffWaitTime::Set(endpoint, 0);

            // Stop timer on the endpoint
            emberEventControlSetInactive(getEventControl(endpoint));
            emberAfOnOffClusterPrintln("On/Toggle Command - Stop Timer");
        }

        Attributes::GlobalSceneControl::Set(endpoint, true);

        // write the new on/off value
        status = Attributes::OnOff::Set(endpoint, newValue);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfOnOffClusterPrintln("ERR: writing on/off %x", status);
            return status;
        }
    }
    else // Set Off
    {
        emberAfOnOffClusterPrintln("Off Command - OnTime :  0");
        Attributes::OnTime::Set(endpoint, 0); // Reset onTime

        // write the new on/off value
        status = Attributes::OnOff::Set(endpoint, newValue);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfOnOffClusterPrintln("ERR: writing on/off %x", status);
            return status;
        }
    }

#ifdef EMBER_AF_PLUGIN_ZLL_ON_OFF_SERVER
    if (initiatedByLevelChange)
    {
        emberAfPluginZllOnOffServerLevelControlZllExtensions(endpoint);
    }
#endif

#ifdef EMBER_AF_PLUGIN_SCENES
    // the scene has been changed (the value of on/off has changed) so
    // the current scene as descibed in the attribute table is invalid,
    // so mark it as invalid (just writes the valid/invalid attribute)
    if (emberAfContainsServer(endpoint, Scenes::Id))
    {
        emberAfScenesClusterMakeInvalidCallback(endpoint);
    }
#endif // EMBER_AF_PLUGIN_SCENES

    // The returned status is based solely on the On/Off cluster.  Errors in the
    // Level Control and/or Scenes cluster are ignored.
    return EMBER_ZCL_STATUS_SUCCESS;
}

void OnOffServer::initOnOffServer(chip::EndpointId endpoint)
{
#ifdef ZCL_USING_ON_OFF_CLUSTER_START_UP_ON_OFF_ATTRIBUTE
    // StartUp behavior relies on OnOff and StartUpOnOff attributes being tokenized.
    if (areStartUpOnOffServerAttributesTokenized(endpoint))
    {
        // Read the StartUpOnOff attribute and set the OnOff attribute as per
        // following from zcl 7 14-0127-20i-zcl-ch-3-general.doc.
        // 3.8.2.2.5	StartUpOnOff Attribute
        // The StartUpOnOff attribute SHALL define the desired startup behavior of a
        // lamp device when it is supplied with power and this state SHALL be
        // reflected in the OnOff attribute.  The values of the StartUpOnOff
        // attribute are listed below.
        // Table 3 46. Values of the StartUpOnOff Attribute
        // Value      Action on power up
        // 0x00       Set the OnOff attribute to 0 (off).
        // 0x01       Set the OnOff attribute to 1 (on).
        // 0x02       If the previous value of the OnOff attribute is equal to 0,
        //            set the OnOff attribute to 1.If the previous value of the OnOff
        //            attribute is equal to 1, set the OnOff attribute to 0 (toggle).
        // 0x03-0xfe  These values are reserved.  No action.
        // 0xff       Set the OnOff attribute to its previous value.

        // Initialize startUpOnOff to No action value 0xFE
        uint8_t startUpOnOff = 0xFE;
        EmberAfStatus status = Attributes::StartUpOnOff::Get(endpoint, &startUpOnOff);
        if (status == EMBER_ZCL_STATUS_SUCCESS)
        {
            // Initialise updated value to 0
            bool updatedOnOff = 0;
            status            = Attributes::OnOff::Get(endpoint, &udpateOnOff);
            if (status == EMBER_ZCL_STATUS_SUCCESS)
            {
                switch (startUpOnOff)
                {
                case EMBER_ZCL_START_UP_ON_OFF_VALUE_SET_TO_OFF:
                    updatedOnOff = 0; // Off
                    break;
                case EMBER_ZCL_START_UP_ON_OFF_VALUE_SET_TO_ON:
                    updatedOnOff = 1; // On
                    break;
                case EMBER_ZCL_START_UP_ON_OFF_VALUE_SET_TO_TOGGLE:
                    updatedOnOff = !updatedOnOff;
                    break;
                case EMBER_ZCL_START_UP_ON_OFF_VALUE_SET_TO_PREVIOUS:
                default:
                    // All other values 0x03- 0xFE are reserved - no action.
                    // When value is 0xFF - update with last value - that is as good as
                    // no action.
                    break;
                }
                status = Attributes::OnOff::Set(endpoint, updatedOnOff);
            }
        }
    }
#endif
    emberAfPluginOnOffClusterServerPostInitCallback(endpoint);
}

bool OnOffServer::offCommand()
{
    EmberAfStatus status = setOnOffValue(emberAfCurrentEndpoint(), Commands::Off::Id, false);
#ifdef EMBER_AF_PLUGIN_ZLL_ON_OFF_SERVER
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPluginZllOnOffServerOffZllExtensions(emberAfCurrentCommand());
    }
#endif
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool OnOffServer::onCommand()
{
    EmberAfStatus status = setOnOffValue(emberAfCurrentEndpoint(), Commands::On::Id, false);

#ifdef EMBER_AF_PLUGIN_ZLL_ON_OFF_SERVER
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPluginZllOnOffServerOnZllExtensions(emberAfCurrentCommand());
    }
#endif

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool OnOffServer::toggleCommand()
{
    EmberAfStatus status = setOnOffValue(emberAfCurrentEndpoint(), Commands::Toggle::Id, false);
#ifdef EMBER_AF_PLUGIN_ZLL_ON_OFF_SERVER
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPluginZllOnOffServerToggleZllExtensions(emberAfCurrentCommand());
    }
#endif
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool OnOffServer::offWithEffectCommand(uint8_t effectId, uint8_t effectVariant)
{
    chip::EndpointId endpoint = emberAfCurrentEndpoint();
    EmberAfStatus status      = EMBER_ZCL_STATUS_SUCCESS;

    bool globalSceneControl = false;
    OnOff::Attributes::GlobalSceneControl::Get(endpoint, &globalSceneControl);

    bool isOnBeforeCommand = false;
    OnOff::Attributes::OnOff::Get(endpoint, &isOnBeforeCommand);

    if (globalSceneControl)
    {
        OnOff::Attributes::GlobalSceneControl::Set(endpoint, false);

        status = setOnOffValue(endpoint, Commands::Off::Id, false);
        Attributes::OnTime::Set(endpoint, 0);
    }
    else
    {
        status = setOnOffValue(endpoint, Commands::Off::Id, false);
    }

    // Only apply effect if OnOff is on
    if (isOnBeforeCommand)
    {
        OnOffEffect * effect = inst(endpoint);

        if (effect != nullptr && effect->mOffWithEffectTrigger != nullptr)
        {
            effect->mEffectIdentifier = effectId;
            effect->mEffectVariant    = effectVariant;

            effect->mOffWithEffectTrigger(effect);
        }
    }

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool OnOffServer::OnWithRecallGlobalSceneCommand()
{
    chip::EndpointId endpoint = emberAfCurrentEndpoint();
    EmberAfStatus status      = EMBER_ZCL_STATUS_SUCCESS;

    bool globalSceneControl = false;
    OnOff::Attributes::GlobalSceneControl::Get(endpoint, &globalSceneControl);

    if (globalSceneControl)
    {
        emberAfSendImmediateDefaultResponse(status);
        return true;
    }

    OnOff::Attributes::GlobalSceneControl::Set(endpoint, true);
    setOnOffValue(endpoint, Commands::On::Id, false);

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool OnOffServer::OnWithTimedOffCommand(BitFlags<OnOffControl> onOffControl, uint16_t onTime, uint16_t offWaitTime)
{
    EmberAfStatus status      = EMBER_ZCL_STATUS_SUCCESS;
    chip::EndpointId endpoint = emberAfCurrentEndpoint();

    bool isOn = false;
    OnOff::Attributes::OnOff::Get(endpoint, &isOn);

    // OnOff is off and the commands is only accepted if on
    if (onOffControl.Has(OnOffControl::kAcceptOnlyWhenOn) && !isOn)
    {
        emberAfSendImmediateDefaultResponse(status);
        return true;
    }

    uint16_t currentOffWaitTime = MAX_TIME_VALUE;
    OnOff::Attributes::OffWaitTime::Get(endpoint, &currentOffWaitTime);

    uint16_t currentOnTime = 0;
    OnOff::Attributes::OnTime::Get(endpoint, &currentOnTime);

    if (currentOffWaitTime > 0 && !isOn)
    {
        uint16_t newOffWaitTime = currentOffWaitTime < offWaitTime ? currentOffWaitTime : offWaitTime;
        OnOff::Attributes::OffWaitTime::Set(endpoint, newOffWaitTime);

        currentOffWaitTime = newOffWaitTime;
    }
    else
    {
        uint16_t newOnTime = currentOnTime > onTime ? currentOnTime : onTime;
        OnOff::Attributes::OnTime::Set(endpoint, newOnTime);

        OnOff::Attributes::OffWaitTime::Set(endpoint, offWaitTime);
        setOnOffValue(endpoint, Commands::On::Id, false);

        currentOnTime      = newOnTime;
        currentOffWaitTime = offWaitTime;
    }

    emberAfOnOffClusterPrintln("On Time:  %d | off wait Time: %d", currentOnTime, currentOffWaitTime);

    if (currentOnTime < MAX_TIME_VALUE && currentOffWaitTime < MAX_TIME_VALUE)
    {
        emberEventControlSetDelayMS(configureEventControl(endpoint), UPDATE_TIME_MS);
    }

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

/**
 * @brief Updates OnOff values after timer is finished
 *
 * @param[in] endpoint endpoint associated with the finished timer
 */
void OnOffServer::updateOnOffTimeCommand(chip::EndpointId endpoint)
{
    emberAfOnOffClusterPrintln("Timer callback - Entering callbackc");

    bool isOn = false;
    OnOff::Attributes::OnOff::Get(endpoint, &isOn);

    if (isOn) // OnOff On case
    {
        // Restart Timer
        emberEventControlSetDelayMS(configureEventControl(endpoint), UPDATE_TIME_MS);

        // Update onTime values
        uint16_t onTime = MIN_TIME_VALUE;
        OnOff::Attributes::OnTime::Get(endpoint, &onTime);
        emberAfOnOffClusterPrintln("Timer callback - On Time:  %d", onTime);

        if (onTime > 0)
        {
            onTime--;
            OnOff::Attributes::OnTime::Set(endpoint, onTime);
        }

        if (onTime == 0)
        {
            emberAfOnOffClusterPrintln("Timer callback - Turning off OnOff");

            OnOff::Attributes::OffWaitTime::Set(endpoint, 0);
            setOnOffValue(endpoint, Commands::Off::Id, false);
        }
    }
    else // OnOff Off Case
    {
        uint16_t offWaitTime = 0;
        OnOff::Attributes::OffWaitTime::Get(endpoint, &offWaitTime);

        // Validate before decreasing counter
        if (offWaitTime > 0)
        {
            offWaitTime--;
            OnOff::Attributes::OffWaitTime::Set(endpoint, offWaitTime);
        }

        emberAfOnOffClusterPrintln("Timer Callback - wait Off Time:  %d", offWaitTime);

        // Validate if necessary to restart timer
        if (offWaitTime > 0)
        {
            // Restart Timer
            emberEventControlSetDelayMS(configureEventControl(endpoint), UPDATE_TIME_MS);
        }
        else
        {
            emberAfOnOffClusterPrintln("Timer  Callback - wait Off Time cycle finished");

            // Stop timer on the endpoint
            emberEventControlSetInactive(getEventControl(endpoint));
        }
    }
}

#ifdef ZCL_USING_ON_OFF_CLUSTER_START_UP_ON_OFF_ATTRIBUTE
bool OnOffServer::areStartUpOnOffServerAttributesTokenized(EndpointId endpoint)
{
    EmberAfAttributeMetadata * metadata;

    metadata = emberAfLocateAttributeMetadata(endpoint, OnOff::Id, Attributes::OnOff::Id, CLUSTER_MASK_SERVER,
                                              EMBER_AF_NULL_MANUFACTURER_CODE);
    if (!emberAfAttributeIsTokenized(metadata))
    {
        return false;
    }

    metadata = emberAfLocateAttributeMetadata(endpoint, OnOff::Id, Attributes::StartUpOnOff::Id, CLUSTER_MASK_SERVER,
                                              EMBER_AF_NULL_MANUFACTURER_CODE);
    if (!emberAfAttributeIsTokenized(metadata))
    {
        return false;
    }

    return true;
}
#endif // ZCL_USING_ON_OFF_CLUSTER_START_UP_ON_OFF_ATTRIBUTE

/**
 * @brief event control object for an endpoint
 *
 * @param[in] endpoint
 * @return EmberEventControl* configured event control
 */
EmberEventControl * OnOffServer::getEventControl(EndpointId endpoint)
{
    uint16_t index = emberAfFindClusterServerEndpointIndex(endpoint, OnOff::Id);
    return &eventControls[index];
}

/**
 * @brief Configures EnventControl callback when using XY colors
 *
 * @param[in] endpoint endpoint to start timer for
 * @return EmberEventControl* configured event control
 */
EmberEventControl * OnOffServer::configureEventControl(EndpointId endpoint)
{
    EmberEventControl * controller = getEventControl(endpoint);

    controller->endpoint = endpoint;
    controller->callback = &onOffWaitTimeOffEventHandler;

    return controller;
}

/**********************************************************
 * OnOffEffect Implementation
 *********************************************************/

static OnOffEffect * inst(EndpointId endpoint)
{
    for (size_t i = 0; i < instances.size(); i++)
    {
        if (nullptr != instances[i] && endpoint == instances[i]->mEndpoint)
        {
            return instances[i];
        }
    }

    return nullptr;
}

static inline void reg(OnOffEffect * inst)
{
    for (size_t i = 0; i < instances.size(); i++)
    {
        if (nullptr == instances[i])
        {
            instances[i] = inst;
            break;
        }
    }
}

static inline void unreg(OnOffEffect * inst)
{
    for (size_t i = 0; i < instances.size(); i++)
    {
        if (inst == instances[i])
        {
            instances[i] = nullptr;
        }
    }
}

OnOffEffect::OnOffEffect(chip::EndpointId endpoint, OffWithEffectTriggerCommand offWithEffectTrigger, uint8_t effectIdentifier,
                         uint8_t effectVariant) :
    mEndpoint(endpoint),
    mOffWithEffectTrigger(offWithEffectTrigger), mEffectIdentifier(effectIdentifier), mEffectVariant(effectVariant)
{
    reg(this);
};

OnOffEffect::~OnOffEffect()
{
    unreg(this);
};

/**********************************************************
 * Callbacks Implementation
 *********************************************************/

bool emberAfOnOffClusterOffCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                    const Commands::Off::DecodableType & commandData)
{
    return OnOffServer::Instance().offCommand();
}

bool emberAfOnOffClusterOnCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                   const Commands::On::DecodableType & commandData)
{
    return OnOffServer::Instance().onCommand();
}

bool emberAfOnOffClusterToggleCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                       const Commands::Toggle::DecodableType & commandData)
{
    return OnOffServer::Instance().toggleCommand();
}

bool emberAfOnOffClusterOffWithEffectCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                              const Commands::OffWithEffect::DecodableType & commandData)
{
    auto & effectId      = commandData.effectId;
    auto & effectVariant = commandData.effectVariant;

    return OnOffServer::Instance().offWithEffectCommand(effectId, effectVariant);
}

bool emberAfOnOffClusterOnWithRecallGlobalSceneCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::OnWithRecallGlobalScene::DecodableType & commandData)
{
    return OnOffServer::Instance().OnWithRecallGlobalSceneCommand();
}

bool emberAfOnOffClusterOnWithTimedOffCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::OnWithTimedOff::DecodableType & commandData)
{
    auto & onOffControl = commandData.onOffControl;
    auto & onTime       = commandData.onTime;
    auto & offWaitTime  = commandData.offWaitTime;

    return OnOffServer::Instance().OnWithTimedOffCommand(onOffControl, onTime, offWaitTime);
}

void emberAfOnOffClusterServerInitCallback(chip::EndpointId endpoint)
{
    OnOffServer::Instance().initOnOffServer(endpoint);
}

void onOffWaitTimeOffEventHandler(chip::EndpointId endpoint)
{
    OnOffServer::Instance().updateOnOffTimeCommand(endpoint);
}

void emberAfPluginOnOffClusterServerPostInitCallback(EndpointId endpoint) {}

void MatterOnOffPluginServerInitCallback() {}

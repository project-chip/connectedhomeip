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
#include <app/data-model/Nullable.h>
#include <app/reporting/reporting.h>
#include <app/util/af-event.h>
#include <app/util/af.h>
#include <app/util/util.h>

#ifdef EMBER_AF_PLUGIN_SCENES
#include <app/clusters/scenes/scenes.h>
#endif // EMBER_AF_PLUGIN_SCENES

#ifdef EMBER_AF_PLUGIN_LEVEL_CONTROL
#include <app/clusters/level-control/level-control.h>
#endif // EMBER_AF_PLUGIN_LEVEL_CONTROL

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OnOff;

/**********************************************************
 * Attributes Definition
 *********************************************************/

static OnOffEffect * firstEffect = nullptr;
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

bool OnOffServer::HasFeature(chip::EndpointId endpoint, OnOffFeature feature)
{
    bool success;
    uint32_t featureMap;
    success = (Attributes::FeatureMap::Get(endpoint, &featureMap) == EMBER_ZCL_STATUS_SUCCESS);

    return success ? ((featureMap & to_underlying(feature)) != 0) : false;
}

EmberAfStatus OnOffServer::getOnOffValue(chip::EndpointId endpoint, bool * currentOnOffValue)
{
    // read current on/off value
    EmberAfStatus status = Attributes::OnOff::Get(endpoint, currentOnOffValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfOnOffClusterPrintln("ERR: reading on/off %x", status);
    }

    emberAfOnOffClusterPrintln("On/Off ep%d value: %d", endpoint, *currentOnOffValue);

    return status;
}

#ifdef EMBER_AF_PLUGIN_LEVEL_CONTROL
static bool LevelControlWithOnOffFeaturePresent(EndpointId endpoint)
{
    if (!emberAfContainsServer(endpoint, LevelControl::Id))
    {
        return false;
    }

    return LevelControlHasFeature(endpoint, LevelControl::LevelControlFeature::kOnOff);
}
#endif // EMBER_AF_PLUGIN_LEVEL_CONTROL

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
        if (SupportsLightingApplications(endpoint))
        {
            uint16_t onTime = 0;
            Attributes::OnTime::Get(endpoint, &onTime);

            if (onTime == 0)
            {
                emberAfOnOffClusterPrintln("On Command - OffWaitTime :  0");
                Attributes::OffWaitTime::Set(endpoint, 0);

                // Stop timer on the endpoint
                EmberEventControl * event = getEventControl(endpoint);
                if (event != nullptr)
                {
                    emberEventControlSetInactive(event);
                    emberAfOnOffClusterPrintln("On/Toggle Command - Stop Timer");
                }
            }

            Attributes::GlobalSceneControl::Set(endpoint, true);
        }

        // write the new on/off value
        status = Attributes::OnOff::Set(endpoint, newValue);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfOnOffClusterPrintln("ERR: writing on/off %x", status);
            return status;
        }

#ifdef EMBER_AF_PLUGIN_LEVEL_CONTROL
        // If initiatedByLevelChange is false, then we assume that the level change
        // ZCL stuff has not happened and we do it here
        if (!initiatedByLevelChange && LevelControlWithOnOffFeaturePresent(endpoint))
        {
            emberAfOnOffClusterLevelControlEffectCallback(endpoint, newValue);
        }
#endif
#ifdef EMBER_AF_PLUGIN_MODE_SELECT
        // If OnMode is not a null value, then change the current mode to it.
        if (emberAfContainsServer(endpoint, ModeSelect::Id) &&
            emberAfContainsAttribute(endpoint, ModeSelect::Id, ModeSelect::Attributes::OnMode::Id))
        {
            ModeSelect::Attributes::OnMode::TypeInfo::Type onMode;
            if (ModeSelect::Attributes::OnMode::Get(endpoint, onMode) == EMBER_ZCL_STATUS_SUCCESS && !onMode.IsNull())
            {
                emberAfOnOffClusterPrintln("Changing Current Mode to %x", onMode.Value());
                status = ModeSelect::Attributes::CurrentMode::Set(endpoint, onMode.Value());
            }
        }
#endif
    }
    else // Set Off
    {
        if (SupportsLightingApplications(endpoint))
        {
            emberAfOnOffClusterPrintln("Off Command - OnTime :  0");
            Attributes::OnTime::Set(endpoint, 0); // Reset onTime
        }

#ifdef EMBER_AF_PLUGIN_LEVEL_CONTROL
        // If initiatedByLevelChange is false, then we assume that the level change
        // ZCL stuff has not happened and we do it here
        if (!initiatedByLevelChange && LevelControlWithOnOffFeaturePresent(endpoint))
        {
            emberAfOnOffClusterLevelControlEffectCallback(endpoint, newValue);
        }
#endif

        // write the new on/off value
        status = Attributes::OnOff::Set(endpoint, newValue);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfOnOffClusterPrintln("ERR: writing on/off %x", status);
            return status;
        }
    }

#ifdef EMBER_AF_PLUGIN_SCENES
    // the scene has been changed (the value of on/off has changed) so
    // the current scene as described in the attribute table is invalid,
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
#ifndef IGNORE_ON_OFF_CLUSTER_START_UP_ON_OFF
    // StartUp behavior relies on OnOff and StartUpOnOff attributes being non-volatile.
    if (SupportsLightingApplications(endpoint) && areStartUpOnOffServerAttributesNonVolatile(endpoint))
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
        // 0xff      This value cannot happen.
        // null       Set the OnOff attribute to its previous value.

        bool onOffValueForStartUp = 0;
        EmberAfStatus status      = getOnOffValueForStartUp(endpoint, onOffValueForStartUp);
        if (status == EMBER_ZCL_STATUS_SUCCESS)
        {
            status = setOnOffValue(endpoint, onOffValueForStartUp, false);
        }

#ifdef EMBER_AF_PLUGIN_MODE_SELECT
        // If OnMode is not a null value, then change the current mode to it.
        if (onOffValueForStartUp && emberAfContainsServer(endpoint, ModeSelect::Id) &&
            emberAfContainsAttribute(endpoint, ModeSelect::Id, ModeSelect::Attributes::OnMode::Id))
        {
            ModeSelect::Attributes::OnMode::TypeInfo::Type onMode;
            if (ModeSelect::Attributes::OnMode::Get(endpoint, onMode) == EMBER_ZCL_STATUS_SUCCESS && !onMode.IsNull())
            {
                emberAfOnOffClusterPrintln("Changing Current Mode to %x", onMode.Value());
                status = ModeSelect::Attributes::CurrentMode::Set(endpoint, onMode.Value());
            }
        }
#endif
    }
#endif // IGNORE_ON_OFF_CLUSTER_START_UP_ON_OFF
    emberAfPluginOnOffClusterServerPostInitCallback(endpoint);
}

/** @brief Get the OnOff value when server starts.
 *
 * This function determines how StartUpOnOff affects the OnOff value when the server starts.
 *
 * @param endpoint   Ver.: always
 * @param onOffValueForStartUp Ver.: always
 */
EmberAfStatus OnOffServer::getOnOffValueForStartUp(chip::EndpointId endpoint, bool & onOffValueForStartUp)
{
    app::DataModel::Nullable<OnOff::OnOffStartUpOnOff> startUpOnOff;
    EmberAfStatus status = Attributes::StartUpOnOff::Get(endpoint, startUpOnOff);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // Initialise updated value to 0
        bool updatedOnOff = 0;
        status            = Attributes::OnOff::Get(endpoint, &updatedOnOff);
        if (status == EMBER_ZCL_STATUS_SUCCESS)
        {
            if (!startUpOnOff.IsNull())
            {
                switch (startUpOnOff.Value())
                {
                case OnOff::OnOffStartUpOnOff::kOff:
                    updatedOnOff = 0; // Off
                    break;
                case OnOff::OnOffStartUpOnOff::kOn:
                    updatedOnOff = 1; // On
                    break;
                case OnOff::OnOffStartUpOnOff::kTogglePreviousOnOff:
                    updatedOnOff = !updatedOnOff;
                    break;
                default:
                    // All other values 0x03- 0xFE are reserved - no action.
                    break;
                }
            }
            onOffValueForStartUp = updatedOnOff;
        }
    }
    return status;
}

bool OnOffServer::offCommand(const app::ConcreteCommandPath & commandPath)
{
    EmberAfStatus status = setOnOffValue(commandPath.mEndpointId, Commands::Off::Id, false);

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool OnOffServer::onCommand(const app::ConcreteCommandPath & commandPath)
{
    EmberAfStatus status = setOnOffValue(commandPath.mEndpointId, Commands::On::Id, false);

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool OnOffServer::toggleCommand(const app::ConcreteCommandPath & commandPath)
{
    EmberAfStatus status = setOnOffValue(commandPath.mEndpointId, Commands::Toggle::Id, false);

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool OnOffServer::offWithEffectCommand(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                       const Commands::OffWithEffect::DecodableType & commandData)
{
    OnOffEffectIdentifier effectId = commandData.effectId;
    uint8_t effectVariant          = commandData.effectVariant;
    chip::EndpointId endpoint      = commandPath.mEndpointId;
    EmberAfStatus status           = EMBER_ZCL_STATUS_SUCCESS;

    if (SupportsLightingApplications(endpoint))
    {
#ifdef EMBER_AF_PLUGIN_SCENES
        FabricIndex fabric = commandObj->GetAccessingFabricIndex();
#endif // EMBER_AF_PLUGIN_SCENES
        bool globalSceneControl = false;
        OnOff::Attributes::GlobalSceneControl::Get(endpoint, &globalSceneControl);

        bool isOnBeforeCommand = false;
        OnOff::Attributes::OnOff::Get(endpoint, &isOnBeforeCommand);

        if (globalSceneControl)
        {
#ifdef EMBER_AF_PLUGIN_SCENES
            GroupId groupId = ZCL_SCENES_GLOBAL_SCENE_GROUP_ID;
            if (commandObj->GetExchangeContext()->IsGroupExchangeContext())
            {
                groupId = commandObj->GetExchangeContext()->GetSessionHandle()->AsIncomingGroupSession()->GetGroupId();
            }

            emberAfScenesClusterStoreCurrentSceneCallback(fabric, endpoint, groupId, ZCL_SCENES_GLOBAL_SCENE_SCENE_ID);
#endif // EMBER_AF_PLUGIN_SCENES

            OnOff::Attributes::GlobalSceneControl::Set(endpoint, false);
            Attributes::OnTime::Set(endpoint, 0);
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

        status = setOnOffValue(endpoint, Commands::Off::Id, false);
    }
    else
    {
        status = EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND;
    }

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool OnOffServer::OnWithRecallGlobalSceneCommand(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath)
{
    chip::EndpointId endpoint = commandPath.mEndpointId;
    EmberAfStatus status      = EMBER_ZCL_STATUS_SUCCESS;

    if (!SupportsLightingApplications(endpoint))
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND);
        return true;
    }

#ifdef EMBER_AF_PLUGIN_SCENES
    FabricIndex fabric = commandObj->GetAccessingFabricIndex();
#endif // EMBER_AF_PLUGIN_SCENES

    bool globalSceneControl = false;
    OnOff::Attributes::GlobalSceneControl::Get(endpoint, &globalSceneControl);

    if (globalSceneControl)
    {
        emberAfSendImmediateDefaultResponse(status);
        return true;
    }

#ifdef EMBER_AF_PLUGIN_SCENES
    GroupId groupId = ZCL_SCENES_GLOBAL_SCENE_GROUP_ID;
    if (commandObj->GetExchangeContext()->IsGroupExchangeContext())
    {
        groupId = commandObj->GetExchangeContext()->GetSessionHandle()->AsIncomingGroupSession()->GetGroupId();
    }

    emberAfScenesClusterRecallSavedSceneCallback(fabric, endpoint, groupId, ZCL_SCENES_GLOBAL_SCENE_SCENE_ID);
#endif // EMBER_AF_PLUGIN_SCENES

    OnOff::Attributes::GlobalSceneControl::Set(endpoint, true);
    setOnOffValue(endpoint, Commands::On::Id, false);

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

uint32_t OnOffServer::calculateNextWaitTimeMS(void)
{
    const chip::System::Clock::Timestamp currentTime = chip::System::SystemClock().GetMonotonicTimestamp();
    chip::System::Clock::Timestamp waitTime          = UPDATE_TIME_MS;
    chip::System::Clock::Timestamp latency;

    if (currentTime > nextDesiredOnWithTimedOffTimestamp)
    {
        latency = currentTime - nextDesiredOnWithTimedOffTimestamp;
        if (latency >= UPDATE_TIME_MS)
            waitTime = chip::System::Clock::Milliseconds32(1);
        else
            waitTime -= latency;
    }

    nextDesiredOnWithTimedOffTimestamp += UPDATE_TIME_MS;

    return (uint32_t) waitTime.count();
}

bool OnOffServer::OnWithTimedOffCommand(const app::ConcreteCommandPath & commandPath,
                                        const Commands::OnWithTimedOff::DecodableType & commandData)
{
    BitFlags<OnOffControl> onOffControl = commandData.onOffControl;
    uint16_t onTime                     = commandData.onTime;
    uint16_t offWaitTime                = commandData.offWaitTime;
    EmberAfStatus status                = EMBER_ZCL_STATUS_SUCCESS;
    chip::EndpointId endpoint           = commandPath.mEndpointId;
    bool isOn                           = false;
    uint16_t currentOffWaitTime         = MAX_TIME_VALUE;
    uint16_t currentOnTime              = 0;

    EmberEventControl * event = configureEventControl(endpoint);
    VerifyOrExit(event != nullptr, status = EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);
    VerifyOrExit(SupportsLightingApplications(endpoint), status = EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND);

    OnOff::Attributes::OnOff::Get(endpoint, &isOn);

    // OnOff is off and the commands is only accepted if on
    if (onOffControl.Has(OnOffControl::kAcceptOnlyWhenOn) && !isOn)
    {
        emberAfSendImmediateDefaultResponse(status);
        return true;
    }

    OnOff::Attributes::OffWaitTime::Get(endpoint, &currentOffWaitTime);
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
        nextDesiredOnWithTimedOffTimestamp = chip::System::SystemClock().GetMonotonicTimestamp() + UPDATE_TIME_MS;
        emberEventControlSetDelayMS(configureEventControl(endpoint), (uint32_t) UPDATE_TIME_MS.count());
    }

exit:
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
        emberEventControlSetDelayMS(configureEventControl(endpoint), calculateNextWaitTimeMS());

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
            emberEventControlSetDelayMS(configureEventControl(endpoint), calculateNextWaitTimeMS());
        }
        else
        {
            emberAfOnOffClusterPrintln("Timer  Callback - wait Off Time cycle finished");

            // Stop timer on the endpoint
            emberEventControlSetInactive(getEventControl(endpoint));
        }
    }
}

#ifndef IGNORE_ON_OFF_CLUSTER_START_UP_ON_OFF
bool OnOffServer::areStartUpOnOffServerAttributesNonVolatile(EndpointId endpoint)
{
    return !emberAfIsKnownVolatileAttribute(endpoint, OnOff::Id, Attributes::OnOff::Id) &&
        !emberAfIsKnownVolatileAttribute(endpoint, OnOff::Id, Attributes::StartUpOnOff::Id);
}
#endif // IGNORE_ON_OFF_CLUSTER_START_UP_ON_OFF

/**
 * @brief event control object for an endpoint
 *
 * @param[in] endpoint
 * @return EmberEventControl* configured event control
 */
EmberEventControl * OnOffServer::getEventControl(EndpointId endpoint)
{
    uint16_t index            = emberAfFindClusterServerEndpointIndex(endpoint, OnOff::Id);
    EmberEventControl * event = nullptr;

    if (index < ArraySize(eventControls))
    {
        event = &eventControls[index];
    }

    return event;
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
    VerifyOrReturnError(controller != nullptr, nullptr);

    controller->endpoint = endpoint;
    controller->callback = &onOffWaitTimeOffEventHandler;

    return controller;
}

/**********************************************************
 * OnOffEffect Implementation
 *********************************************************/

static OnOffEffect * inst(EndpointId endpoint)
{
    OnOffEffect * current = firstEffect;
    while (current != nullptr && current->mEndpoint != endpoint)
    {
        current = current->next();
    }

    return current;
}

static inline void reg(OnOffEffect * inst)
{
    inst->setNext(firstEffect);
    firstEffect = inst;
}

static inline void unreg(OnOffEffect * inst)
{
    if (firstEffect == inst)
    {
        firstEffect = firstEffect->next();
    }
    else
    {
        OnOffEffect * previous = firstEffect;
        OnOffEffect * current  = firstEffect->next();

        while (current != nullptr && current != inst)
        {
            previous = current;
            current  = current->next();
        }

        if (current != nullptr)
        {
            previous->setNext(current->next());
        }
    }
}

OnOffEffect::OnOffEffect(chip::EndpointId endpoint, OffWithEffectTriggerCommand offWithEffectTrigger,
                         OnOffEffectIdentifier effectIdentifier, uint8_t effectVariant) :
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
    return OnOffServer::Instance().offCommand(commandPath);
}

bool emberAfOnOffClusterOnCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                   const Commands::On::DecodableType & commandData)
{
    return OnOffServer::Instance().onCommand(commandPath);
}

bool emberAfOnOffClusterToggleCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                       const Commands::Toggle::DecodableType & commandData)
{
    return OnOffServer::Instance().toggleCommand(commandPath);
}

bool emberAfOnOffClusterOffWithEffectCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                              const Commands::OffWithEffect::DecodableType & commandData)
{
    return OnOffServer::Instance().offWithEffectCommand(commandObj, commandPath, commandData);
}

bool emberAfOnOffClusterOnWithRecallGlobalSceneCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::OnWithRecallGlobalScene::DecodableType & commandData)
{
    return OnOffServer::Instance().OnWithRecallGlobalSceneCommand(commandObj, commandPath);
}

bool emberAfOnOffClusterOnWithTimedOffCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::OnWithTimedOff::DecodableType & commandData)
{
    return OnOffServer::Instance().OnWithTimedOffCommand(commandPath, commandData);
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

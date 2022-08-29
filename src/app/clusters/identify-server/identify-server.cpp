/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include "identify-server.h"

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>

#include <app/util/af.h>
#include <app/util/common.h>
#include <array>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#if CHIP_DEVICE_LAYER_NONE
#error "identify requrires a device layer"
#endif

#ifndef emberAfIdentifyClusterPrintln
#define emberAfIdentifyClusterPrintln(...) ChipLogProgress(Zcl, __VA_ARGS__);
#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Identify;

static Identify * firstIdentify = nullptr;

static void onIdentifyClusterTick(chip::System::Layer * systemLayer, void * appState);

static Identify * inst(EndpointId endpoint)
{
    Identify * current = firstIdentify;
    while (current != nullptr && current->mEndpoint != endpoint)
    {
        current = current->next();
    }

    return current;
}

static inline void reg(Identify * inst)
{
    inst->setNext(firstIdentify);
    firstIdentify = inst;
}

static inline void unreg(Identify * inst)
{
    if (firstIdentify == inst)
    {
        firstIdentify = firstIdentify->next();
    }
    else
    {
        Identify * previous = firstIdentify;
        Identify * current  = firstIdentify->next();

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

void emberAfIdentifyClusterServerInitCallback(EndpointId endpoint)
{
    Identify * identify = inst(endpoint);
    if (identify != nullptr)
    {
        (void) Clusters::Identify::Attributes::IdentifyType::Set(endpoint, identify->mIdentifyType);
    }
}

static void onIdentifyClusterTick(chip::System::Layer * systemLayer, void * appState)
{
    uint16_t identifyTime = 0;
    Identify * identify   = reinterpret_cast<Identify *>(appState);

    if (nullptr != identify)
    {
        EndpointId endpoint = identify->mEndpoint;

        if (EMBER_ZCL_STATUS_SUCCESS == Clusters::Identify::Attributes::IdentifyTime::Get(endpoint, &identifyTime) &&
            0 != identifyTime)
        {
            identifyTime = static_cast<uint16_t>(identifyTime == 0 ? 0 : identifyTime - 1);
            // This tick writes the new attribute, which will trigger the Attribute
            // Changed callback.
            (void) Clusters::Identify::Attributes::IdentifyTime::Set(endpoint, identifyTime);
        }
    }
}

static inline void identify_activate(Identify * identify)
{
    if (nullptr != identify->mOnIdentifyStart && nullptr != identify->mOnIdentifyStop && false == identify->mActive)
    {
        identify->mActive = true;
        identify->mOnIdentifyStart(identify);
    }
}

static inline void identify_deactivate(Identify * identify)
{
    if (nullptr != identify->mOnIdentifyStop)
    {
        identify->mActive = false;
        identify->mOnIdentifyStop(identify);
    }
}

void MatterIdentifyClusterServerAttributeChangedCallback(const app::ConcreteAttributePath & attributePath)
{
    if (attributePath.mAttributeId == Clusters::Identify::Attributes::IdentifyTime::Id)
    {
        EndpointId endpoint = attributePath.mEndpointId;
        Identify * identify = inst(endpoint);
        uint16_t identifyTime;

        if (identify == nullptr)
        {
            return;
        }

        if (EMBER_ZCL_STATUS_SUCCESS == Clusters::Identify::Attributes::IdentifyTime::Get(endpoint, &identifyTime))
        {
            /* effect identifier changed during identify */
            if (identify->mTargetEffectIdentifier != identify->mCurrentEffectIdentifier)
            {
                identify->mCurrentEffectIdentifier = identify->mTargetEffectIdentifier;

                /* finish identify process */
                if (EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_FINISH_EFFECT == identify->mCurrentEffectIdentifier && identifyTime > 0)
                {
                    Clusters::Identify::Attributes::IdentifyTime::Set(endpoint, 1);
                }
                /* stop identify process */
                if (EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT == identify->mCurrentEffectIdentifier && identifyTime > 0)
                {
                    Clusters::Identify::Attributes::IdentifyTime::Set(endpoint, 0);
                    identify_deactivate(identify);
                }
                /* change from e.g. Breathe to Blink during identify */
                else
                {
                    /* cancel identify */
                    Clusters::Identify::Attributes::IdentifyTime::Set(endpoint, 0);
                    identify_deactivate(identify);

                    /* trigger effect identifier callback */
                    if (nullptr != identify->mOnEffectIdentifier)
                        identify->mOnEffectIdentifier(identify);
                }
            }
            else if (identifyTime > 0)
            {
                /* we only start if both callbacks are set */
                identify_activate(identify);

                (void) chip::DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onIdentifyClusterTick, identify);
                return;
            }
            else
            {
                identify_deactivate(identify);
            }
        }

        (void) chip::DeviceLayer::SystemLayer().CancelTimer(onIdentifyClusterTick, identify);
    }
}

bool emberAfIdentifyClusterIdentifyCallback(CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                            const Commands::Identify::DecodableType & commandData)
{
    auto & identifyTime = commandData.identifyTime;

    // cmd Identify
    return EMBER_SUCCESS ==
        emberAfSendImmediateDefaultResponse(
               Clusters::Identify::Attributes::IdentifyTime::Set(commandPath.mEndpointId, identifyTime));
}

bool emberAfIdentifyClusterTriggerEffectCallback(CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::TriggerEffect::DecodableType & commandData)
{
    auto & effectIdentifier = commandData.effectIdentifier;
    auto & effectVariant    = commandData.effectVariant;

    EndpointId endpoint = commandPath.mEndpointId;

    // cmd TriggerEffect
    Identify * identify                      = inst(endpoint);
    uint16_t identifyTime                    = 0;
    EmberAfIdentifyEffectIdentifier effectId = static_cast<EmberAfIdentifyEffectIdentifier>(effectIdentifier);

    emberAfIdentifyClusterPrintln("RX identify:trigger effect 0x%X variant 0x%X", effectId, effectVariant);

    if (identify == nullptr)
    {
        return false;
    }

    identify->mTargetEffectIdentifier = effectId;
    identify->mEffectVariant          = effectVariant;

    /* only call the callback if no identify is in progress */
    if (nullptr != identify->mOnEffectIdentifier &&
        EMBER_ZCL_STATUS_SUCCESS == Clusters::Identify::Attributes::IdentifyTime::Get(endpoint, &identifyTime) && 0 == identifyTime)
    {
        identify->mCurrentEffectIdentifier = identify->mTargetEffectIdentifier;
        identify->mOnEffectIdentifier(identify);
    }

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

Identify::Identify(chip::EndpointId endpoint, onIdentifyStartCb onIdentifyStart, onIdentifyStopCb onIdentifyStop,
                   EmberAfIdentifyIdentifyType identifyType, onEffectIdentifierCb onEffectIdentifier,
                   EmberAfIdentifyEffectIdentifier effectIdentifier, EmberAfIdentifyEffectVariant effectVariant) :
    mEndpoint(endpoint),
    mOnIdentifyStart(onIdentifyStart), mOnIdentifyStop(onIdentifyStop), mIdentifyType(identifyType),
    mOnEffectIdentifier(onEffectIdentifier), mCurrentEffectIdentifier(effectIdentifier), mTargetEffectIdentifier(effectIdentifier),
    mEffectVariant(static_cast<uint8_t>(effectVariant))
{
    reg(this);
};

Identify::~Identify()
{
    unreg(this);
}

void MatterIdentifyPluginServerInitCallback() {}

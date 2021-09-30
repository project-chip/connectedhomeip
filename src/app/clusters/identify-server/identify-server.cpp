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
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/CommandHandler.h>

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

static std::array<Identify *, EMBER_AF_IDENTIFY_CLUSTER_SERVER_ENDPOINT_COUNT> instances = { 0 };

static void onIdentifyClusterTick(chip::System::Layer * systemLayer, void * appState);

static Identify * inst(EndpointId endpoint)
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

static inline void reg(Identify * inst)
{
    for (size_t i = 0; i < instances.size(); i++)
    {
        if (nullptr == instances[i])
        {
            instances[i] = inst;
        }
    }
}

static inline void unreg(Identify * inst)
{
    for (size_t i = 0; i < instances.size(); i++)
    {
        if (inst == instances[i])
        {
            instances[i] = nullptr;
        }
    }
}

void emberAfIdentifyClusterServerInitCallback(EndpointId endpoint)
{
    (void) endpoint;
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

void emberAfIdentifyClusterServerAttributeChangedCallback(EndpointId endpoint, AttributeId attributeId)
{
    if (attributeId == Clusters::Identify::Attributes::IdentifyTime::Id)
    {
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
                    (void) chip::DeviceLayer::SystemLayer().StartTimer(MILLISECOND_TICKS_PER_SECOND, onIdentifyClusterTick,
                                                                       identify);
                    return;
                }
                /* stop identify process */
                else if (EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT == identify->mCurrentEffectIdentifier && identifyTime > 0)
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

                (void) chip::DeviceLayer::SystemLayer().StartTimer(MILLISECOND_TICKS_PER_SECOND, onIdentifyClusterTick, identify);
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

bool emberAfIdentifyClusterIdentifyCallback(EndpointId endpoint, CommandHandler * commandObj, uint16_t identifyTime)
{
    // cmd Identify
    return EMBER_SUCCESS ==
        emberAfSendImmediateDefaultResponse(Clusters::Identify::Attributes::IdentifyTime::Set(endpoint, identifyTime));
}

bool emberAfIdentifyClusterIdentifyQueryCallback(EndpointId endpoint, CommandHandler * commandObj)
{
    // cmd IdentifyQuery
    uint16_t identifyTime  = 0;
    EmberAfStatus status   = EMBER_ZCL_STATUS_SUCCESS;
    EmberStatus sendStatus = EMBER_SUCCESS;
    CHIP_ERROR err         = CHIP_NO_ERROR;

    status = Clusters::Identify::Attributes::IdentifyTime::Get(endpoint, &identifyTime);

    if (status != EMBER_ZCL_STATUS_SUCCESS || 0 == identifyTime)
    {
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfIdentifyClusterPrintln("Error reading identify time");
        }
        else
        {
            emberAfIdentifyClusterPrintln("identifyTime is at 0");
        }
        emberAfIdentifyClusterPrintln("Sending back default response");
        sendStatus = emberAfSendImmediateDefaultResponse(status);
        if (EMBER_SUCCESS != sendStatus)
        {
            emberAfIdentifyClusterPrintln("Identify: failed to send %s response: "
                                          "0x%x",
                                          "default", sendStatus);
        }
        return true;
    }

    emberAfIdentifyClusterPrintln("Identifying for %u more seconds", identifyTime);
    {
        app::CommandPathParams cmdParams = { endpoint, /* group id */ 0, Clusters::Identify::Id,
                                             ZCL_IDENTIFY_QUERY_RESPONSE_COMMAND_ID, (app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;

        VerifyOrExit(commandObj != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), identifyTime));
        SuccessOrExit(err = commandObj->FinishCommand());
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        emberAfIdentifyClusterPrintln("Failed to encode response command.");
    }
    return true;
}

bool emberAfIdentifyClusterTriggerEffectCallback(EndpointId endpoint, CommandHandler * commandObj, uint8_t effectIdentifier,
                                                 uint8_t effectVariant)
{
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
    mOnIdentifyStart(onIdentifyStart), mOnIdentifyStop(onIdentifyStop), mOnEffectIdentifier(onEffectIdentifier),
    mCurrentEffectIdentifier(effectIdentifier), mTargetEffectIdentifier(effectIdentifier),
    mEffectVariant(static_cast<uint8_t>(effectVariant))
{
    (void) Clusters::Identify::Attributes::IdentifyType::Set(endpoint, identifyType);
    reg(this);
};

Identify::~Identify()
{
    unreg(this);
}

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

#include "identify-server.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <clusters/Identify/Metadata.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <tracing/macros.h>

#if CHIP_DEVICE_LAYER_NONE
#error "identify requrires a device layer"
#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Identify;
using chip::Protocols::InteractionModel::Status;

static Identify * firstIdentify = nullptr;

static void onIdentifyClusterTick(chip::System::Layer * systemLayer, void * appState);

// The Q quality rules for the Identify cluster are:
// 1. When it changes from 0 to any other value and vice versa, or
// 2. When it is written by a client, or
// 3. When the value (identifytime) is set by an Identify command.
//
// Changes to identifytime by either a write or a command will result
// in the attribute being set and marked dirty. This covers rules 2 and 3.
// Rule 1 is handled by not marking the regular countdown changes as dirty
// in the onIdentifyClusterTick function and only marking the attribute dirty
// when the identifytime is set to 0.

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
        (void) Attributes::IdentifyType::Set(endpoint, identify->mIdentifyType);
    }
}

static void onIdentifyClusterTick(chip::System::Layer * systemLayer, void * appState)
{
    Identify * identify = reinterpret_cast<Identify *>(appState);

    if (nullptr != identify)
    {
        EndpointId endpoint = identify->mEndpoint;

        uint16_t identifyTime = 0;
        if (Status::Success == Attributes::IdentifyTime::Get(endpoint, &identifyTime) && 0 != identifyTime)
        {
            auto markDirty = MarkAttributeDirty::kNo;
            identifyTime   = static_cast<uint16_t>(identifyTime - 1);

            // If the new identify time is 0, we need to mark the attribute dirty so that
            // the attribute change is reported
            if (identifyTime == 0)
            {
                markDirty = MarkAttributeDirty::kYes;
            }

            // This tick writes the new attribute, which will trigger the Attribute
            // Changed callback.
            (void) Attributes::IdentifyTime::Set(endpoint, identifyTime, markDirty);
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
    if (attributePath.mAttributeId == Attributes::IdentifyTime::Id)
    {
        EndpointId endpoint = attributePath.mEndpointId;
        Identify * identify = inst(endpoint);
        uint16_t identifyTime;

        if (identify == nullptr)
        {
            return;
        }

        if (Status::Success == Attributes::IdentifyTime::Get(endpoint, &identifyTime))
        {
            /* effect identifier changed during identify */
            if (identify->mTargetEffectIdentifier != identify->mCurrentEffectIdentifier)
            {
                identify->mCurrentEffectIdentifier = identify->mTargetEffectIdentifier;

                /* finish identify process */
                if (EffectIdentifierEnum::kFinishEffect == identify->mCurrentEffectIdentifier && identifyTime > 0)
                {
                    Attributes::IdentifyTime::Set(endpoint, 1);
                }
                /* stop identify process */
                if (EffectIdentifierEnum::kStopEffect == identify->mCurrentEffectIdentifier && identifyTime > 0)
                {
                    Attributes::IdentifyTime::Set(endpoint, 0);
                    identify_deactivate(identify);
                }
                /* change from e.g. Breathe to Blink during identify */
                else
                {
                    /* cancel identify */
                    Attributes::IdentifyTime::Set(endpoint, 0);
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

bool emberAfIdentifyClusterIdentifyCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                            const Commands::Identify::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("IdentifyCommand", "Identify");
    auto & identifyTime = commandData.identifyTime;

    // cmd Identify
    commandObj->AddStatus(commandPath, Attributes::IdentifyTime::Set(commandPath.mEndpointId, identifyTime));
    return true;
}

bool emberAfIdentifyClusterTriggerEffectCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                 const Commands::TriggerEffect::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("TriggerEffect", "Identify");
    auto & effectIdentifier = commandData.effectIdentifier;
    auto & effectVariant    = commandData.effectVariant;

    EndpointId endpoint = commandPath.mEndpointId;

    // cmd TriggerEffect
    Identify * identify   = inst(endpoint);
    uint16_t identifyTime = 0;

    ChipLogProgress(Zcl, "RX identify:trigger effect identifier 0x%X variant 0x%X", to_underlying(effectIdentifier),
                    to_underlying(effectVariant));

    if (identify == nullptr)
    {
        return false;
    }

    identify->mTargetEffectIdentifier = effectIdentifier;
    identify->mEffectVariant          = effectVariant;

    /* only call the callback if no identify is in progress */
    if (nullptr != identify->mOnEffectIdentifier && Status::Success == Attributes::IdentifyTime::Get(endpoint, &identifyTime) &&
        0 == identifyTime)
    {
        identify->mCurrentEffectIdentifier = identify->mTargetEffectIdentifier;
        identify->mOnEffectIdentifier(identify);
    }

    commandObj->AddStatus(commandPath, Status::Success);
    return true;
}

Identify::Identify(EndpointId endpoint, onIdentifyStartCb onIdentifyStart, onIdentifyStopCb onIdentifyStop,
                   IdentifyTypeEnum identifyType, onEffectIdentifierCb onEffectIdentifier, EffectIdentifierEnum effectIdentifier,
                   EffectVariantEnum effectVariant) :
    mEndpoint(endpoint),
    mOnIdentifyStart(onIdentifyStart), mOnIdentifyStop(onIdentifyStop), mIdentifyType(identifyType),
    mOnEffectIdentifier(onEffectIdentifier), mCurrentEffectIdentifier(effectIdentifier), mTargetEffectIdentifier(effectIdentifier),
    mEffectVariant(effectVariant)
{
    reg(this);
};

Identify::~Identify()
{
    unreg(this);
}

namespace {
IdentifyAttrAccess gAttrAccess;
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace Identify {
CHIP_ERROR IdentifyAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::ClusterRevision::Id:
        return aEncoder.Encode(Identify::kRevision);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}
} // namespace Identify
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterIdentifyPluginServerInitCallback()
{
    app::AttributeAccessInterfaceRegistry::Instance().Register(&gAttrAccess);
}
void MatterIdentifyPluginServerShutdownCallback()
{
    app::AttributeAccessInterfaceRegistry::Instance().Unregister(&gAttrAccess);
}

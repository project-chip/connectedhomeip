/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "OnOffLightSwitch.h"

#include <app/clusters/bindings/BindingManager.h>
#include <app/clusters/bindings/binding-table.h>
#include <clusters/Identify/Commands.h>
#include <clusters/Identify/Ids.h>
#include <clusters/OnOff/Commands.h>
#include <clusters/OnOff/Ids.h>
#include <devices/Types.h>
#include <platform/PlatformManager.h>

using namespace chip::app::Clusters;

namespace chip::app {

namespace {
const ClusterId kClientClusters[] = { OnOff::Id, Identify::Id };
} // namespace

OnOffLightSwitch::OnOffLightSwitch(const Config & config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kOnOffLightSwitch, 1)), mConfig(config)
{}

CHIP_ERROR OnOffLightSwitch::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                      EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mConfig.timerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mBindingCluster.Create(
        BindingCluster::Context{
            .bindingTable    = mConfig.bindingTable,
            .bindingManager  = mConfig.bindingManager,
            .platformManager = mConfig.platformManager,
        },
        endpoint);
    ReturnErrorOnFailure(provider.AddCluster(mBindingCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void OnOffLightSwitch::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mBindingCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mBindingCluster.Cluster()));
        mBindingCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

CHIP_ERROR OnOffLightSwitch::ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const
{
    return out.ReferenceExisting(Span<const ClusterId>(kClientClusters));
}

template <typename CommandType>
CHIP_ERROR OnOffLightSwitch::SendCommand(const CommandType & command)
{
    VerifyOrReturnError(mEndpointId != kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    return mConfig.bindingHandler.DispatchSimple(mEndpointId, command);
}

CHIP_ERROR OnOffLightSwitch::SendOn()
{
    return SendCommand(OnOff::Commands::On::Type{});
}

CHIP_ERROR OnOffLightSwitch::SendOff()
{
    return SendCommand(OnOff::Commands::Off::Type{});
}

CHIP_ERROR OnOffLightSwitch::SendToggle()
{
    return SendCommand(OnOff::Commands::Toggle::Type{});
}

CHIP_ERROR OnOffLightSwitch::SendIdentify(uint16_t identifyTimeSeconds)
{
    return SendCommand(Identify::Commands::Identify::Type{ .identifyTime = identifyTimeSeconds });
}

CHIP_ERROR OnOffLightSwitch::SendTriggerEffect(Identify::EffectIdentifierEnum effectIdentifier,
                                               Identify::EffectVariantEnum effectVariant)
{
    return SendCommand(Identify::Commands::TriggerEffect::Type{
        .effectIdentifier = effectIdentifier,
        .effectVariant    = effectVariant,
    });
}

} // namespace chip::app

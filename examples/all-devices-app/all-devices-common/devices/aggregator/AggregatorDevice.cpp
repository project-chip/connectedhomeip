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

#include <devices/Types.h>
#include <devices/aggregator/AggregatorDevice.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/StringBuilder.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {

class AggregatorIdentifyDelegate : public IdentifyDelegate
{
public:
    void OnIdentifyStart(IdentifyCluster & cluster) override
    {
        ChipLogProgress(DeviceLayer, "Aggregator [Endpoint %d]: Identify START", cluster.GetPaths()[0].mEndpointId);
    }

    void OnIdentifyStop(IdentifyCluster & cluster) override
    {
        ChipLogProgress(DeviceLayer, "Aggregator [Endpoint %d]: Identify STOP", cluster.GetPaths()[0].mEndpointId);
    }

    void OnTriggerEffect(IdentifyCluster & cluster) override
    {
        StringBuilder<48> msg;

        switch (cluster.GetEffectIdentifier())
        {
        case Identify::EffectIdentifierEnum::kBlink:
            msg.Add("BlinkEffect");
            break;
        case Identify::EffectIdentifierEnum::kBreathe:
            msg.Add("BreatheEffect");
            break;
        case Identify::EffectIdentifierEnum::kOkay:
            msg.Add("OkayEffect");
            break;
        case Identify::EffectIdentifierEnum::kChannelChange:
            msg.Add("ChannelChangeEffect");
            break;
        case Identify::EffectIdentifierEnum::kFinishEffect:
            msg.Add("FinishEffect");
            break;
        case Identify::EffectIdentifierEnum::kStopEffect:
            msg.Add("StopEffect");
            break;
        default:
            msg.AddFormat("UnknownEffect(%d)", static_cast<int>(cluster.GetEffectIdentifier()));
            break;
        }
        msg.Add(" / ");

        switch (cluster.GetEffectVariant())
        {
        case Identify::EffectVariantEnum::kDefault:
            msg.Add("DefaultVariant");
            break;
        default:
            msg.AddFormat("UnknownVariant(%d)", static_cast<int>(cluster.GetEffectVariant()));
            break;
        }

        ChipLogProgress(DeviceLayer, "Aggregator [Endpoint %d]: Trigger effect: %s", cluster.GetPaths()[0].mEndpointId,
                        msg.c_str());
    }

    bool IsTriggerEffectEnabled() const override { return true; }
};

AggregatorIdentifyDelegate & GetIdentifyDelegate()
{
    static AggregatorIdentifyDelegate gIdentifyDelegate;
    return gIdentifyDelegate;
}

} // namespace

AggregatorDevice::AggregatorDevice(TimerDelegate & timerDelegate) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kAggregator, 1)), mTimerDelegate(timerDelegate)
{}

CHIP_ERROR AggregatorDevice::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    ReturnErrorOnFailure(SingleEndpointRegistration(endpoint, provider, parentId));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate).WithDelegate(&GetIdentifyDelegate()));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void AggregatorDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    SingleEndpointUnregistration(provider);
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

} // namespace app
} // namespace chip

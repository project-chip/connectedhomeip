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

#pragma once
#include "app/clusters/flow-measurement-server/FlowMeasurementCluster.h"
#include "app/clusters/identify-server/IdentifyCluster.h"
#include <app/clusters/operational-state-server/OperationalStateCluster.h>
#include "app/server-cluster/ServerClusterInterfaceRegistry.h"
#include <app/clusters/valve-configuration-and-control-server/ValveConfigurationAndControlCluster.h>
#include "lib/core/CHIPError.h"
#include "lib/support/TimerDelegate.h"
#include <lib/core/DataModelTypes.h>
#include <device/api/Interface.h>
#include <clusters/shared/Enums.h>
#include <devices/Types.h>

namespace chip {
namespace app {

class Irrigation :  public DeviceInterface
{
public:
    struct ValveList
    { 
        DataModel::Nullable<Clusters::ValveConfigurationAndControlCluster::StartupConfiguration>  startupConfiguration;
        DataModel::Nullable<Clusters::ValveConfigurationAndControlCluster::ValveContext> valveContext;
        Span<const EndpointComposition::SemanticTag> tags;
    };

    Irrigation(TimerDelegate & TDelegate,Clusters::IdentifyDelegate & IDelegate,
                Clusters::OperationalState::OperationalStateCluster::Delegate * ODelegate) :  DeviceInterface(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kIrrigationSystem, 1)),
                                                                                    
                mTimerDelegate(TDelegate),mIdentifyDelegate(&IDelegate),
                                                                                    mOperationalStateDelegate(ODelegate)
                                                                                    
                                                                                    {}
    ~Irrigation() override = default;

    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                EndpointComposition composition) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::IdentifyCluster & IdentifyCluster()
    {
        VerifyOrDie(mIdentifyCluster.IsConstructed());
        return mIdentifyCluster.Cluster();
    }
    Clusters::OperationalState::OperationalStateCluster & OperationalStateCluster()
    {
        VerifyOrDie(mOperationalStateCluster.IsConstructed());
        return mOperationalStateCluster.Cluster();
    }
    Clusters::FlowMeasurementCluster & FlowMeasurementCluster()
    {
        VerifyOrDie(mFlowMeasurementCluster.IsConstructed());
        return mFlowMeasurementCluster.Cluster();
    }

    EndpointId GetEndpointId() const
    {
        return mEndpointId;
    }
protected:
    TimerDelegate & mTimerDelegate;
private:
    virtual CHIP_ERROR RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                     EndpointComposition composition)    = 0;
    virtual void UnregisterParts(CodeDrivenDataModelProvider & provider) = 0;
    EndpointId mEndpointId = kInvalidEndpointId;
    Clusters::IdentifyDelegate * mIdentifyDelegate;
    Clusters::OperationalState::OperationalStateCluster::Delegate * mOperationalStateDelegate;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::OperationalState::OperationalStateCluster> mOperationalStateCluster;
    LazyRegisteredServerCluster<Clusters::FlowMeasurementCluster> mFlowMeasurementCluster;
};

} // namespace app
} // namespace chip

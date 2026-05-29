/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
 * @file
 *   Application-facing server API for the Smoke CO Alarm cluster.
 *
 */

#pragma once

#include <app/clusters/smoke-co-alarm-server/smokeCOAlarmCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

namespace chip {
namespace app {
namespace Clusters {
namespace SmokeCoAlarm {

/**
 * @brief Application-facing wrapper for SmokeCoAlarmCluster.
 *
 * Owns the cluster instance and manages its lifetime with the data model provider.
 * The application creates one SmokeCoAlarmServer per endpoint, calls Init() to
 * register with the provider, and destroys the object to unregister.
 */
class SmokeCoAlarmServer
{
public:
    explicit SmokeCoAlarmServer(EndpointId endpointId, const SmokeCoAlarmCluster::Config & config) :
        mEndpointId(endpointId), mConfig(config)
    {}

    ~SmokeCoAlarmServer()
    {
        if (mCluster.IsConstructed())
        {
            LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster()));
        }
        // ~LazyRegisteredServerCluster calls Destroy() automatically if still constructed
    }

    CHIP_ERROR Init()
    {
        VerifyOrReturnValue(!mCluster.IsConstructed(), CHIP_NO_ERROR);
        mCluster.Create(mEndpointId, mConfig);
        CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
        if (err != CHIP_NO_ERROR)
        {
            mCluster.Destroy();
            return err;
        }
        return CHIP_NO_ERROR;
    }

    SmokeCoAlarmCluster & Cluster()
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster();
    }

    void
    SetExpressedStateByPriority(const std::array<ExpressedStateEnum, SmokeCoAlarmCluster::kPriorityOrderLength> & priorityOrder)
    {
        VerifyOrDie(mCluster.IsConstructed());
        mCluster.Cluster().SetExpressedStateByPriority(priorityOrder);
    }

    bool RequestSelfTest()
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().RequestSelfTest();
    }

    bool SetSmokeState(AlarmStateEnum v)
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().SetSmokeState(v);
    }
    bool SetCOState(AlarmStateEnum v)
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().SetCOState(v);
    }
    bool SetBatteryAlert(AlarmStateEnum v)
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().SetBatteryAlert(v);
    }
    bool SetDeviceMuted(MuteStateEnum v)
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().SetDeviceMuted(v);
    }
    bool SetTestInProgress(bool v)
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().SetTestInProgress(v);
    }
    bool SetHardwareFaultAlert(bool v)
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().SetHardwareFaultAlert(v);
    }
    bool SetEndOfServiceAlert(EndOfServiceEnum v)
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().SetEndOfServiceAlert(v);
    }
    bool SetInterconnectSmokeAlarm(AlarmStateEnum v)
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().SetInterconnectSmokeAlarm(v);
    }
    bool SetInterconnectCOAlarm(AlarmStateEnum v)
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().SetInterconnectCOAlarm(v);
    }
    bool SetContaminationState(ContaminationStateEnum v)
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().SetContaminationState(v);
    }
    bool SetSmokeSensitivityLevel(SensitivityEnum v)
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().SetSmokeSensitivityLevel(v);
    }
    bool SetUnmountedState(bool v)
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().SetUnmountedState(v);
    }

    bool GetExpressedState(ExpressedStateEnum & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetExpressedState(v);
    }
    bool GetSmokeState(AlarmStateEnum & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetSmokeState(v);
    }
    bool GetCOState(AlarmStateEnum & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetCOState(v);
    }
    bool GetBatteryAlert(AlarmStateEnum & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetBatteryAlert(v);
    }
    bool GetDeviceMuted(MuteStateEnum & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetDeviceMuted(v);
    }
    bool GetTestInProgress(bool & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetTestInProgress(v);
    }
    bool GetHardwareFaultAlert(bool & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetHardwareFaultAlert(v);
    }
    bool GetEndOfServiceAlert(EndOfServiceEnum & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetEndOfServiceAlert(v);
    }
    bool GetInterconnectSmokeAlarm(AlarmStateEnum & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetInterconnectSmokeAlarm(v);
    }
    bool GetInterconnectCOAlarm(AlarmStateEnum & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetInterconnectCOAlarm(v);
    }
    bool GetContaminationState(ContaminationStateEnum & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetContaminationState(v);
    }
    bool GetSmokeSensitivityLevel(SensitivityEnum & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetSmokeSensitivityLevel(v);
    }
    bool GetExpiryDate(uint32_t & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetExpiryDate(v);
    }
    bool GetUnmountedState(bool & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetUnmountedState(v);
    }

    chip::BitFlags<Feature> GetFeatures() const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetFeatures();
    }
    bool SupportsSmokeAlarm() const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().SupportsSmokeAlarm();
    }
    bool SupportsCOAlarm() const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().SupportsCOAlarm();
    }

private:
    EndpointId mEndpointId;
    SmokeCoAlarmCluster::Config mConfig;
    LazyRegisteredServerCluster<SmokeCoAlarmCluster> mCluster;
};

} // namespace SmokeCoAlarm
} // namespace Clusters
} // namespace app
} // namespace chip

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
            if (sInstance == this)
                sInstance = nullptr;
            LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster()));
        }
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
        sInstance = this;
        return CHIP_NO_ERROR;
    }

    SmokeCoAlarmCluster & Cluster()
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster();
    }

    static constexpr size_t kPriorityOrderLength = SmokeCoAlarmCluster::kPriorityOrderLength;

    static SmokeCoAlarmServer & Instance()
    {
        VerifyOrDie(sInstance != nullptr);
        return *sInstance;
    }

    bool RequestSelfTest()
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().RequestSelfTest();
    }

    // Endpoint-taking overloads for backwards compatibility with old call sites.
    // The endpoint param is ignored; Instance() already owns a specific endpoint.
    void SetExpressedStateByPriority(EndpointId, const std::array<ExpressedStateEnum, kPriorityOrderLength> & o)
    {
        Cluster().SetExpressedStateByPriority(o);
    }
    bool SetSmokeState(EndpointId, AlarmStateEnum v) { return Cluster().SetSmokeState(v); }
    bool SetCOState(EndpointId, AlarmStateEnum v) { return Cluster().SetCOState(v); }
    bool SetBatteryAlert(EndpointId, AlarmStateEnum v) { return Cluster().SetBatteryAlert(v); }
    bool SetDeviceMuted(EndpointId, MuteStateEnum v) { return Cluster().SetDeviceMuted(v); }
    bool SetTestInProgress(EndpointId, bool v) { return Cluster().SetTestInProgress(v); }
    bool SetHardwareFaultAlert(EndpointId, bool v) { return Cluster().SetHardwareFaultAlert(v); }
    bool SetEndOfServiceAlert(EndpointId, EndOfServiceEnum v) { return Cluster().SetEndOfServiceAlert(v); }
    bool SetInterconnectSmokeAlarm(EndpointId, AlarmStateEnum v) { return Cluster().SetInterconnectSmokeAlarm(v); }
    bool SetInterconnectCOAlarm(EndpointId, AlarmStateEnum v) { return Cluster().SetInterconnectCOAlarm(v); }
    bool SetContaminationState(EndpointId, ContaminationStateEnum v) { return Cluster().SetContaminationState(v); }
    bool SetSmokeSensitivityLevel(EndpointId, SensitivityEnum v) { return Cluster().SetSmokeSensitivityLevel(v); }
    bool SetUnmountedState(EndpointId, bool v) { return Cluster().SetUnmountedState(v); }

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

    inline static SmokeCoAlarmServer * sInstance = nullptr;
};

} // namespace SmokeCoAlarm
} // namespace Clusters
} // namespace app
} // namespace chip

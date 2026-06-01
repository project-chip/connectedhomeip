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

/**
 * @brief Application-facing wrapper for SmokeCoAlarm::SmokeCoAlarmCluster.
 *
 * Owns the cluster instance and manages its lifetime with the data model provider.
 * The application creates one SmokeCoAlarmServer per endpoint, calls Init() to
 * register with the provider, and destroys the object to unregister.
 */
class SmokeCoAlarmServer
{
public:
    explicit SmokeCoAlarmServer(EndpointId endpointId, const SmokeCoAlarm::SmokeCoAlarmCluster::Config & config) :
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
    void SetInoperativeWhenUnmounted(bool v) { Cluster().SetInoperativeWhenUnmounted(v); }

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

    SmokeCoAlarm::SmokeCoAlarmCluster & Cluster()
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster();
    }

    static constexpr size_t kPriorityOrderLength = SmokeCoAlarm::SmokeCoAlarmCluster::kPriorityOrderLength;

    static SmokeCoAlarmServer & Instance()
    {
        VerifyOrDie(sInstance != nullptr);
        return *sInstance;
    }

    bool RequestSelfTest(EndpointId endpoint)
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().RequestSelfTest();
    }

    // Endpoint-taking overloads for backwards compatibility with old call sites.
    // The endpoint param is ignored; Instance() already owns a specific endpoint.
    void SetExpressedStateByPriority(EndpointId, const std::array<SmokeCoAlarm::ExpressedStateEnum, kPriorityOrderLength> & o)
    {
        Cluster().SetExpressedStateByPriority(o);
    }
    bool SetSmokeState(EndpointId, SmokeCoAlarm::AlarmStateEnum v) { return Cluster().SetSmokeState(v); }
    bool SetCOState(EndpointId, SmokeCoAlarm::AlarmStateEnum v) { return Cluster().SetCOState(v); }
    bool SetBatteryAlert(EndpointId, SmokeCoAlarm::AlarmStateEnum v) { return Cluster().SetBatteryAlert(v); }
    bool SetDeviceMuted(EndpointId, SmokeCoAlarm::MuteStateEnum v) { return Cluster().SetDeviceMuted(v); }
    bool SetTestInProgress(EndpointId, bool v) { return Cluster().SetTestInProgress(v); }
    bool SetHardwareFaultAlert(EndpointId, bool v) { return Cluster().SetHardwareFaultAlert(v); }
    bool SetEndOfServiceAlert(EndpointId, SmokeCoAlarm::EndOfServiceEnum v) { return Cluster().SetEndOfServiceAlert(v); }
    bool SetInterconnectSmokeAlarm(EndpointId, SmokeCoAlarm::AlarmStateEnum v) { return Cluster().SetInterconnectSmokeAlarm(v); }
    bool SetInterconnectCOAlarm(EndpointId, SmokeCoAlarm::AlarmStateEnum v) { return Cluster().SetInterconnectCOAlarm(v); }
    bool SetContaminationState(EndpointId, SmokeCoAlarm::ContaminationStateEnum v) { return Cluster().SetContaminationState(v); }
    bool SetSmokeSensitivityLevel(EndpointId, SmokeCoAlarm::SensitivityEnum v) { return Cluster().SetSmokeSensitivityLevel(v); }
    bool SetUnmountedState(EndpointId, bool v) { return Cluster().SetUnmountedState(v); }

    chip::BitFlags<SmokeCoAlarm::Feature> GetFeatures() const
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

    // Endpoint-taking Get overloads for backwards compatibility with old call sites.
    // The endpoint param is ignored; Instance() already owns a specific endpoint.
    bool GetExpressedState(EndpointId, SmokeCoAlarm::ExpressedStateEnum & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetExpressedState(v);
    }
    bool GetSmokeState(EndpointId, SmokeCoAlarm::AlarmStateEnum & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetSmokeState(v);
    }
    bool GetCOState(EndpointId, SmokeCoAlarm::AlarmStateEnum & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetCOState(v);
    }
    bool GetBatteryAlert(EndpointId, SmokeCoAlarm::AlarmStateEnum & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetBatteryAlert(v);
    }
    bool GetDeviceMuted(EndpointId, SmokeCoAlarm::MuteStateEnum & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetDeviceMuted(v);
    }
    bool GetTestInProgress(EndpointId, bool & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetTestInProgress(v);
    }
    bool GetHardwareFaultAlert(EndpointId, bool & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetHardwareFaultAlert(v);
    }
    bool GetEndOfServiceAlert(EndpointId, SmokeCoAlarm::EndOfServiceEnum & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetEndOfServiceAlert(v);
    }
    bool GetInterconnectSmokeAlarm(EndpointId, SmokeCoAlarm::AlarmStateEnum & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetInterconnectSmokeAlarm(v);
    }
    bool GetInterconnectCOAlarm(EndpointId, SmokeCoAlarm::AlarmStateEnum & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetInterconnectCOAlarm(v);
    }
    bool GetContaminationState(EndpointId, SmokeCoAlarm::ContaminationStateEnum & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetContaminationState(v);
    }
    bool GetSmokeSensitivityLevel(EndpointId, SmokeCoAlarm::SensitivityEnum & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetSmokeSensitivityLevel(v);
    }
    bool GetExpiryDate(EndpointId, uint32_t & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetExpiryDate(v);
    }
    bool GetUnmountedState(EndpointId, bool & v) const
    {
        VerifyOrDie(mCluster.IsConstructed());
        return mCluster.Cluster().GetUnmountedState(v);
    }
    chip::BitFlags<SmokeCoAlarm::Feature> GetFeatures(EndpointId) const { return GetFeatures(); }
    bool SupportsSmokeAlarm(EndpointId) const { return SupportsSmokeAlarm(); }
    bool SupportsCOAlarm(EndpointId) const { return SupportsCOAlarm(); }

private:
    EndpointId mEndpointId;
    SmokeCoAlarm::SmokeCoAlarmCluster::Config mConfig;
    LazyRegisteredServerCluster<SmokeCoAlarm::SmokeCoAlarmCluster> mCluster;

    inline static SmokeCoAlarmServer * sInstance = nullptr;
};

} // namespace Clusters
} // namespace app
} // namespace chip

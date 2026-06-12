/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include "CodegenIntegration.h"
#include <lib/support/CodeUtils.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SmokeCoAlarm;

SmokeCoAlarmServer::~SmokeCoAlarmServer()
{
    if (mCluster.IsConstructed())
    {
        LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster()));
        mCluster.Destroy();
    }
}

void SmokeCoAlarmServer::SetInoperativeWhenUnmounted(bool v)
{
    Cluster().SetInoperativeWhenUnmounted(v);
}

CHIP_ERROR SmokeCoAlarmServer::Init(EndpointId endpointId, const SmokeCoAlarmCluster::Config & config,
                                    SmokeCoAlarmDelegate * delegate)
{
    VerifyOrReturnValue(!mCluster.IsConstructed(), CHIP_NO_ERROR);
    mEndpointId = endpointId;
    mConfig     = config;
    mCluster.Create(mEndpointId, mConfig);
    mCluster.Cluster().SetDelegate(delegate);
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
    if (err != CHIP_NO_ERROR)
    {
        mCluster.Destroy();
        return err;
    }
    return CHIP_NO_ERROR;
}

SmokeCoAlarmCluster & SmokeCoAlarmServer::Cluster()
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster();
}

bool SmokeCoAlarmServer::RequestSelfTest(EndpointId endpoint)
{
    VerifyOrDie(mCluster.IsConstructed());
    if (endpoint != mEndpointId)
    {
        ChipLogError(Zcl, "RequestSelfTest called with endpoint %d but cluster is on endpoint %d", endpoint, mEndpointId);
        return false;
    }
    return mCluster.Cluster().RequestSelfTest();
}

void SmokeCoAlarmServer::SetExpressedStateByPriority(EndpointId endpoint,
                                                     const std::array<ExpressedStateEnum, kPriorityOrderLength> & o)
{
    VerifyOrReturn(endpoint == mEndpointId,
                   ChipLogError(Zcl, "%s: endpoint %d does not match cluster endpoint %d", __func__, endpoint, mEndpointId));
    Cluster().SetExpressedStateByPriority(o);
}

bool SmokeCoAlarmServer::SetSmokeState(EndpointId endpoint, AlarmStateEnum v)
{
    VerifyOrReturnValue(endpoint == mEndpointId, false,
                        ChipLogError(Zcl, "%s: endpoint %d does not match cluster endpoint %d", __func__, endpoint, mEndpointId));
    return Cluster().SetSmokeState(v);
}
bool SmokeCoAlarmServer::SetCOState(EndpointId endpoint, AlarmStateEnum v)
{
    VerifyOrReturnValue(endpoint == mEndpointId, false,
                        ChipLogError(Zcl, "%s: endpoint %d does not match cluster endpoint %d", __func__, endpoint, mEndpointId));
    return Cluster().SetCOState(v);
}
void SmokeCoAlarmServer::SetBatteryAlert(EndpointId endpoint, AlarmStateEnum v)
{
    VerifyOrReturn(endpoint == mEndpointId,
                   ChipLogError(Zcl, "%s: endpoint %d does not match cluster endpoint %d", __func__, endpoint, mEndpointId));
    Cluster().SetBatteryAlert(v);
}
bool SmokeCoAlarmServer::SetDeviceMuted(EndpointId endpoint, MuteStateEnum v)
{
    VerifyOrReturnValue(endpoint == mEndpointId, false,
                        ChipLogError(Zcl, "%s: endpoint %d does not match cluster endpoint %d", __func__, endpoint, mEndpointId));
    return Cluster().SetDeviceMuted(v);
}
void SmokeCoAlarmServer::SetTestInProgress(EndpointId endpoint, bool v)
{
    VerifyOrReturn(endpoint == mEndpointId,
                   ChipLogError(Zcl, "%s: endpoint %d does not match cluster endpoint %d", __func__, endpoint, mEndpointId));
    Cluster().SetTestInProgress(v);
}
void SmokeCoAlarmServer::SetHardwareFaultAlert(EndpointId endpoint, bool v)
{
    VerifyOrReturn(endpoint == mEndpointId,
                   ChipLogError(Zcl, "%s: endpoint %d does not match cluster endpoint %d", __func__, endpoint, mEndpointId));
    Cluster().SetHardwareFaultAlert(v);
}
void SmokeCoAlarmServer::SetEndOfServiceAlert(EndpointId endpoint, EndOfServiceEnum v)
{
    VerifyOrReturn(endpoint == mEndpointId,
                   ChipLogError(Zcl, "%s: endpoint %d does not match cluster endpoint %d", __func__, endpoint, mEndpointId));
    Cluster().SetEndOfServiceAlert(v);
}
bool SmokeCoAlarmServer::SetInterconnectSmokeAlarm(EndpointId endpoint, AlarmStateEnum v)
{
    VerifyOrReturnValue(endpoint == mEndpointId, false,
                        ChipLogError(Zcl, "%s: endpoint %d does not match cluster endpoint %d", __func__, endpoint, mEndpointId));
    return Cluster().SetInterconnectSmokeAlarm(v);
}
bool SmokeCoAlarmServer::SetInterconnectCOAlarm(EndpointId endpoint, AlarmStateEnum v)
{
    VerifyOrReturnValue(endpoint == mEndpointId, false,
                        ChipLogError(Zcl, "%s: endpoint %d does not match cluster endpoint %d", __func__, endpoint, mEndpointId));
    return Cluster().SetInterconnectCOAlarm(v);
}
void SmokeCoAlarmServer::SetContaminationState(EndpointId endpoint, ContaminationStateEnum v)
{
    VerifyOrReturn(endpoint == mEndpointId,
                   ChipLogError(Zcl, "%s: endpoint %d does not match cluster endpoint %d", __func__, endpoint, mEndpointId));
    Cluster().SetContaminationState(v);
}
void SmokeCoAlarmServer::SetSmokeSensitivityLevel(EndpointId endpoint, SensitivityEnum v)
{
    VerifyOrReturn(endpoint == mEndpointId,
                   ChipLogError(Zcl, "%s: endpoint %d does not match cluster endpoint %d", __func__, endpoint, mEndpointId));
    Cluster().SetSmokeSensitivityLevel(v);
}
void SmokeCoAlarmServer::SetExpiryDate(EndpointId endpoint, uint32_t v)
{
    VerifyOrReturn(endpoint == mEndpointId,
                   ChipLogError(Zcl, "%s: endpoint %d does not match cluster endpoint %d", __func__, endpoint, mEndpointId));
    Cluster().SetExpiryDate(v);
}
bool SmokeCoAlarmServer::SetUnmountedState(EndpointId endpoint, bool v)
{
    VerifyOrReturnValue(endpoint == mEndpointId, false,
                        ChipLogError(Zcl, "%s: endpoint %d does not match cluster endpoint %d", __func__, endpoint, mEndpointId));
    return Cluster().SetUnmountedState(v);
}

chip::BitFlags<Feature> SmokeCoAlarmServer::GetFeatures(EndpointId endpoint) const
{
    VerifyOrDie(endpoint == mEndpointId);
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetFeatures();
}

bool SmokeCoAlarmServer::SupportsSmokeAlarm(EndpointId endpoint) const
{
    VerifyOrDie(endpoint == mEndpointId);
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().SupportsSmokeAlarm();
}
bool SmokeCoAlarmServer::SupportsCOAlarm(EndpointId endpoint) const
{
    VerifyOrDie(endpoint == mEndpointId);
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().SupportsCOAlarm();
}

bool SmokeCoAlarmServer::GetExpressedState(EndpointId endpoint, ExpressedStateEnum & v) const
{
    VerifyOrReturnValue(endpoint == mEndpointId, false);
    VerifyOrDie(mCluster.IsConstructed());
    v = mCluster.Cluster().GetExpressedState();
    return true;
}

bool SmokeCoAlarmServer::GetSmokeState(EndpointId endpoint, AlarmStateEnum & v) const
{
    VerifyOrReturnValue(endpoint == mEndpointId, false);
    VerifyOrDie(mCluster.IsConstructed());
    v = mCluster.Cluster().GetSmokeState();
    return true;
}

bool SmokeCoAlarmServer::GetCOState(EndpointId endpoint, AlarmStateEnum & v) const
{
    VerifyOrReturnValue(endpoint == mEndpointId, false);
    VerifyOrDie(mCluster.IsConstructed());
    v = mCluster.Cluster().GetCOState();
    return true;
}

bool SmokeCoAlarmServer::GetBatteryAlert(EndpointId endpoint, AlarmStateEnum & v) const
{
    VerifyOrReturnValue(endpoint == mEndpointId, false);
    VerifyOrDie(mCluster.IsConstructed());
    v = mCluster.Cluster().GetBatteryAlert();
    return true;
}

bool SmokeCoAlarmServer::GetDeviceMuted(EndpointId endpoint, MuteStateEnum & v) const
{
    VerifyOrReturnValue(endpoint == mEndpointId, false);
    VerifyOrDie(mCluster.IsConstructed());
    v = mCluster.Cluster().GetDeviceMuted();
    return true;
}

bool SmokeCoAlarmServer::GetTestInProgress(EndpointId endpoint, bool & v) const
{
    VerifyOrReturnValue(endpoint == mEndpointId, false);
    VerifyOrDie(mCluster.IsConstructed());
    v = mCluster.Cluster().GetTestInProgress();
    return true;
}

bool SmokeCoAlarmServer::GetHardwareFaultAlert(EndpointId endpoint, bool & v) const
{
    VerifyOrReturnValue(endpoint == mEndpointId, false);
    VerifyOrDie(mCluster.IsConstructed());
    v = mCluster.Cluster().GetHardwareFaultAlert();
    return true;
}

bool SmokeCoAlarmServer::GetEndOfServiceAlert(EndpointId endpoint, EndOfServiceEnum & v) const
{
    VerifyOrReturnValue(endpoint == mEndpointId, false);
    VerifyOrDie(mCluster.IsConstructed());
    v = mCluster.Cluster().GetEndOfServiceAlert();
    return true;
}

bool SmokeCoAlarmServer::GetInterconnectSmokeAlarm(EndpointId endpoint, AlarmStateEnum & v) const
{
    VerifyOrReturnValue(endpoint == mEndpointId, false);
    VerifyOrDie(mCluster.IsConstructed());
    v = mCluster.Cluster().GetInterconnectSmokeAlarm();
    return true;
}

bool SmokeCoAlarmServer::GetInterconnectCOAlarm(EndpointId endpoint, AlarmStateEnum & v) const
{
    VerifyOrReturnValue(endpoint == mEndpointId, false);
    VerifyOrDie(mCluster.IsConstructed());
    v = mCluster.Cluster().GetInterconnectCOAlarm();
    return true;
}

bool SmokeCoAlarmServer::GetContaminationState(EndpointId endpoint, ContaminationStateEnum & v) const
{
    VerifyOrReturnValue(endpoint == mEndpointId, false);
    VerifyOrDie(mCluster.IsConstructed());
    v = mCluster.Cluster().GetContaminationState();
    return true;
}

bool SmokeCoAlarmServer::GetSmokeSensitivityLevel(EndpointId endpoint, SensitivityEnum & v) const
{
    VerifyOrReturnValue(endpoint == mEndpointId, false);
    VerifyOrDie(mCluster.IsConstructed());
    v = mCluster.Cluster().GetSmokeSensitivityLevel();
    return true;
}

bool SmokeCoAlarmServer::GetExpiryDate(EndpointId endpoint, uint32_t & v) const
{
    VerifyOrReturnValue(endpoint == mEndpointId, false);
    VerifyOrDie(mCluster.IsConstructed());
    v = mCluster.Cluster().GetExpiryDate();
    return true;
}

bool SmokeCoAlarmServer::GetUnmountedState(EndpointId endpoint, bool & v) const
{
    VerifyOrReturnValue(endpoint == mEndpointId, false);
    VerifyOrDie(mCluster.IsConstructed());
    v = mCluster.Cluster().GetUnmountedState();
    return true;
}

__attribute__((weak)) void MatterSmokeCoAlarmPluginServerInitCallback() {}
__attribute__((weak)) void MatterSmokeCoAlarmPluginServerShutdownCallback() {}

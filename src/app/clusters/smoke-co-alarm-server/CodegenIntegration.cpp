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
#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SmokeCoAlarm;

SmokeCoAlarmServer SmokeCoAlarmServer::sInstance;

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
    return mCluster.Cluster().RequestSelfTest();
}

void SmokeCoAlarmServer::HandleRemoteSelfTestRequest(EndpointId, CommandHandler * commandObj,
                                                     const ConcreteCommandPath & commandPath)
{
    Cluster().HandleRemoteSelfTestRequest(commandObj, commandPath);
}

void SmokeCoAlarmServer::SetExpressedStateByPriority(EndpointId, const std::array<ExpressedStateEnum, kPriorityOrderLength> & o)
{
    Cluster().SetExpressedStateByPriority(o);
}

bool SmokeCoAlarmServer::SetSmokeState(EndpointId, AlarmStateEnum v)
{
    return Cluster().SetSmokeState(v);
}
bool SmokeCoAlarmServer::SetCOState(EndpointId, AlarmStateEnum v)
{
    return Cluster().SetCOState(v);
}
bool SmokeCoAlarmServer::SetBatteryAlert(EndpointId, AlarmStateEnum v)
{
    return Cluster().SetBatteryAlert(v);
}
bool SmokeCoAlarmServer::SetDeviceMuted(EndpointId, MuteStateEnum v)
{
    return Cluster().SetDeviceMuted(v);
}
bool SmokeCoAlarmServer::SetTestInProgress(EndpointId, bool v)
{
    return Cluster().SetTestInProgress(v);
}
bool SmokeCoAlarmServer::SetHardwareFaultAlert(EndpointId, bool v)
{
    return Cluster().SetHardwareFaultAlert(v);
}
bool SmokeCoAlarmServer::SetEndOfServiceAlert(EndpointId, EndOfServiceEnum v)
{
    return Cluster().SetEndOfServiceAlert(v);
}
bool SmokeCoAlarmServer::SetInterconnectSmokeAlarm(EndpointId, AlarmStateEnum v)
{
    return Cluster().SetInterconnectSmokeAlarm(v);
}
bool SmokeCoAlarmServer::SetInterconnectCOAlarm(EndpointId, AlarmStateEnum v)
{
    return Cluster().SetInterconnectCOAlarm(v);
}
bool SmokeCoAlarmServer::SetContaminationState(EndpointId, ContaminationStateEnum v)
{
    return Cluster().SetContaminationState(v);
}
bool SmokeCoAlarmServer::SetSmokeSensitivityLevel(EndpointId, SensitivityEnum v)
{
    return Cluster().SetSmokeSensitivityLevel(v);
}
bool SmokeCoAlarmServer::SetExpiryDate(EndpointId, uint32_t v)
{
    return Cluster().SetExpiryDate(v);
}
bool SmokeCoAlarmServer::SetUnmountedState(EndpointId, bool v)
{
    return Cluster().SetUnmountedState(v);
}

chip::BitFlags<Feature> SmokeCoAlarmServer::GetFeatures() const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetFeatures();
}

bool SmokeCoAlarmServer::SupportsSmokeAlarm() const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().SupportsSmokeAlarm();
}

bool SmokeCoAlarmServer::SupportsCOAlarm() const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().SupportsCOAlarm();
}

bool SmokeCoAlarmServer::GetExpressedState(EndpointId, ExpressedStateEnum & v) const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetExpressedState(v);
}

bool SmokeCoAlarmServer::GetSmokeState(EndpointId, AlarmStateEnum & v) const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetSmokeState(v);
}

bool SmokeCoAlarmServer::GetCOState(EndpointId, AlarmStateEnum & v) const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetCOState(v);
}

bool SmokeCoAlarmServer::GetBatteryAlert(EndpointId, AlarmStateEnum & v) const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetBatteryAlert(v);
}

bool SmokeCoAlarmServer::GetDeviceMuted(EndpointId, MuteStateEnum & v) const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetDeviceMuted(v);
}

bool SmokeCoAlarmServer::GetTestInProgress(EndpointId, bool & v) const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetTestInProgress(v);
}

bool SmokeCoAlarmServer::GetHardwareFaultAlert(EndpointId, bool & v) const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetHardwareFaultAlert(v);
}

bool SmokeCoAlarmServer::GetEndOfServiceAlert(EndpointId, EndOfServiceEnum & v) const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetEndOfServiceAlert(v);
}

bool SmokeCoAlarmServer::GetInterconnectSmokeAlarm(EndpointId, AlarmStateEnum & v) const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetInterconnectSmokeAlarm(v);
}

bool SmokeCoAlarmServer::GetInterconnectCOAlarm(EndpointId, AlarmStateEnum & v) const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetInterconnectCOAlarm(v);
}

bool SmokeCoAlarmServer::GetContaminationState(EndpointId, ContaminationStateEnum & v) const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetContaminationState(v);
}

bool SmokeCoAlarmServer::GetSmokeSensitivityLevel(EndpointId, SensitivityEnum & v) const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetSmokeSensitivityLevel(v);
}

bool SmokeCoAlarmServer::GetExpiryDate(EndpointId, uint32_t & v) const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetExpiryDate(v);
}

bool SmokeCoAlarmServer::GetUnmountedState(EndpointId, bool & v) const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetUnmountedState(v);
}

chip::BitFlags<Feature> SmokeCoAlarmServer::GetFeatures(EndpointId) const
{
    return GetFeatures();
}
bool SmokeCoAlarmServer::SupportsSmokeAlarm(EndpointId) const
{
    return SupportsSmokeAlarm();
}
bool SmokeCoAlarmServer::SupportsCOAlarm(EndpointId) const
{
    return SupportsCOAlarm();
}

bool __attribute__((weak))
emberAfSmokeCoAlarmClusterSelfTestRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                  const SmokeCoAlarm::Commands::SelfTestRequest::DecodableType & commandData)
{
    SmokeCoAlarmServer::Instance().Cluster().HandleRemoteSelfTestRequest(commandObj, commandPath);
    return true;
}

void __attribute__((weak)) MatterSmokeCoAlarmPluginServerInitCallback() {}
void __attribute__((weak)) MatterSmokeCoAlarmPluginServerShutdownCallback() {}

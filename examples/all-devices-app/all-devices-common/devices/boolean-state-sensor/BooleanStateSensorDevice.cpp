/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <devices/boolean-state-sensor/BooleanStateSensorDevice.h>

using namespace chip::app::Clusters;

namespace chip::app {

CHIP_ERROR BooleanStateSensorDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                              EndpointId parentId)
{
    ReturnErrorOnFailure(SingleEndpointRegistration(endpoint, provider, parentId));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, *mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mBooleanStateCluster.Create(endpoint);
    ReturnErrorOnFailure(provider.AddCluster(mBooleanStateCluster.Registration()));
#if defined(PW_RPC_ENABLED) && PW_RPC_ENABLED
    mPwAccessor.SetDevice(this);
    chip::rpc::PigweedDebugAccessInterceptorRegistry::Instance().Register(&mPwAccessor);
#endif // defined(PW_RPC_ENABLED)

    return provider.AddEndpoint(mEndpointRegistration);
}

void BooleanStateSensorDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    SingleEndpointUnregistration(provider);
    if (mBooleanStateCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mBooleanStateCluster.Cluster()));
        mBooleanStateCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
#if defined(PW_RPC_ENABLED) && PW_RPC_ENABLED
    mPwAccessor.SetDevice(nullptr);
    chip::rpc::PigweedDebugAccessInterceptorRegistry::Instance().Unregister(&mPwAccessor);
#endif // defined(PW_RPC_ENABLED)
}

#if defined(PW_RPC_ENABLED) && PW_RPC_ENABLED
std::optional<::pw::Status> BooleanStateSensor::AttributeAccessor::Write(const ConcreteDataAttributePath & path,
                                                                         AttributeValueDecoder & decoder)
{
    if (!mBooleanStateDevice || path.mEndpointId != mBooleanStateDevice->GetEndpointId())
    {
        return std::nullopt;
    }
    switch (path.mClusterId)
    {
    case BooleanState::Id:
        switch (path.mAttributeId)
        {
        case BooleanState::Attributes::StateValue::Id: {
            bool stateValue;
            CHIP_ERROR err = decoder.Decode(stateValue);
            if (err != CHIP_NO_ERROR)
            {
                return ::pw::Status::Internal();
            }
            mBooleanStateDevice->BooleanState().SetStateValue(stateValue);
            return ::pw::OkStatus();
        }
        }
        break;
    }
    return std::nullopt;
}
#endif // defined(PW_RPC_ENABLED)

} // namespace chip::app

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

#include "ModeSelect.h"
#include <devices/Types.h>

namespace chip {
namespace app {

ModeSelect::ModeSelect(const Config & config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kModeSelect, 1)), mConfig(config)
{}

CHIP_ERROR ModeSelect::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    DeviceRegistrationTransaction transaction(*this, provider);
    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mModeSelectCluster.Create(endpoint, mConfig.delegate, mConfig.clusterConfig);
    ReturnErrorOnFailure(provider.AddCluster(mModeSelectCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void ModeSelect::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mModeSelectCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mModeSelectCluster.Cluster()));
        mModeSelectCluster.Destroy();
    }
}

} // namespace app
} // namespace chip

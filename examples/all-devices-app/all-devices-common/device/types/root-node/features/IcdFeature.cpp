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
#include <device/types/root-node/features/IcdFeature.h>

#if CHIP_CONFIG_ENABLE_ICD_SERVER

#include <app/icd/server/ICDConfigurationData.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

CHIP_ERROR IcdFeature::RegisterFeatureClusters(EndpointId endpointId, CodeDrivenDataModelProvider & provider,
                                               RootNode::Context & rootContext, Clusters::GeneralCommissioningCluster &)
{
#if CHIP_CONFIG_ENABLE_ICD_CIP
    using ClusterType = ICDManagementClusterWithCIP;
#else
    using ClusterType = ICDManagementCluster;
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

    constexpr ClusterType::OptionalCommandSet enabledCommands =
#if CHIP_CONFIG_ENABLE_ICD_LIT
        ClusterType::OptionalCommandSet().Set<IcdManagement::Commands::StayActiveRequest::Id>();
#else
        ClusterType::OptionalCommandSet();
#endif // CHIP_CONFIG_ENABLE_ICD_LIT

    mIcdManagementCluster.Create(endpointId, mContext.symmetricKeystore, rootContext.fabricTable,
                                 ICDConfigurationData::GetInstance(), ClusterType::OptionalAttributeSet(0), enabledCommands,
                                 BitMask<IcdManagement::UserActiveModeTriggerBitmap>(0), CharSpan());
    return provider.AddCluster(mIcdManagementCluster.Registration());
}

void IcdFeature::UnregisterFeatureClusters(CodeDrivenDataModelProvider & provider)
{
    if (mIcdManagementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIcdManagementCluster.Cluster()));
        mIcdManagementCluster.Destroy();
    }
}

} // namespace app
} // namespace chip

#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

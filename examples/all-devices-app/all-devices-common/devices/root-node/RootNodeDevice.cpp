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
#include <devices/root-node/RootNodeDevice.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/InteractionModelEngine.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;
namespace chip {
namespace app {

CHIP_ERROR RootNodeDevice::Register(EndpointId endpointId, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    ReturnErrorOnFailure(SingleEndpointRegistration(endpointId, provider, parentId));

    // TODO: This needs to be refactored so the optional attributes, commands and features being set for
    //  the cluster are configurable to allow different settings
    const BasicInformationCluster::OptionalAttributesSet optionalAttributeSet =
        BasicInformationCluster::OptionalAttributesSet()
            .Set<BasicInformation::Attributes::ManufacturingDate::Id>()
            .Set<BasicInformation::Attributes::PartNumber::Id>()
            .Set<BasicInformation::Attributes::ProductURL::Id>()
            .Set<BasicInformation::Attributes::ProductLabel::Id>()
            .Set<BasicInformation::Attributes::SerialNumber::Id>()
            .Set<BasicInformation::Attributes::LocalConfigDisabled::Id>()
            .Set<BasicInformation::Attributes::Reachable::Id>();

    mBasicInformationCluster.Create(
        optionalAttributeSet,
        BasicInformationCluster::Context{ .deviceInstanceInfoProvider = mContext.deviceInstanceInfoProvider,
                                          .configurationManager       = mContext.configurationManager,
                                          .platformManager            = mContext.platformManager });

    ReturnErrorOnFailure(provider.AddCluster(mBasicInformationCluster.Registration()));
    mGeneralCommissioningCluster.Create(
        GeneralCommissioningCluster::Context {
            .commissioningWindowManager = mContext.commissioningWindowManager, //
                .configurationManager   = mContext.configurationManager,       //
                .deviceControlServer    = mContext.deviceControlServer,        //
                .fabricTable            = mContext.fabricTable,                //
                .failSafeContext        = mContext.failSafeContext,            //
                .platformManager        = mContext.platformManager,            //
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
                .termsAndConditionsProvider = mContext.termsAndConditionsProvider,
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
        },
        GeneralCommissioningCluster::OptionalAttributes());
    ReturnErrorOnFailure(provider.AddCluster(mGeneralCommissioningCluster.Registration()));

    mAdministratorCommissioningCluster.Create(
        endpointId, BitFlags<AdministratorCommissioning::Feature>{},
        AdministratorCommissioningCluster::Context{ .commissioningWindowManager = mContext.commissioningWindowManager,
                                                    .fabricTable                = mContext.fabricTable,
                                                    // Note: We pull FailSafeContext directly from Server instead of via
                                                    // CommissioningWindowManager because the WindowManager's internal
                                                    // Server pointer is not yet initialized at this stage of registration.
                                                    .failSafeContext = mContext.failSafeContext });
    ReturnErrorOnFailure(provider.AddCluster(mAdministratorCommissioningCluster.Registration()));

    mGeneralDiagnosticsCluster.Create(GeneralDiagnosticsCluster::OptionalAttributeSet{}, BitFlags<GeneralDiagnostics::Feature>{},
                                      GeneralDiagnosticsCluster::Context{
                                          .deviceLoadStatusProvider = mContext.deviceLoadStatusProvider,
                                          .diagnosticDataProvider   = mContext.diagnosticDataProvider,
                                          .testEventTriggerDelegate = mContext.testEventTriggerDelegate,
                                      });
    ReturnErrorOnFailure(provider.AddCluster(mGeneralDiagnosticsCluster.Registration()));

    mGroupKeyManagementCluster.Create(GroupKeyManagementCluster::Context{
        .fabricTable       = mContext.fabricTable,
        .groupDataProvider = mContext.groupDataProvider,
    });
    ReturnErrorOnFailure(provider.AddCluster(mGroupKeyManagementCluster.Registration()));

    mSoftwareDiagnosticsServerCluster.Create(SoftwareDiagnosticsLogic::OptionalAttributeSet{});
    ReturnErrorOnFailure(provider.AddCluster(mSoftwareDiagnosticsServerCluster.Registration()));

    mAccessControlCluster.Create(AccessControlCluster::Context{
        .persistentStorage = mContext.persistentStorage,
        .fabricTable       = mContext.fabricTable,
        .accessControl     = mContext.accessControl,
    });
    ReturnErrorOnFailure(provider.AddCluster(mAccessControlCluster.Registration()));

    mOperationalCredentialsCluster.Create(endpointId,
                                          OperationalCredentialsCluster::Context{
                                              .fabricTable                = mContext.fabricTable,
                                              .failSafeContext            = mContext.failSafeContext,
                                              .sessionManager             = mContext.sessionManager,
                                              .dnssdServer                = mContext.dnssdServer,
                                              .commissioningWindowManager = mContext.commissioningWindowManager,
                                          });
    ReturnErrorOnFailure(provider.AddCluster(mOperationalCredentialsCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void RootNodeDevice::UnRegister(CodeDrivenDataModelProvider & provider)
{
    SingleEndpointUnregistration(provider);
    if (mBasicInformationCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mBasicInformationCluster.Cluster()));
        mBasicInformationCluster.Destroy();
    }
    if (mGeneralCommissioningCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mGeneralCommissioningCluster.Cluster()));
        mGeneralCommissioningCluster.Destroy();
    }
    if (mAdministratorCommissioningCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mAdministratorCommissioningCluster.Cluster()));
        mAdministratorCommissioningCluster.Destroy();
    }
    if (mGeneralDiagnosticsCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mGeneralDiagnosticsCluster.Cluster()));
        mGeneralDiagnosticsCluster.Destroy();
    }
    if (mGroupKeyManagementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mGroupKeyManagementCluster.Cluster()));
        mGroupKeyManagementCluster.Destroy();
    }
    if (mSoftwareDiagnosticsServerCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mSoftwareDiagnosticsServerCluster.Cluster()));
        mSoftwareDiagnosticsServerCluster.Destroy();
    }
    if (mAccessControlCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mAccessControlCluster.Cluster()));
        mAccessControlCluster.Destroy();
    }
    if (mOperationalCredentialsCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mOperationalCredentialsCluster.Cluster()));
        mOperationalCredentialsCluster.Destroy();
    }
}

} // namespace app
} // namespace chip

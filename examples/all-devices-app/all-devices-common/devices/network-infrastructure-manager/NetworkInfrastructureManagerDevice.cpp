/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/server/Server.h>
#include <devices/Types.h>
#include <devices/network-infrastructure-manager/NetworkInfrastructureManagerDevice.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

inline ByteSpan ByteSpanFromCharSpan(CharSpan span)
{
    return ByteSpan(Uint8::from_const_char(span.data()), span.size());
}

namespace {
constexpr uint16_t kThreadVersionForThread_1_3_1 = 5;
} // namespace

NetworkInfrastructureManagerDevice::NetworkInfrastructureManagerDevice(PersistentStorageDelegate & storage) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kNetworkInfrastructureManager, 1)),
    mThreadNetworkDirectoryStorage(storage)
{}

NetworkInfrastructureManagerDevice::~NetworkInfrastructureManagerDevice()
{
    DeviceLayer::SystemLayer().CancelTimer(ActivateActiveDataset, this);
    DeviceLayer::SystemLayer().CancelTimer(ActivatePendingDataset, this);
}

CHIP_ERROR NetworkInfrastructureManagerDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                        EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    // 1. Thread Border Router Management
    ThreadBorderRouterManagementCluster::Config tbrConfig(*this, Server::GetInstance().GetFailSafeContext(), mBreadCrumbTracker,
                                                          DeviceLayer::PlatformMgr());
    mThreadBorderRouterManagementCluster.Create(endpoint, tbrConfig);
    ReturnErrorOnFailure(provider.AddCluster(mThreadBorderRouterManagementCluster.Registration()));

    // 2. WiFi Network Management
    mWiFiNetworkManagementCluster.Create(endpoint);
    ReturnErrorOnFailure(provider.AddCluster(mWiFiNetworkManagementCluster.Registration()));
    ReturnErrorOnFailure(mWiFiNetworkManagementCluster.Cluster().SetNetworkCredentials(
        ByteSpanFromCharSpan("MatterAP"_span), ByteSpanFromCharSpan("Setec Astronomy"_span)));

    // 3. Thread Network Directory
    mThreadNetworkDirectoryCluster.Create(endpoint, mThreadNetworkDirectoryStorage);
    ReturnErrorOnFailure(provider.AddCluster(mThreadNetworkDirectoryCluster.Registration()));

    // 4. Thread Network Diagnostics
    mThreadNetworkDiagnosticsCluster.Create(endpoint, ThreadNetworkDiagnosticsCluster::ClusterType::kFull,
                                            mThreadDiagnosticsProvider);
    ReturnErrorOnFailure(provider.AddCluster(mThreadNetworkDiagnosticsCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void NetworkInfrastructureManagerDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);

    if (mThreadNetworkDiagnosticsCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mThreadNetworkDiagnosticsCluster.Cluster()));
        mThreadNetworkDiagnosticsCluster.Destroy();
    }
    if (mThreadNetworkDirectoryCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mThreadNetworkDirectoryCluster.Cluster()));
        mThreadNetworkDirectoryCluster.Destroy();
    }
    if (mWiFiNetworkManagementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mWiFiNetworkManagementCluster.Cluster()));
        mWiFiNetworkManagementCluster.Destroy();
    }
    if (mThreadBorderRouterManagementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mThreadBorderRouterManagementCluster.Cluster()));
        mThreadBorderRouterManagementCluster.Destroy();
    }
}

CHIP_ERROR NetworkInfrastructureManagerDevice::Init(AttributeChangeCallback * attributeChangeCallback)
{
    ChipLogProgress(AppServer, "NetworkInfrastructureManagerDevice::Init called");
    mAttributeChangeCallback = attributeChangeCallback;
    return CHIP_NO_ERROR;
}

bool NetworkInfrastructureManagerDevice::GetPanChangeSupported()
{
    ChipLogProgress(AppServer, "NetworkInfrastructureManagerDevice::GetPanChangeSupported called");
    return true;
}

void NetworkInfrastructureManagerDevice::GetBorderRouterName(MutableCharSpan & borderRouterName)
{
    ChipLogProgress(AppServer, "NetworkInfrastructureManagerDevice::GetBorderRouterName called");
    CopyCharSpanToMutableCharSpanWithTruncation("all-devices-br"_span, borderRouterName);
}

CHIP_ERROR NetworkInfrastructureManagerDevice::GetBorderAgentId(MutableByteSpan & borderAgentId)
{
    ChipLogProgress(AppServer, "NetworkInfrastructureManagerDevice::GetBorderAgentId called");
    static constexpr uint8_t kBorderAgentId[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                  0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
    VerifyOrReturnError(borderAgentId.size() == Clusters::ThreadBorderRouterManagementDelegate::kBorderAgentIdLength,
                        CHIP_ERROR_INVALID_ARGUMENT);
    return CopySpanToMutableSpan(ByteSpan(kBorderAgentId), borderAgentId);
}

uint16_t NetworkInfrastructureManagerDevice::GetThreadVersion()
{
    ChipLogProgress(AppServer, "NetworkInfrastructureManagerDevice::GetThreadVersion called");
    return kThreadVersionForThread_1_3_1;
}

bool NetworkInfrastructureManagerDevice::GetInterfaceEnabled()
{
    ChipLogProgress(AppServer, "NetworkInfrastructureManagerDevice::GetInterfaceEnabled called");
    return !mActiveDataset.IsEmpty();
}

CHIP_ERROR NetworkInfrastructureManagerDevice::GetDataset(Thread::OperationalDataset & dataset, DatasetType type)
{
    ChipLogProgress(AppServer, "NetworkInfrastructureManagerDevice::GetDataset called (type: %d)", static_cast<int>(type));
    Thread::OperationalDataset * source;
    switch (type)
    {
    case DatasetType::kActive:
        source = &mActiveDataset;
        break;
    case DatasetType::kPending:
        source = &mPendingDataset;
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    VerifyOrReturnError(!source->IsEmpty(), CHIP_ERROR_NOT_FOUND);
    return dataset.Init(source->AsByteSpan());
}

void NetworkInfrastructureManagerDevice::SetActiveDataset(const Thread::OperationalDataset & activeDataset, uint32_t sequenceNum,
                                                          ActivateDatasetCallback * callback)
{
    ChipLogProgress(AppServer, "NetworkInfrastructureManagerDevice::SetActiveDataset called (seq: %" PRIu32 ")", sequenceNum);
    if (mActivateDatasetCallback != nullptr)
    {
        callback->OnActivateDatasetComplete(sequenceNum, CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    CHIP_ERROR err = mActiveDataset.Init(activeDataset.AsByteSpan());
    if (err != CHIP_NO_ERROR)
    {
        callback->OnActivateDatasetComplete(sequenceNum, err);
        return;
    }

    mActivateDatasetCallback = callback;
    mActivateDatasetSequence = sequenceNum;
    DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(1000), ActivateActiveDataset, this);
}

CHIP_ERROR NetworkInfrastructureManagerDevice::CommitActiveDataset()
{
    ChipLogProgress(AppServer, "NetworkInfrastructureManagerDevice::CommitActiveDataset called");
    return CHIP_NO_ERROR;
}

CHIP_ERROR NetworkInfrastructureManagerDevice::RevertActiveDataset()
{
    ChipLogProgress(AppServer, "NetworkInfrastructureManagerDevice::RevertActiveDataset called");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR NetworkInfrastructureManagerDevice::SetPendingDataset(const Thread::OperationalDataset & pendingDataset)
{
    ChipLogProgress(AppServer, "NetworkInfrastructureManagerDevice::SetPendingDataset called");
    ReturnErrorOnFailure(mPendingDataset.Init(pendingDataset.AsByteSpan()));
    uint32_t delayTimerMillis;
    ReturnErrorOnFailure(mPendingDataset.GetDelayTimer(delayTimerMillis));
    return DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(delayTimerMillis), ActivatePendingDataset, this);
}

void NetworkInfrastructureManagerDevice::ActivateActiveDataset(System::Layer *, void * context)
{
    auto * self                    = static_cast<NetworkInfrastructureManagerDevice *>(context);
    auto * callback                = self->mActivateDatasetCallback;
    auto sequenceNum               = self->mActivateDatasetSequence;
    self->mActivateDatasetCallback = nullptr;
    callback->OnActivateDatasetComplete(sequenceNum, CHIP_NO_ERROR);
}

void NetworkInfrastructureManagerDevice::ActivatePendingDataset(System::Layer *, void * context)
{
    auto * self = static_cast<NetworkInfrastructureManagerDevice *>(context);
    TEMPORARY_RETURN_IGNORED self->mActiveDataset.Init(self->mPendingDataset.AsByteSpan());
    self->mPendingDataset.Clear();
    if (self->mAttributeChangeCallback)
    {
        self->mAttributeChangeCallback->ReportAttributeChanged(
            Clusters::ThreadBorderRouterManagement::Attributes::ActiveDatasetTimestamp::Id);
        self->mAttributeChangeCallback->ReportAttributeChanged(
            Clusters::ThreadBorderRouterManagement::Attributes::PendingDatasetTimestamp::Id);
    }
}

} // namespace app
} // namespace chip

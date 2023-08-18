/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *    @file
 *          Provides a generic implementation of ConnectivityManager features
 *          for use on platforms that do NOT support Thread.
 */

#pragma once
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/AttributeAccessInterface.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides a generic implementation of WiFi-specific ConnectivityManager features for
 * use on platforms that do NOT support Thread.
 *
 * This class is intended to be inherited (directly or indirectly) by the ConnectivityManagerImpl
 * class, which also appears as the template's ImplClass parameter.
 *
 */
template <class ImplClass>
class GenericConnectivityManagerImpl_NoThread
{
protected:
    // ===== Methods that implement the ConnectivityManager abstract interface.

    ConnectivityManager::ThreadMode _GetThreadMode(void);
    CHIP_ERROR _SetThreadMode(ConnectivityManager::ThreadMode val);
    bool _IsThreadEnabled(void);
    bool _IsThreadApplicationControlled(void);
    ConnectivityManager::ThreadDeviceType _GetThreadDeviceType(void);
    CHIP_ERROR _SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType);
    bool _IsThreadAttached(void);
    bool _IsThreadProvisioned(void);
    void _ErasePersistentInfo(void);
    void _ResetThreadNetworkDiagnosticsCounts(void);
    CHIP_ERROR _WriteThreadNetworkDiagnosticAttributeToTlv(AttributeId attributeId, app::AttributeValueEncoder & encoder);

    ImplClass * Impl() { return static_cast<ImplClass *>(this); }
};

template <class ImplClass>
inline ConnectivityManager::ThreadMode GenericConnectivityManagerImpl_NoThread<ImplClass>::_GetThreadMode(void)
{
    return ConnectivityManager::kThreadMode_NotSupported;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_NoThread<ImplClass>::_SetThreadMode(ConnectivityManager::ThreadMode val)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_NoThread<ImplClass>::_IsThreadEnabled(void)
{
    return false;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_NoThread<ImplClass>::_IsThreadApplicationControlled(void)
{
    return false;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_NoThread<ImplClass>::_IsThreadAttached(void)
{
    return false;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_NoThread<ImplClass>::_IsThreadProvisioned(void)
{
    return false;
}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_NoThread<ImplClass>::_ErasePersistentInfo(void)
{}

template <class ImplClass>
inline ConnectivityManager::ThreadDeviceType GenericConnectivityManagerImpl_NoThread<ImplClass>::_GetThreadDeviceType(void)
{
    return ConnectivityManager::kThreadDeviceType_NotSupported;
}

template <class ImplClass>
inline CHIP_ERROR
GenericConnectivityManagerImpl_NoThread<ImplClass>::_SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_NoThread<ImplClass>::_ResetThreadNetworkDiagnosticsCounts()
{}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_NoThread<ImplClass>::_WriteThreadNetworkDiagnosticAttributeToTlv(
    AttributeId attributeId, app::AttributeValueEncoder & encoder)
{
    // If we get here the Thread Network Diagnostic cluster is enabled on the device but doesn't run thread.
    // Encode Null or default values for all attributes of the cluster.
    CHIP_ERROR err = CHIP_NO_ERROR;
    switch (attributeId)
    {
    // Encode EmptyList
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RouteTable::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::NeighborTable::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::ActiveNetworkFaultsList::Id:
        err = encoder.EncodeEmptyList();
        break;
    // Encode Null
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::Channel::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RoutingRole::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::NetworkName::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::PanId::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::ExtendedPanId::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::MeshLocalPrefix::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::PartitionId::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::Weighting::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::DataVersion::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::StableDataVersion::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::LeaderRouterId::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::ActiveTimestamp::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::PendingTimestamp::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::Delay::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::ChannelPage0Mask::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::SecurityPolicy::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::OperationalDatasetComponents::Id:
        err = encoder.EncodeNull();
        break;
    // Encode UINT64_T 0
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::OverrunCount::Id:
        err = encoder.Encode(static_cast<uint64_t>(0));
        break;
    // Encode UINT16_T 0
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::DetachedRoleCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::ChildRoleCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RouterRoleCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::LeaderRoleCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::AttachAttemptCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::PartitionIdChangeCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::BetterPartitionAttachAttemptCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::ParentChangeCount::Id:
        err = encoder.Encode(static_cast<uint16_t>(0));
        break;
    // Encode UINT32_T 0
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxTotalCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxUnicastCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxBroadcastCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxAckedCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxAckRequestedCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxNoAckRequestedCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxDataCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxDataPollCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxBeaconCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxBeaconRequestCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxOtherCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxRetryCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxDirectMaxRetryExpiryCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxIndirectMaxRetryExpiryCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxErrCcaCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxErrAbortCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxErrBusyChannelCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxTotalCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxUnicastCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxBroadcastCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxDataCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxDataPollCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxBeaconCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxBeaconRequestCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxOtherCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxAddressFilteredCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxDestAddrFilteredCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxDuplicatedCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxErrNoFrameCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxErrUnknownNeighborCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxErrInvalidSrcAddrCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxErrSecCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxErrFcsCount::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxErrOtherCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    default:
        err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        break;
    }

    return err;
}
} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

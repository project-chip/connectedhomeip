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
    CHIP_ERROR _GetSEDIntervalsConfig(ConnectivityManager::SEDIntervalsConfig & intervalsConfig);
    CHIP_ERROR _SetSEDIntervalsConfig(const ConnectivityManager::SEDIntervalsConfig & intervalsConfig);
    CHIP_ERROR _RequestSEDActiveMode(bool onOff);
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
inline CHIP_ERROR GenericConnectivityManagerImpl_NoThread<ImplClass>::_GetSEDIntervalsConfig(
    ConnectivityManager::SEDIntervalsConfig & intervalsConfig)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_NoThread<ImplClass>::_SetSEDIntervalsConfig(
    const ConnectivityManager::SEDIntervalsConfig & intervalsConfig)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_NoThread<ImplClass>::_RequestSEDActiveMode(bool onOff)
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
    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (attributeId)
    {
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::NeighborTableList::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RouteTableList::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::ActiveNetworkFaultsList::Id:
        err = encoder.EncodeEmptyList();
        break;
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
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::ChannelMask::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::SecurityPolicy::Id:
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::OperationalDatasetComponents::Id:
        err = encoder.EncodeNull();
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::OverrunCount::Id:
        err = encoder.Encode(static_cast<uint64_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::DetachedRoleCount::Id:
        err = encoder.Encode(static_cast<uint16_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::ChildRoleCount::Id:
        err = encoder.Encode(static_cast<uint16_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RouterRoleCount::Id:
        err = encoder.Encode(static_cast<uint16_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::LeaderRoleCount::Id:
        err = encoder.Encode(static_cast<uint16_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::AttachAttemptCount::Id:
        err = encoder.Encode(static_cast<uint16_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::PartitionIdChangeCount::Id:
        err = encoder.Encode(static_cast<uint16_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::BetterPartitionAttachAttemptCount::Id:
        err = encoder.Encode(static_cast<uint16_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::ParentChangeCount::Id:
        err = encoder.Encode(static_cast<uint16_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxTotalCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxUnicastCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxBroadcastCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxAckRequestedCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxAckedCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxNoAckRequestedCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxDataCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxDataPollCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxBeaconCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxBeaconRequestCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxOtherCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxRetryCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxDirectMaxRetryExpiryCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxIndirectMaxRetryExpiryCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxErrCcaCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxErrAbortCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::TxErrBusyChannelCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxTotalCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxUnicastCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxBroadcastCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxDataCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxDataPollCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxBeaconCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxBeaconRequestCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxOtherCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxAddressFilteredCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxDestAddrFilteredCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxDuplicatedCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxErrNoFrameCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxErrUnknownNeighborCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxErrInvalidSrcAddrCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxErrSecCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case app::Clusters::ThreadNetworkDiagnostics::Attributes::RxErrFcsCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
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

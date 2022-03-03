/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *      Implementation of SetUp Code Pairer, a class that parses a given
 *      setup code and uses the extracted informations to discover and
 *      filter commissionables nodes, before initiating the pairing process.
 *
 */

#include <controller/SetUpCodePairer.h>

#include <controller/CHIPDeviceController.h>
#include <lib/dnssd/Resolver.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace Controller {

CHIP_ERROR SetUpCodePairer::PairDevice(NodeId remoteId, const char * setUpCode)
{
    SetupPayload payload;

    bool isQRCode = strncmp(setUpCode, kQRCodePrefix, strlen(kQRCodePrefix)) == 0;
    ReturnErrorOnFailure(isQRCode ? QRCodeSetupPayloadParser(setUpCode).populatePayload(payload)
                                  : ManualSetupPayloadParser(setUpCode).populatePayload(payload));

    mRemoteId     = remoteId;
    mSetUpPINCode = payload.setUpPINCode;

    return Connect(payload);
}

CHIP_ERROR SetUpCodePairer::Connect(SetupPayload & payload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool isRunning = false;

    bool searchOverAll = payload.rendezvousInformation == RendezvousInformationFlag::kNone;
    if (searchOverAll || payload.rendezvousInformation == RendezvousInformationFlag::kBLE)
    {
        if (CHIP_NO_ERROR == (err = StartDiscoverOverBle(payload)))
        {
            isRunning = true;
        }
        VerifyOrReturnError(searchOverAll || CHIP_NO_ERROR == err, err);
    }

    if (searchOverAll || payload.rendezvousInformation == RendezvousInformationFlag::kSoftAP)
    {
        if (CHIP_NO_ERROR == (err = StartDiscoverOverSoftAP(payload)))
        {
            isRunning = true;
        }
        VerifyOrReturnError(searchOverAll || CHIP_NO_ERROR == err, err);
    }

    // We always want to search on network because any node that has already been commissioned will use on-network regardless of the
    // QR code flag.
    if (CHIP_NO_ERROR == (err = StartDiscoverOverIP(payload)))
    {
        isRunning = true;
    }
    VerifyOrReturnError(searchOverAll || CHIP_NO_ERROR == err, err);

    return isRunning ? CHIP_NO_ERROR : CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR SetUpCodePairer::StartDiscoverOverBle(SetupPayload & payload)
{
#if CONFIG_NETWORK_LAYER_BLE
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    VerifyOrReturnError(mCommissioner != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mCommissioner->ConnectBleTransportToSelf();
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    VerifyOrReturnError(mBleLayer != nullptr, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    return mBleLayer->NewBleConnectionByDiscriminator(payload.discriminator, this, OnDiscoveredDeviceOverBleSuccess,
                                                      OnDiscoveredDeviceOverBleError);
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // CONFIG_NETWORK_LAYER_BLE
}

CHIP_ERROR SetUpCodePairer::StopConnectOverBle()
{
#if CONFIG_NETWORK_LAYER_BLE
    VerifyOrReturnError(mBleLayer != nullptr, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    return mBleLayer->CancelBleIncompleteConnection();
#else
    return CHIP_NO_ERROR;
#endif // CONFIG_NETWORK_LAYER_BLE
}

CHIP_ERROR SetUpCodePairer::StartDiscoverOverIP(SetupPayload & payload)
{
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    currentFilter.type = payload.isShortDiscriminator ? Dnssd::DiscoveryFilterType::kShortDiscriminator
                                                      : Dnssd::DiscoveryFilterType::kLongDiscriminator;
    currentFilter.code =
        payload.isShortDiscriminator ? static_cast<uint16_t>((payload.discriminator >> 8) & 0x0F) : payload.discriminator;
    return mCommissioner->DiscoverCommissionableNodes(currentFilter);
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // CHIP_DEVICE_CONFIG_ENABLE_DNSSD
}

CHIP_ERROR SetUpCodePairer::StopConnectOverIP()
{
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    currentFilter.type = Dnssd::DiscoveryFilterType::kNone;
#endif // CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    return CHIP_NO_ERROR;
}

CHIP_ERROR SetUpCodePairer::StartDiscoverOverSoftAP(SetupPayload & payload)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR SetUpCodePairer::StopConnectOverSoftAP()
{
    return CHIP_NO_ERROR;
}

void SetUpCodePairer::OnDeviceDiscovered(RendezvousParameters & params)
{
    LogErrorOnFailure(mCommissioner->PairDevice(mRemoteId, params.SetSetupPINCode(mSetUpPINCode)));
}

#if CONFIG_NETWORK_LAYER_BLE
void SetUpCodePairer::OnDiscoveredDeviceOverBle(BLE_CONNECTION_OBJECT connObj)
{
    LogErrorOnFailure(StopConnectOverIP());
    LogErrorOnFailure(StopConnectOverSoftAP());

    Transport::PeerAddress peerAddress = Transport::PeerAddress::BLE();
    RendezvousParameters params        = RendezvousParameters().SetPeerAddress(peerAddress).SetConnectionObject(connObj);
    // We don't have network credentials, so can't do the entire pairing flow.  Just establish a PASE session to the
    //  device and let our consumer deal with the rest.
    LogErrorOnFailure(mCommissioner->EstablishPASEConnection(mRemoteId, params.SetSetupPINCode(mSetUpPINCode)));
}

void SetUpCodePairer::OnDiscoveredDeviceOverBleSuccess(void * appState, BLE_CONNECTION_OBJECT connObj)
{
    (static_cast<SetUpCodePairer *>(appState))->OnDiscoveredDeviceOverBle(connObj);
}

void SetUpCodePairer::OnDiscoveredDeviceOverBleError(void * appState, CHIP_ERROR err)
{
    LogErrorOnFailure(err);
}
#endif // CONFIG_NETWORK_LAYER_BLE

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD

bool SetUpCodePairer::NodeMatchesCurrentFilter(const Dnssd::DiscoveredNodeData & nodeData)
{
    if (nodeData.commissioningMode == 0)
    {
        return false;
    }

    switch (currentFilter.type)
    {
    case Dnssd::DiscoveryFilterType::kShortDiscriminator:
        return ((nodeData.longDiscriminator >> 8) & 0x0F) == currentFilter.code;
    case Dnssd::DiscoveryFilterType::kLongDiscriminator:
        return nodeData.longDiscriminator == currentFilter.code;
    default:
        return false;
    }
    return false;
}

void SetUpCodePairer::NotifyCommissionableDeviceDiscovered(const Dnssd::DiscoveredNodeData & nodeData)
{
    if (!NodeMatchesCurrentFilter(nodeData))
    {
        return;
    }
    LogErrorOnFailure(StopConnectOverBle());
    LogErrorOnFailure(StopConnectOverIP());
    LogErrorOnFailure(StopConnectOverSoftAP());

    Inet::InterfaceId interfaceId      = nodeData.ipAddress[0].IsIPv6LinkLocal() ? nodeData.interfaceId : Inet::InterfaceId::Null();
    Transport::PeerAddress peerAddress = Transport::PeerAddress::UDP(nodeData.ipAddress[0], nodeData.port, interfaceId);
    RendezvousParameters params        = RendezvousParameters().SetPeerAddress(peerAddress);
    OnDeviceDiscovered(params);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_DNSSD

} // namespace Controller
} // namespace chip

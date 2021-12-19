/*
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#pragma once

#include <commissioner/Commissionee.h>
#include <controller/AbstractDnssdDiscoveryController.h>
#include <setup_payload/SetupPayload.h>

namespace chip {
namespace Commissioner {
namespace CommissionableNodeDiscoverer {

struct Delegate
{
    virtual ~Delegate()                 = default;
    virtual void OnDiscovery()          = 0;
    virtual void OnDiscovererShutdown() = 0;
};

struct ShutdownToken
{
    ShutdownToken(Delegate ** delegate) : mDelegate(delegate) {}
    ~ShutdownToken() { (*mDelegate)->OnDiscovererShutdown(); }

private:
    Delegate ** mDelegate;
};

struct Joinable
{
    Joinable(Delegate ** delegate) : mDelegate(delegate) {}

protected:
    bool InProgress() { return mShutdownToken.get() != nullptr ? true : false; }
    void RetainShutdownToken(Platform::SharedPtr<ShutdownToken> token) { mShutdownToken = token; }
    void ReleaseShutdownToken() { mShutdownToken = std::shared_ptr<ShutdownToken>(nullptr); }
    Platform::SharedPtr<ShutdownToken> GetShutdownToken() { return mShutdownToken; }

    Delegate ** mDelegate;

private:
    Platform::SharedPtr<ShutdownToken> mShutdownToken;
};

#if CONFIG_NETWORK_LAYER_BLE
class BleDiscoverer : Joinable
{
public:
    BleDiscoverer(Controller::DeviceControllerSystemState & systemState, Delegate ** delegate);
    ~BleDiscoverer();

    void Shutdown();
    CHIP_ERROR StartBleDiscovery(SetupPayload & payload, Platform::SharedPtr<ShutdownToken> token);
    CHIP_ERROR GetNextBleCandidate(Commissionee & commissionee);
    void TallyBleDiscovery(BLE_CONNECTION_OBJECT connection);
    void TallyBleFailure();

private:
    static void OnBleDiscoverySuccess(void * context, BLE_CONNECTION_OBJECT connection);
    static void OnBleDiscoveryError(void * context, CHIP_ERROR err);
    Controller::DeviceControllerSystemState & mSystemState;
    chip::Optional<BLE_CONNECTION_OBJECT> mBleConnection;
};
#endif // CONFIG_NETWORK_LAYER_BLE

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
class DnssdDiscoverer : Controller::AbstractDnssdDiscoveryController, Controller::DeviceDiscoveryDelegate, Joinable
{
public:
    DnssdDiscoverer(Controller::DeviceControllerSystemState & systemState, Delegate ** delegate);
    ~DnssdDiscoverer();

    void Shutdown();
    CHIP_ERROR StartDnssdDiscovery(SetupPayload & payload, Platform::SharedPtr<ShutdownToken> token);
    CHIP_ERROR GetNextDnssdCandidate(Commissionee & commissionee);

private:
    DiscoveredNodeList GetDiscoveredNodes() { return DiscoveredNodeList(mNodes); }
    void OnDiscoveredDevice(const chip::Dnssd::DiscoveredNodeData & node) { (*mDelegate)->OnDiscovery(); }
    void OnNodeIdResolved(const Dnssd::ResolvedNodeData & nodeData) {}
    void OnNodeIdResolutionFailed(const PeerId & peerId, CHIP_ERROR error) {}

    CHIP_ERROR GetValidRecord();
    CHIP_ERROR GetAddress(chip::Optional<Transport::PeerAddress> & address,
                          chip::Optional<ReliableMessageProtocolConfig> & mrpConfig);

    Controller::DeviceControllerSystemState & mSystemState;
    Dnssd::DiscoveredNodeData mNodes[CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES];
    size_t mNodeIdx = 0;
    int mIpIdx      = 0;
};
#endif // CHIP_DEVICE_CONFIG_ENABLE_DNSSD

class Discoverer : Joinable
{
public:
    Discoverer(Controller::DeviceControllerSystemState & systemState, Platform::SharedPtr<SetupPayload> payload,
               Delegate * delegate);

    CHIP_ERROR Init();
    void Shutdown();
    void SetDelegate(Delegate * delegate);
    CHIP_ERROR Discover();
    CHIP_ERROR GetNextCandidate(Commissionee & commissionee);

private:
    Delegate * mDelegate;
    Platform::SharedPtr<SetupPayload> mPayload;
#if CONFIG_NETWORK_LAYER_BLE
    BleDiscoverer mBleDiscoverer;
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    DnssdDiscoverer mDnssdDiscoverer;
#endif
};

} // namespace CommissionableNodeDiscoverer
} // namespace Commissioner
} // namespace chip

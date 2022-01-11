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

/**
 *  @struct Delegate
 *
 *  @brief
 *    Delegate interface for Discoverer.
 */
struct Delegate
{
    virtual ~Delegate() = default;

    /**
     * @brief Called on the Delegate when a commissionable node candidate is
     *        discovered.
     */
    virtual void OnDiscovery() = 0;

    /**
     * @brief Called on the Delegate when the discoverer shutdown sequence
     *        completes.
     */
    virtual void OnShutdownComplete() = 0;
};

/**
 *  @struct ShutdownToken
 *
 *  @brief
 *    Constructs with a reference to the discoverer delegate and calls
 *    OnShutdownComplete on the delegate from its destructor.
 *
 *    This can be wrapped in a Platform::SharedPtr such that at decrement of
 *    the reference count to 0, OnShutdownComplete is called.
 */
struct ShutdownToken
{
    ShutdownToken(Delegate ** delegate) : mDelegate(delegate) {}
    ~ShutdownToken() { (*mDelegate)->OnShutdownComplete(); }

private:
    Delegate ** mDelegate;
};

/**
 *  @struct Joinable
 *
 *  @brief
 *    Joinable interface for discoverers.  When a discoverer begins a discovery
 *    operation, a reference to the shutdown token is captured.  When discovery
 *    completes, the token reference is released.
 *
 *    If shutdown is called while discovery is in progress, this begins
 *    discovery cancellation, which may be long-running.  When cancellation
 *    is complete, the token reference is likewise released.
 */
struct Joinable
{
    Joinable(Delegate ** delegate) : mDelegate(delegate) {}

protected:
    /**
     *  @brief
     *   Determine whether the Joinable is in progress, as indicated by
     *   posession of a ShutdownToken reference.
     *
     * @return true if in-progress, else false
     */
    bool InProgress() { return mShutdownToken.get() != nullptr ? true : false; }

    /**
     *  @brief
     *     Capture a reference to the passed ShutdownToken.
     *
     * @param token shutdown token to capture
     */
    void RetainShutdownToken(Platform::SharedPtr<ShutdownToken> token) { mShutdownToken = token; }

    /**
     *  @brief
     *     Release the reference to any ShutdownToken that is held.
     */
    void ReleaseShutdownToken() { mShutdownToken = Platform::SharedPtr<ShutdownToken>(nullptr); }

    /**
     *  @brief
     *    Acquire a copy of any held ShutdownToken.
     *
     * @return copy of the ShutdownToken if any is held, else an empty token
     */
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

    /**
     *  @brief
     *    Begin the potentially long-running shutdown sequence.   If a
     *    reference is held to the ShutdownToken, this will be released on
     *    completion of shutdown.
     */
    void Shutdown();

    /**
     *  @brief
     *    Start BLE discovery, filtering with the passed Onboarding Payload.
     *
     *  @param payload onboarding payload
     *  @param token a Shutdown Token to which a reference will be captured if Discovery begins successfully
     *
     *  @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     */
    CHIP_ERROR StartBleDiscovery(SetupPayload & payload, Platform::SharedPtr<ShutdownToken> token);

    /**
     *  @brief
     *    Get the next BLE candidate, if any was found, and place it in the
     *    passed commissionee.  When a BLE candidate is placed in the passed
     *    commissionee, the commissione takes ownership of the underlying
     *    connection object.
     *
     *  @param[in,out] commissione into which the next BLE candidate will be placed if any was found
     *
     *  @return Returns CHIP_NO_ERROR if a candidate was found, CHIP_ERROR_NOT_FOUND if not
     */
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

    /**
     *  @brief
     *    Begin the potentially long-running shutdown sequence.   If a
     *    reference is held to the ShutdownToken, this will be released on
     *    completion of shutdown.
     */
    void Shutdown();

    /**
     *  @brief
     *    Start DNS-SD discovery, filtering with the passed Onboarding Payload.
     *
     *  @param payload onboarding payload
     *  @param token a Shutdown Token to which a reference will be captured if Discovery begins successfully
     *
     *  @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     */
    CHIP_ERROR StartDnssdDiscovery(SetupPayload & payload, Platform::SharedPtr<ShutdownToken> token);

    /**
     *  @brief
     *    Get the next DNS-SD candidate, if any was found, and place it in the
     *    passed commissionee.  When a call successfully locates a candidate,
     *    this advances the internal pointers such that each call will provide
     *    a different candidate address.
     *
     *  @param[in,out] commissione into which the next DNS-SD candidate will be placed if any was found
     *
     *  @return Returns CHIP_NO_ERROR if a candidate was found, CHIP_ERROR_NOT_FOUND if not
     */
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
    Discoverer(Controller::DeviceControllerSystemState & systemState, Delegate * delegate);

    /**
     *  @brief
     *    Initialize the discoverer.
     *
     *  @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     */
    CHIP_ERROR Init();

    /**
     *  @brief
     *    Shutdown the discoverer.  Shutdown is a potentially long-running
     *    process.  The discoverer calls OnShutdownComplete on its delegate
     *    when this is complete.  Only when this is complete is it safe to
     *    call the discoverer's destructor.
     */
    void Shutdown();

    /**
     *  @brief
     *    Set the delegate for the discoverer.
     */
    void SetDelegate(Delegate * delegate);

    /**
     *  @brief
     *    Begin commissionable node discovery across the available networks,
     *    filtering with the passed Onboarding Payload.
     *
     *  @param payload onboarding payload
     *
     *  @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     */
    CHIP_ERROR Discover(SetupPayload & payload);

    /**
     *  @brief
     *    Get the next commissionable node candidate, if any was found, and
     *    place it in the passed commissionee.  When a call successfully
     *    locates a candidate, this advances the internal pointers such that
     *    each call will provide a different candidate or candidate address.
     *
     *  @param[in,out] commissione into which the next DNS-SD candidate will be placed if any was found
     *
     *  @return Returns CHIP_NO_ERROR if a candidate was found, CHIP_ERROR_NOT_FOUND if not
     */
    CHIP_ERROR GetNextCandidate(Commissionee & commissionee);

private:
    Delegate * mDelegate;
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

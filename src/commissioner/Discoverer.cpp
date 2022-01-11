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

#include <commissioner/Discoverer.h>

namespace chip {
namespace Commissioner {
namespace CommissionableNodeDiscoverer {

Discoverer::Discoverer(Controller::DeviceControllerSystemState & systemState, Delegate * delegate) :
    Joinable(&mDelegate), mDelegate(delegate)
#if CONFIG_NETWORK_LAYER_BLE
    ,
    mBleDiscoverer(systemState, &mDelegate)
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    ,
    mDnssdDiscoverer(systemState, &mDelegate)
#endif
{}

CHIP_ERROR Discoverer::Init()
{
    RetainShutdownToken(Platform::MakeShared<ShutdownToken>(&mDelegate));
    return InProgress() ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
}

void Discoverer::Shutdown()
{
#if CONFIG_NETWORK_LAYER_BLE
    mBleDiscoverer.Shutdown();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    mDnssdDiscoverer.Shutdown();
#endif
    ReleaseShutdownToken();
}

void Discoverer::SetDelegate(Delegate * delegate)
{
    mDelegate = delegate;
}

CHIP_ERROR Discoverer::Discover(SetupPayload & payload)
{
    CHIP_ERROR err = CHIP_ERROR_NOT_IMPLEMENTED;
#if CONFIG_NETWORK_LAYER_BLE
    bool searchAllOver = payload.rendezvousInformation == RendezvousInformationFlag::kNone;
    if (searchAllOver || payload.rendezvousInformation == RendezvousInformationFlag::kBLE)
    {
        SuccessOrExit(err = mBleDiscoverer.StartBleDiscovery(payload, GetShutdownToken()));
    }
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    // We always want to search on-network because any node that has
    // already been commissioned will use on-network regardless of
    // onboarding payload contents.
    SuccessOrExit(err = mDnssdDiscoverer.StartDnssdDiscovery(payload, GetShutdownToken()));
#endif
exit:
    return err;
}

CHIP_ERROR Discoverer::GetNextCandidate(Commissionee & commissionee)
{
#if CONFIG_NETWORK_LAYER_BLE
    VerifyOrReturnError(mBleDiscoverer.GetNextBleCandidate(commissionee) != CHIP_NO_ERROR, CHIP_NO_ERROR);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    VerifyOrReturnError(mDnssdDiscoverer.GetNextDnssdCandidate(commissionee) != CHIP_NO_ERROR, CHIP_NO_ERROR);
#endif
    // CHIP_ERROR_NOT_FOUND is a special signal to the caller that no
    // candidates were found, but discovery is still in progress.
    return InProgress() ? CHIP_ERROR_NOT_FOUND : CHIP_ERROR_INTERNAL;
}

} // namespace CommissionableNodeDiscoverer
} // namespace Commissioner
} // namespace chip

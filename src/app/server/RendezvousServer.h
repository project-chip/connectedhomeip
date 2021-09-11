/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app/server/AppDelegate.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/RendezvousParameters.h>
#include <protocols/secure_channel/SessionIDAllocator.h>

namespace chip {

class RendezvousServer : public SessionEstablishmentDelegate
{
public:
    CHIP_ERROR WaitForPairing(const RendezvousParameters & params, uint32_t pbkdf2IterCount, const ByteSpan & salt,
                              uint16_t passcodeID, Messaging::ExchangeManager * exchangeManager, TransportMgrBase * transportMgr,
                              SecureSessionMgr * sessionMgr);

    CHIP_ERROR Init(AppDelegate * delegate, SessionIDAllocator * idAllocator)
    {
        VerifyOrReturnError(idAllocator != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        mIDAllocator = idAllocator;

        // The caller may chose to not provide a delegate object. The RendezvousServer checks for null delegate before calling
        // its methods.
        mDelegate = delegate;
        return CHIP_NO_ERROR;
    }

    //////////// SessionEstablishmentDelegate Implementation ///////////////
    void OnSessionEstablishmentError(CHIP_ERROR error) override;
    void OnSessionEstablished() override;

    void Cleanup();

    void OnPlatformEvent(const DeviceLayer::ChipDeviceEvent * event);

private:
    AppDelegate * mDelegate;
    Messaging::ExchangeManager * mExchangeManager = nullptr;

    PASESession mPairingSession;
    SecureSessionMgr * mSessionMgr = nullptr;

    SessionIDAllocator * mIDAllocator = nullptr;

    RendezvousAdvertisementDelegate * mAdvDelegate;

    bool HasAdvertisementDelegate() const { return mAdvDelegate != nullptr; }
    RendezvousAdvertisementDelegate * GetAdvertisementDelegate() { return mAdvDelegate; }
};

} // namespace chip

/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/icd/server/ICDMonitoringTable.h>
#include <credentials/FabricTable.h>
#include <lib/address_resolve/AddressResolve.h>

#include <messaging/ExchangeMgr.h>

namespace chip {
namespace app {

/**
 * @brief ICD Check-In Sender is responsible for resolving the NodeId and sending the check-in message
 */
class ICDCheckInSender : public AddressResolve::NodeListener
{
public:
    ICDCheckInSender(Messaging::ExchangeManager * exchangeManager);
    ~ICDCheckInSender() = default;

    CHIP_ERROR RequestResolve(ICDMonitoringEntry & entry, FabricTable * fabricTable, uint32_t counter);

    // AddressResolve::NodeListener - notifications when dnssd finds a node IP address
    void OnNodeAddressResolved(const PeerId & peerId, const AddressResolve::ResolveResult & result) override;
    void OnNodeAddressResolutionFailed(const PeerId & peerId, CHIP_ERROR reason) override;

    bool mResolveInProgress = false;

private:
    static constexpr uint8_t kApplicationDataSize = 2; // ActiveModeThreshold is 2 bytes

    CHIP_ERROR SendCheckInMsg(const Transport::PeerAddress & addr);

    // This is used when a node address is required.
    AddressResolve::NodeLookupHandle mAddressLookupHandle;

    Messaging::ExchangeManager * mExchangeManager = nullptr;

    Crypto::Aes128KeyHandle mAes128KeyHandle   = Crypto::Aes128KeyHandle();
    Crypto::Hmac128KeyHandle mHmac128KeyHandle = Crypto::Hmac128KeyHandle();

    uint32_t mICDCounter = 0;
};

} // namespace app
} // namespace chip

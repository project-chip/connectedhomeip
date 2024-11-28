/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <lib/support/Pool.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <transport/raw/MessageHeader.h>

#include <controller/python/chip/bdx/bdx-transfer.h>

namespace chip {
namespace bdx {

// This class handles unsolicited BDX messages. It keeps track of the number of expect transfers, and will only allocate
// BdxTransfer objects if a transfer is expected.
//
// The controller must inform this manager when a transfer is expected:
//   bdxTransferServer->ExpectATransfer();
// At which point the next unsolicited BDX init message will allocate a BdxTransfer object.
class TestBdxTransferServer : public Messaging::UnsolicitedMessageHandler
{
public:
    TestBdxTransferServer(BdxTransfer::Delegate * bdxTransferDelegate);
    ~TestBdxTransferServer() override;

    CHIP_ERROR Init(System::Layer * systemLayer, Messaging::ExchangeManager * exchangeManager);
    void Shutdown();

    // These keep track of the number of expected transfers. A transfer must be expected before this will allocate a
    // BdxTransfer object.
    void ExpectATransfer();
    void StopExpectingATransfer();

    void Release(BdxTransfer * bdxTransfer);

    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, Messaging::ExchangeDelegate *& delegate) override;
    void OnExchangeCreationFailed(Messaging::ExchangeDelegate * delegate) override;

private:
    // The maximum number of transfers to support at once. This number was chosen because it should be sufficient for
    // current tests that use BDX.
    static constexpr size_t kTransferPoolSize = 2;

    ObjectPool<BdxTransfer, kTransferPoolSize> mTransferPool;
    System::Layer * mSystemLayer                  = nullptr;
    Messaging::ExchangeManager * mExchangeManager = nullptr;
    BdxTransfer::Delegate * mBdxTransferDelegate  = nullptr;
    size_t mExpectedTransfers                     = 0;
};

} // namespace bdx
} // namespace chip

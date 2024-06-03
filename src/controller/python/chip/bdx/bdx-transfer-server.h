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

#include <messaging/ExchangeDelegate.h>
#include <transport/raw/MessageHeader.h>

#include <controller/python/chip/bdx/bdx-transfer-pool.h>

namespace chip {
namespace bdx {

// This class handles incoming BDX init messages to start BDX transfers. It allocates BdxTransfer objects from a pool.
class BdxTransferServer : public Messaging::UnsolicitedMessageHandler
{
public:
    BdxTransferServer(BdxTransferPool * bdxTransferPool);

    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader& payloadHeader, Messaging::ExchangeDelegate *& delegate) override;
    void OnExchangeCreationFailed(Messaging::ExchangeDelegate * delegate) override;

private:
    BdxTransferPool * mBdxTransferPool;
};

} // namespace bdx
} // namespace chip
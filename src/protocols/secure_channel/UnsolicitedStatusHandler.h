/**
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMgr.h>

namespace chip {
namespace Protocols {
namespace SecureChannel {

class UnsolicitedStatusHandler : public Messaging::ExchangeDelegate, public Messaging::UnsolicitedMessageHandler
{
public:
    CHIP_ERROR Init(Messaging::ExchangeManager * exchangeManager);

protected:
    // ExchangeDelegate
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;

    // UnsolicitedMessageHandler
    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override;
};

} // namespace SecureChannel
} // namespace Protocols
} // namespace chip

/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/dnssd/minimal_mdns/records/Txt.h>
#include <lib/dnssd/minimal_mdns/responders/RecordResponder.h>

namespace mdns {
namespace Minimal {

class TxtResponder : public RecordResponder
{
public:
    TxtResponder(const TxtResourceRecord & record) : RecordResponder(QType::TXT, record.GetName()), mRecord(record) {}

    void AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate,
                         const ResponseConfiguration & configuration) override
    {
        if (!delegate->ShouldSend(*this))
        {
            return;
        }

        TxtResourceRecord record = mRecord;
        configuration.Adjust(record);
        delegate->AddResponse(record);
        delegate->ResponsesAdded(*this);
    }

private:
    const TxtResourceRecord mRecord;
};

} // namespace Minimal
} // namespace mdns

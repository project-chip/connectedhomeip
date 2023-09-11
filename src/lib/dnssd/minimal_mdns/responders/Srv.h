/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/dnssd/minimal_mdns/records/Srv.h>
#include <lib/dnssd/minimal_mdns/responders/RecordResponder.h>

namespace mdns {
namespace Minimal {

class SrvResponder : public RecordResponder
{
public:
    SrvResponder(const SrvResourceRecord & record) : RecordResponder(QType::SRV, record.GetName()), mRecord(record) {}

    void AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate,
                         const ResponseConfiguration & configuration) override
    {
        if (!delegate->ShouldSend(*this))
        {
            return;
        }

        SrvResourceRecord record = mRecord;
        configuration.Adjust(record);
        delegate->AddResponse(record);
        delegate->ResponsesAdded(*this);
    }

private:
    const SrvResourceRecord mRecord;
};

} // namespace Minimal
} // namespace mdns

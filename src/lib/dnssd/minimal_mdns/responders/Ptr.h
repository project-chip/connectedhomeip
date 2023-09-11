/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/dnssd/minimal_mdns/records/Ptr.h>
#include <lib/dnssd/minimal_mdns/responders/RecordResponder.h>

namespace mdns {
namespace Minimal {

class PtrResponder : public RecordResponder
{
public:
    PtrResponder(const FullQName & qname, const FullQName & target) : RecordResponder(QType::PTR, qname), mTarget(target) {}

    void AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate,
                         const ResponseConfiguration & configuration) override
    {
        if (!delegate->ShouldSend(*this))
        {
            return;
        }

        PtrResourceRecord record(GetQName(), mTarget);
        configuration.Adjust(record);
        delegate->AddResponse(record);
        delegate->ResponsesAdded(*this);
    }

private:
    const FullQName mTarget;
};

} // namespace Minimal
} // namespace mdns

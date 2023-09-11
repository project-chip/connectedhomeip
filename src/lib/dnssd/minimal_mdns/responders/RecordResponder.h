/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/dnssd/minimal_mdns/responders/Responder.h>
namespace mdns {
namespace Minimal {

// This is a container class for the various record responders (PTR, SRV, TXT, A, AAAA etc.)
// This class is used to restrict the set of possible responders added to a QueryResponder.
class RecordResponder : public Responder
{
public:
    RecordResponder(QType qType, const FullQName & qName) : Responder(qType, qName) {}
};

} // namespace Minimal
} // namespace mdns

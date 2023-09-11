/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <lib/dnssd/minimal_mdns/core/Constants.h>
#include <lib/dnssd/minimal_mdns/core/QName.h>

namespace mdns {
namespace Minimal {

/// Filter used to determine what kind of a reply is acceptable
/// or not
class ReplyFilter
{
public:
    virtual ~ReplyFilter() {}

    /// Returns true if specified answer should be sent back as a reply
    virtual bool Accept(QType qType, QClass qClass, FullQName qname) = 0;
};

} // namespace Minimal
} // namespace mdns

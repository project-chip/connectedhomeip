/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <lib/dnssd/minimal_mdns/core/QNameString.h>

namespace mdns {
namespace Minimal {

QNameString::QNameString(const mdns::Minimal::FullQName & name)
{
    for (unsigned i = 0; i < name.nameCount; i++)
    {
        if (i != 0)
        {
            mBuffer.Add(".");
        }
        mBuffer.Add(name.names[i]);
    }
}

QNameString::QNameString(mdns::Minimal::SerializedQNameIterator name)
{
    bool first = true;
    while (name.Next())
    {
        if (first)
        {
            first = false;
        }
        else
        {
            mBuffer.Add(".");
        }
        mBuffer.Add(name.Value());
    }
    if (!name.IsValid())
    {
        mBuffer.Add("(!INVALID!)");
    }
}

} // namespace Minimal
} // namespace mdns

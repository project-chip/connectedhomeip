/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <lib/dnssd/minimal_mdns/core/QName.h>
#include <lib/support/StringBuilder.h>

namespace mdns {
namespace Minimal {

// Allows for a FullQName to be represented as a user-readable logging string
class QNameString
{
public:
    QNameString(const mdns::Minimal::FullQName & name);

    QNameString(mdns::Minimal::SerializedQNameIterator name);

    inline const char * c_str() const { return mBuffer.c_str(); }

    inline bool Fit() const { return mBuffer.Fit(); }

private:
    static constexpr size_t kMaxQNameLength = 128;
    chip::StringBuilder<kMaxQNameLength> mBuffer;
};

} // namespace Minimal
} // namespace mdns

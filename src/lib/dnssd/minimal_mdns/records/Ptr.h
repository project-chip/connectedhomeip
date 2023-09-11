/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/dnssd/minimal_mdns/records/ResourceRecord.h>

namespace mdns {
namespace Minimal {

class PtrResourceRecord : public ResourceRecord
{
public:
    PtrResourceRecord(const FullQName & qName, const FullQName & ptrName) : ResourceRecord(QType::PTR, qName), mPtrName(ptrName) {}

    const FullQName & GetPtr() const { return mPtrName; }

protected:
    bool WriteData(RecordWriter & out) const override { return out.WriteQName(mPtrName).Fit(); }

private:
    const FullQName mPtrName;
};

} // namespace Minimal
} // namespace mdns

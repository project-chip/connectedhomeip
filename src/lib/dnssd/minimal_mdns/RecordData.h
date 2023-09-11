/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inet/IPAddress.h>

#include <lib/dnssd/minimal_mdns/core/BytesRange.h>
#include <lib/dnssd/minimal_mdns/core/QName.h>

namespace mdns {
namespace Minimal {

class TxtRecordDelegate
{
public:
    virtual ~TxtRecordDelegate() {}

    /// Reports that a record was found
    ///
    /// name is mandatory
    /// value will be empty if '=' is missing in the TXT record. I.e code does
    /// *NOT* differentiate between "foo=" and "foo" as record.
    virtual void OnRecord(const BytesRange & name, const BytesRange & value) = 0;
};

/// Parses TXT record data
/// https://tools.ietf.org/html/rfc1035 (included in base RFC)
bool ParseTxtRecord(const BytesRange & data, TxtRecordDelegate * callback);

/// Parses SRV record data
/// https://tools.ietf.org/html/rfc2782
class SrvRecord
{
public:
    SrvRecord() {}

    /// Parses a SRV record within [data]
    ///
    /// [packet] specifies the range of valid data for PTR addresses within
    /// the name
    bool Parse(const BytesRange & data, const BytesRange & packet);

    uint16_t GetPriority() const { return mPriority; }
    uint16_t GetWeight() const { return mWeight; }
    uint16_t GetPort() const { return mPort; }
    SerializedQNameIterator GetName() const { return mName; }

private:
    uint16_t mPriority = 0;
    uint16_t mWeight   = 0;
    uint16_t mPort     = 0;
    SerializedQNameIterator mName;
};

/// Parses an IPV4 (A) record data
/// https://tools.ietf.org/html/rfc1035 (included in base RFC)
bool ParseARecord(const BytesRange & data, chip::Inet::IPAddress * addr);

/// Parses an IPV6 (AAAA) record data
/// https://tools.ietf.org/html/rfc3596
bool ParseAAAARecord(const BytesRange & data, chip::Inet::IPAddress * addr);

/// Parses a PTR record data
/// https://tools.ietf.org/html/rfc1035 (included in base RFC)
bool ParsePtrRecord(const BytesRange & data, const BytesRange & packet, SerializedQNameIterator * name);

} // namespace Minimal

} // namespace mdns

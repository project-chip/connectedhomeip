/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#pragma once

#include <lib/dnssd/wire/records/ResourceRecord.h>

namespace chip {
namespace Dnssd {
namespace Uld {

/**
 * @brief EDNS(0) OPT record with an Update Lease option (RFC 6891 / RFC 9665).
 *
 * CLASS holds the UDP payload size. TTL holds EDNS flags (DNSSEC OK).
 * RDATA is option_code=2, length=8, lease, key_lease.
 */
class OptLeaseRecord : public ResourceRecord
{
public:
    // EDNS(0) Update Lease option code (RFC 9665/9664).
    static constexpr uint16_t kEdnsOptionUpdateLease = 2;

    // EDNS(0) TTL default value.
    static constexpr uint32_t kEdnsTtlDefault = 0x00000000;

    OptLeaseRecord(uint16_t udpPayloadSize, uint32_t leaseSeconds, uint32_t keyLeaseSeconds) :
        ResourceRecord(QType::OPT, FullQName()), mLeaseSeconds(leaseSeconds), mKeyLeaseSeconds(keyLeaseSeconds)
    {
        SetClass(static_cast<QClass>(udpPayloadSize));
        SetTtl(kEdnsTtlDefault);
    }

    uint32_t GetLeaseSeconds() const { return mLeaseSeconds; }
    uint32_t GetKeyLeaseSeconds() const { return mKeyLeaseSeconds; }

protected:
    bool WriteData(RecordWriter & out) const override
    {
        return out.Put16(kEdnsOptionUpdateLease).Put16(8).Put32(mLeaseSeconds).Put32(mKeyLeaseSeconds).Fit();
    }

private:
    uint32_t mLeaseSeconds;
    uint32_t mKeyLeaseSeconds;
};

} // namespace Uld
} // namespace Dnssd
} // namespace chip

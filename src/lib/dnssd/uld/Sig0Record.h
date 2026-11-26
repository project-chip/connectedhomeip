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

#include <lib/dnssd/uld/Constants.h>
#include <lib/dnssd/wire/records/ResourceRecord.h>
#include <lib/support/Span.h>

namespace chip {
namespace Dnssd {
namespace Uld {

/**
 * @brief SIG(0) resource record wire encoding (RFC 2931).
 *
 * Owner name is root; CLASS=ANY; TTL=0.
 * RDATA: type_covered | algorithm | labels | original_ttl | expiration |
 * inception | key_tag | signer_name | signature(64).
 * Does not generate or verify the signature.
 */
class Sig0ResourceRecord : public ResourceRecord
{
public:
    Sig0ResourceRecord(const FullQName & signerName, ByteSpan signature, uint16_t keyTag = 0, uint32_t inception = 0,
                       uint32_t expiration = 0) :
        ResourceRecord(QType::SIG, FullQName()),
        mSignerName(signerName), mSignature(signature), mKeyTag(keyTag), mInception(inception), mExpiration(expiration)
    {
        SetClass(QClass::ANY);
        SetTtl(0);
    }

    FullQName GetSignerName() const { return mSignerName; }
    ByteSpan GetSignature() const { return mSignature; }

protected:
    bool WriteData(RecordWriter & out) const override
    {
        if (mSignature.size() != kP256RawSignatureSize)
        {
            return false;
        }

        out.Put16(0) // type covered
            .Put8(kKeyAlgorithmEcdsaP256)
            .Put8(0)  // labels
            .Put32(0) // original TTL
            .Put32(mExpiration)
            .Put32(mInception)
            .Put16(mKeyTag);

        // RFC 3597 §4: names embedded in SIG RDATA must not be compressed.
        out.WriteQNameUncompressed(mSignerName);
        out.Writer().Put(mSignature.data(), mSignature.size());
        return out.Fit();
    }

private:
    FullQName mSignerName;
    ByteSpan mSignature;
    uint16_t mKeyTag;
    uint32_t mInception;
    uint32_t mExpiration;
};

} // namespace Uld
} // namespace Dnssd
} // namespace chip

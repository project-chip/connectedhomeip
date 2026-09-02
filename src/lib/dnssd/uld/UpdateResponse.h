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

#include <cstdint>
#include <optional>

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/dnssd/wire/Parser.h>
#include <lib/support/Span.h>

namespace chip {
namespace Dnssd {
namespace Uld {

/**
 * Parses a DNS UPDATE response and exposes ULD-specific response data.
 */
class UpdateResponse : private ParserDelegate
{
public:
    /**
     * Parses a complete DNS UPDATE response.
     *
     * @return CHIP_NO_ERROR on success or CHIP_ERROR_INVALID_ARGUMENT if the
     *         packet or a recognized ULD record is malformed.
     */
    CHIP_ERROR Parse(ByteSpan packet);

    uint16_t GetMessageId() const { return mMessageId; }
    uint16_t GetResponseCode() const { return mResponseCode; }
    const std::optional<Crypto::P256PublicKey> & GetKey() const { return mKey; }

private:
    void OnHeader(ConstHeaderRef & header) override;
    void OnQuery(const QueryData &) override {}
    void OnResource(ResourceType type, const ResourceData & data) override;

    CHIP_ERROR mParseError = CHIP_NO_ERROR;
    uint16_t mMessageId    = 0;
    uint16_t mResponseCode = 0;
    bool mOptSeen          = false;
    std::optional<Crypto::P256PublicKey> mKey;
};

} // namespace Uld
} // namespace Dnssd
} // namespace chip

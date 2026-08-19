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
 * @brief Delete an RRset (RFC 2136 §2.5.2: CLASS=ANY, TTL=0, empty RDATA).
 *
 * When @p type is QType::ANY this delete all RRsets from a name.
 */
class DeleteRrsetRecord : public ResourceRecord
{
public:
    DeleteRrsetRecord(const FullQName & name, QType type) : ResourceRecord(type, name)
    {
        SetClass(QClass::ANY);
        SetTtl(0);
    }

protected:
    bool WriteData(RecordWriter & out) const override { return out.Fit(); }
};

} // namespace Uld
} // namespace Dnssd
} // namespace chip

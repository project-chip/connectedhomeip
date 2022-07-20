/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

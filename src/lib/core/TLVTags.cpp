/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    All rights reserved.
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
#include <lib/core/TLVTags.h>

#include <inttypes.h>

#include <lib/support/StringBuilder.h>
#include <lib/support/logging/TextOnlyLogging.h>

namespace chip {
namespace TLV {

/// Appends the text representation for a tag to the given string builder base.
StringBuilderBase & Tag::AppendTo(StringBuilderBase & out)
{
    if (TLV::IsProfileTag(*this))
    {
        out.AddFormat("ProfileTag(0x%X::0x%X::0x%" PRIX32 ")", TLV::VendorIdFromTag(*this), TLV::ProfileNumFromTag(*this),
                      TLV::TagNumFromTag(*this));
    }
    else if (TLV::IsContextTag(*this))
    {
        out.AddFormat("ContextTag(0x%" PRIX32 ")", TLV::TagNumFromTag(*this));
    }
    else if (*this == TLV::AnonymousTag())
    {
        out.Add("AnonymousTag()");
    }
    else
    {
        out.AddFormat("UnknownTag(0x" ChipLogFormatX64 ")", ChipLogValueX64(mVal));
    }

    return out;
}

} // namespace TLV
} // namespace chip

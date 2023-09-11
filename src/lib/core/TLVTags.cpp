/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "TLVTags.h"

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
        out.AddFormat("UnknownTag(0x%" PRIX64 ")", mVal);
    }

    return out;
}

} // namespace TLV
} // namespace chip

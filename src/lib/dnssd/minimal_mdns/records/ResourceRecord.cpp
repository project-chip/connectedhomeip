/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ResourceRecord.h"

namespace mdns {
namespace Minimal {

bool ResourceRecord::Append(HeaderRef & hdr, ResourceType asType, RecordWriter & out) const
{
    // order is important based on resource type. First come answers, then authorityAnswers
    // and then additional:
    if ((asType == ResourceType::kAuthority) && (hdr.GetAdditionalCount() != 0))
    {
        return false;
    }
    if ((asType == ResourceType::kAnswer) && ((hdr.GetAdditionalCount() != 0) || (hdr.GetAuthorityCount() != 0)))
    {
        return false;
    }

    out.WriteQName(mQName);

    out.Writer()                                  //
        .Put16(static_cast<uint16_t>(GetType()))  //
        .Put16(static_cast<uint16_t>(GetClass())) //
        .Put32(static_cast<uint32_t>(GetTtl()))   //
        ;

    chip::Encoding::BigEndian::BufferWriter sizeOutput(out.Writer()); // copy to re-output size
    out.Put16(0);                                                     // dummy, will be replaced later

    if (!WriteData(out))
    {
        return false;
    }
    sizeOutput.Put16(static_cast<uint16_t>(out.Writer().Needed() - sizeOutput.Needed() - 2));

    // This MUST be final and separated out: record count is only updated on success.
    if (out.Fit())
    {
        switch (asType)
        {
        case ResourceType::kAdditional:
            hdr.SetAdditionalCount(static_cast<uint16_t>(hdr.GetAdditionalCount() + 1));
            break;
        case ResourceType::kAuthority:
            hdr.SetAuthorityCount(static_cast<uint16_t>(hdr.GetAuthorityCount() + 1));
            break;
        case ResourceType::kAnswer:
            hdr.SetAnswerCount(static_cast<uint16_t>(hdr.GetAnswerCount() + 1));
            break;
        }
    }

    return out.Fit();
}

} // namespace Minimal
} // namespace mdns

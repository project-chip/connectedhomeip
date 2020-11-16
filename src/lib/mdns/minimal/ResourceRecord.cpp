/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "ResourceRecord.h"

namespace mdns {
namespace Minimal {

bool ResourceRecord::Append(HeaderRef & hdr, ResourceType asType, chip::BufBound & out) const
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

    // Write all QName parts
    for (uint16_t i = 0; i < mQNameCount; i++)
    {

        out.Put8(strlen(mQName[i]));
        out.Put(mQName[i]);
    }
    out.Put8(0); // end of qnames

    out.PutBE16(static_cast<uint16_t>(GetClass()));
    out.PutBE16(static_cast<uint16_t>(GetType()));
    out.PutBE32(static_cast<uint32_t>(GetTtl()));

    chip::BufBound sizeOutput(out);        /// copy to re-output size
    out.PutBE32(static_cast<uint32_t>(0)); // dummy, will be replaced later

    if (!WriteData(out))
    {
        return false;
    }
    sizeOutput.PutBE16(static_cast<uint16_t>(out.Written() - sizeOutput.Written() - 4));

    if (out.Fit())
    {
        switch (asType)
        {
        case ResourceType::kAdditional:
            hdr.SetAdditionalCount(hdr.GetAdditionalCount() + 1);
            break;
        case ResourceType::kAuthority:
            hdr.SetAuthorityCount(hdr.GetAuthorityCount() + 1);
            break;
        case ResourceType::kAnswer:
            hdr.SetAnswerCount(hdr.GetAnswerCount() + 1);
            break;
        }
    }

    return out.Fit();
}

} // namespace Minimal
} // namespace mdns

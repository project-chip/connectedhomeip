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

#include "Parser.h"

namespace mdns {
namespace Minimal {

bool ParsePacket(const uint8_t * packet, size_t length, ParserDelegate * delegate)
{
    if (length < HeaderRef::kSizeBytes)
    {
        return false;
    }

    // header is used as const, so cast is safe
    HeaderRef header(const_cast<uint8_t *>(packet));

    if (!header.GetFlags().IsValidMdns())
    {
        return false;
    }

    delegate->Header(header);

    // FIXME: implement the rest
    /*
    mdns::Minimal::HeaderRef hdr(buffer->Start());

    buffer->SetStart(buffer->Start() + mdns::Minimal::HeaderRef::kSizeBytes);

    if (hdr.GetQueryCount() > 0)
    {
        mdns::Minimal::SerializedQNameIterator it(buffer->Start(), buffer->Start() + buffer->DataLength(), buffer->Start());

        printf("      QUERY QNAME: ");
        while (it.Next())
        {
            printf("%s.", it.Value());
        }
        if (!it.ValidData())
        {
            printf("   (INVALID!)");
        }
        printf("\n");
    }
    else if (hdr.GetAnswerCount() > 0)
    {
        mdns::Minimal::SerializedQNameIterator it(buffer->Start(), buffer->Start() + buffer->DataLength(), buffer->Start());

        printf("      ANSWER QNAME: ");
        while (it.Next())
        {
            printf("%s.", it.Value());
        }
        if (!it.ValidData())
        {
            printf("   (INVALID!)");
        }
        printf("\n");
    }
    */

    return true;
}

} // namespace Minimal
} // namespace mdns

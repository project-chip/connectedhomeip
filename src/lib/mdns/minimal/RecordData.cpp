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

#include "RecordData.h"

namespace mdns {
namespace Minimal {

bool ParseTxtRecord(const BytesRange & data, TxtRecordDelegate * callback)
{
    const uint8_t * pos = data.Start();

    while (data.Contains(pos))
    {
        uint8_t length = *pos;

        if (!data.Contains(pos + length))
        {
            return false;
        }
        pos++;

        // name=value string of size length
        const uint8_t * equalPos = pos;
        while ((*equalPos != '=') && ((equalPos - pos) < length))
        {
            equalPos++;
        }

        if (pos + length == equalPos)
        {
            callback->OnRecord(BytesRange(pos, equalPos), BytesRange());
        }
        else
        {
            callback->OnRecord(BytesRange(pos, equalPos), BytesRange(equalPos + 1, pos + length));
        }

        pos += length;
    }

    return pos == data.End();
}

} // namespace Minimal

} // namespace mdns

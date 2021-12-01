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
#include "RecordWriter.h"

namespace mdns {
namespace Minimal {

SerializedQNameIterator RecordWriter::PreviousName(size_t index)
{
    if (index >= kMaxCachedReferences)
    {
        return SerializedQNameIterator();
    }

    uint16_t offset = mPreviousQNames[index];
    if (offset == kInvalidOffset)
    {
        return SerializedQNameIterator();
    }

    return SerializedQNameIterator(BytesRange(mOutput.Buffer(), mOutput.Buffer() + mOutput.WritePos()), mOutput.Buffer() + offset);
}

void RecordWriter::WriteQName(const FullQName & qname)
{
    // Brain dead q name writing
    // TODO: update
    for (uint16_t i = 0; i < qname.nameCount; i++)
    {

        mOutput.Put8(static_cast<uint8_t>(strlen(qname.names[i])));
        mOutput.Put(qname.names[i]);
    }
    mOutput.Put8(0); // end of qnames
}

} // namespace Minimal
} // namespace mdns

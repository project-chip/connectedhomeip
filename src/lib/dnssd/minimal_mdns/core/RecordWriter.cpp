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

SerializedQNameIterator RecordWriter::PreviousName(size_t index) const
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

    return SerializedQNameIterator(BytesRange(mOutput->Buffer(), mOutput->Buffer() + mOutput->WritePos()),
                                   mOutput->Buffer() + offset);
}

RecordWriter & RecordWriter::WriteQName(const FullQName & qname)
{
    size_t qNameWriteStart = mOutput->WritePos();
    bool isFullyCompressed = true;

    for (size_t i = 0; i < qname.nameCount; i++)
    {

        // find out if the record part remaining already is located somewhere
        FullQName remaining;
        remaining.names     = qname.names + i;
        remaining.nameCount = qname.nameCount - i;

        // Try to find a valid offset
        std::optional<uint16_t> offset = FindPreviousName(remaining);

        if (offset.has_value())
        {
            // Pointer to offset: set the highest 2 bits
            mOutput->Put16(*offset | 0xC000);

            if (mOutput->Fit() && !isFullyCompressed)
            {
                RememberWrittenQnameOffset(qNameWriteStart);
            }
            return *this;
        }

        mOutput->Put8(static_cast<uint8_t>(strlen(qname.names[i])));
        mOutput->Put(qname.names[i]);
        isFullyCompressed = false;
    }
    mOutput->Put8(0); // end of qnames

    if (mOutput->Fit())
    {
        RememberWrittenQnameOffset(qNameWriteStart);
    }
    return *this;
}

RecordWriter & RecordWriter::WriteQName(const SerializedQNameIterator & qname)
{
    size_t qNameWriteStart = mOutput->WritePos();
    bool isFullyCompressed = true;

    SerializedQNameIterator copy = qname;
    while (true)
    {
        std::optional<uint16_t> offset = FindPreviousName(copy);

        if (offset.has_value())
        {
            // Pointer to offset: set the highest 2 bits
            // We guarantee that offsets saved are <= kMaxReuseOffset
            mOutput->Put16(*offset | 0xC000);

            if (mOutput->Fit() && !isFullyCompressed)
            {
                RememberWrittenQnameOffset(qNameWriteStart);
            }
            return *this;
        }

        if (!copy.Next())
        {
            break;
        }

        if (!copy.IsValid())
        {
            break;
        }

        mOutput->Put8(static_cast<uint8_t>(strlen(copy.Value())));
        mOutput->Put(copy.Value());
        isFullyCompressed = false;
    }
    mOutput->Put8(0); // end of qnames

    if (mOutput->Fit())
    {
        RememberWrittenQnameOffset(qNameWriteStart);
    }
    return *this;
}

void RecordWriter::RememberWrittenQnameOffset(size_t offset)
{
    if (offset > kMaxReuseOffset)
    {
        // cannot represent this offset properly
        return;
    }

    for (unsigned short & previousName : mPreviousQNames)
    {
        if (previousName == kInvalidOffset)
        {
            previousName = static_cast<unsigned short>(offset);
            return;
        }
    }
}

} // namespace Minimal
} // namespace mdns

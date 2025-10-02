/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <tracing/esp32_diagnostics/DiagnosticStorage.h>
using namespace chip::TLV;

namespace chip {
namespace Tracing {
namespace Diagnostics {

CHIP_ERROR CircularDiagnosticBuffer::Store(const DiagnosticEntry & entry)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    mWriter.Init(*this);
    ReturnLogErrorOnFailure(Encode(mWriter, entry));
    return err;
}

CHIP_ERROR CircularDiagnosticBuffer::Retrieve(MutableByteSpan & span, uint32_t & read_entries)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    mReader.Init(*this);

    TLVWriter writer;
    writer.Init(span.data(), span.size());
    read_entries = 0;

    uint32_t successful_written_bytes = 0; // Store temporary writer length in case last TLV is not copied successfully.

    while ((err = mReader.Next()) == CHIP_NO_ERROR)
    {
        if (mReader.GetType() == kTLVType_Structure && mReader.GetTag() == AnonymousTag())
        {
            err = writer.CopyElement(mReader);
            if (err == CHIP_NO_ERROR)
            {
                successful_written_bytes = writer.GetLengthWritten();
                read_entries++;
            }
            else if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
            {
                // If we ran out of space, this is expected - not an error
                ChipLogProgress(DeviceLayer, "Buffer full after %" PRIu32 " entries", read_entries);
                err = CHIP_NO_ERROR;
                break;
            }
            else
            {
                // error occurred during copy
                ChipLogError(DeviceLayer, "Error copying TLV element: %" CHIP_ERROR_FORMAT, err.Format());
                break;
            }
        }
        else
        {
            ChipLogDetail(DeviceLayer, "Skipping TLV element. Found unexpected type: %d", static_cast<int>(mReader.GetType()));
        }
    }

    // Only finalize if we have at least one successful entry
    if (read_entries > 0)
    {
        ReturnErrorOnFailure(writer.Finalize());
    }
    else if (err != CHIP_END_OF_TLV && err != CHIP_NO_ERROR)
    {
        return err;
    }

    span.reduce_size(successful_written_bytes);
    ChipLogProgress(DeviceLayer, "Total Retrieved bytes: %" PRIu32 " (%" PRIu32 " entries)", successful_written_bytes,
                    read_entries);

    // CHIP_END_OF_TLV is expected when we reach the end of the buffer
    return CHIP_NO_ERROR;
}

bool CircularDiagnosticBuffer::IsBufferEmpty()
{
    return DataLength() == 0;
}

uint32_t CircularDiagnosticBuffer::GetDataSize()
{
    return DataLength();
}

CHIP_ERROR CircularDiagnosticBuffer::ClearBuffer()
{
    while (!IsBufferEmpty())
    {
        ReturnErrorOnFailure(EvictHead());
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR CircularDiagnosticBuffer::ClearBuffer(uint32_t entries)
{
    while (entries--)
    {
        ReturnErrorOnFailure(EvictHead());
    }
    return CHIP_NO_ERROR;
}
} // namespace Diagnostics
} // namespace Tracing
} // namespace chip

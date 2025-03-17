/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <tracing/esp32_diagnostic_trace/DiagnosticStorage.h>
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

    bool close_success                = true; // To check if the last TLV is copied successfully.
    uint32_t successful_written_bytes = 0;    // Store temporary writer length in case last TLV is not copied successfully.

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
            else
            {
                close_success = false;
                break;
            }
        }
        else
        {
            ChipLogDetail(DeviceLayer, "Skipping unexpected TLV element");
        }
    }
    ReturnErrorOnFailure(writer.Finalize());
    if (close_success)
    {
        successful_written_bytes = writer.GetLengthWritten();
    }
    span.reduce_size(successful_written_bytes);
    ChipLogDetail(DeviceLayer, "---------------Total Retrieved bytes : %ld----------------\n", successful_written_bytes);
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

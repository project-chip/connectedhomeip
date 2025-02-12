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

#pragma once

#include <tracing/esp32_diagnostic_trace/Diagnostics.h>

namespace chip {
namespace Tracing {
namespace Diagnostics {
class CircularDiagnosticBuffer : public chip::TLV::TLVCircularBuffer, public DiagnosticStorageInterface
{
public:
    CircularDiagnosticBuffer(uint8_t * buffer, size_t bufferLength) : chip::TLV::TLVCircularBuffer(buffer, bufferLength) {}

    CHIP_ERROR Store(const DiagnosticEntry & entry) override
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        mWriter.Init(*this);
        ReturnLogErrorOnFailure(entry.Encode(mWriter));
        return err;
    }

    CHIP_ERROR Retrieve(MutableByteSpan & span, uint32_t & read_entries) override
    {
        mReader.Init(*this);

        chip::TLV::TLVWriter writer;
        writer.Init(span.data(), span.size());
        read_entries = 0;

        bool close_success                = true; // To check if the last TLV is copied successfully.
        uint32_t successful_written_bytes = 0;    // Store temporary writer length in case last TLV is not copied successfully.

        while (CHIP_NO_ERROR == mReader.Next())
        {
            if (mReader.GetType() == chip::TLV::kTLVType_Structure && mReader.GetTag() == chip::TLV::AnonymousTag())
            {
                CHIP_ERROR err = writer.CopyElement(mReader);
                if (err == CHIP_NO_ERROR)
                {
                    successful_written_bytes = writer.GetLengthWritten();
                    read_entries++;
                }
                else
                {
                    close_success = false;
                    ChipLogError(DeviceLayer, "Failed to copy TLV element: %s", ErrorStr(err));
                    break;
                }
            }
            else
            {
                ChipLogError(DeviceLayer, "Skipping unexpected TLV element");
            }
        }
        ReturnErrorOnFailure(writer.Finalize());
        if (close_success)
        {
            successful_written_bytes = writer.GetLengthWritten();
        }
        span.reduce_size(successful_written_bytes);
        ChipLogProgress(DeviceLayer, "---------------Total Retrieved bytes : %ld----------------\n", successful_written_bytes);
        return CHIP_NO_ERROR;
    }

    bool IsBufferEmpty() override { return DataLength() == 0; }

    uint32_t GetDataSize() override { return DataLength(); }

    CHIP_ERROR ClearReadMemory(uint32_t entries)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        while (entries--)
        {
            err = EvictHead();
            if (err != CHIP_NO_ERROR)
            {
                break;
            }
        }
        return err;
    }

private:
    chip::TLV::CircularTLVReader mReader;
    chip::TLV::CircularTLVWriter mWriter;
};

} // namespace Diagnostics
} // namespace Tracing
} // namespace chip

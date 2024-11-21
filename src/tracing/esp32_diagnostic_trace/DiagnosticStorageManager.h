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

#define TLV_CLOSING_BYTE 1

namespace chip {
namespace Tracing {
namespace Diagnostics {
class CircularDiagnosticBuffer : public chip::TLV::TLVCircularBuffer, public DiagnosticStorageInterface
{
public:
    // Singleton instance getter
    static CircularDiagnosticBuffer & GetInstance()
    {
        static CircularDiagnosticBuffer instance;
        return instance;
    }

    // Delete copy constructor and assignment operator to ensure singleton
    CircularDiagnosticBuffer(const CircularDiagnosticBuffer &)             = delete;
    CircularDiagnosticBuffer & operator=(const CircularDiagnosticBuffer &) = delete;

    void Init(uint8_t * buffer, size_t bufferLength) { chip::TLV::TLVCircularBuffer::Init(buffer, bufferLength); }

    CHIP_ERROR Store(DiagnosticEntry & entry) override
    {
        chip::TLV::CircularTLVWriter writer;
        writer.Init(*this);

        CHIP_ERROR err = entry.Encode(writer);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Failed to write entry: %s", chip::ErrorStr(err));
        }
        return err;
    }

    CHIP_ERROR Retrieve(MutableByteSpan & span) override
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        chip::TLV::TLVReader reader;
        reader.Init(*this);

        chip::TLV::TLVWriter writer;
        writer.Init(span.data(), span.size());

        chip::TLV::TLVType outWriterContainer = chip::TLV::kTLVType_NotSpecified;
        ReturnErrorOnFailure(writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_List, outWriterContainer));

        while (CHIP_NO_ERROR == reader.Next())
        {
            VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                                ChipLogError(DeviceLayer, "Failed to read next TLV element: %s", ErrorStr(err)));

            if (reader.GetType() == chip::TLV::kTLVType_Structure && reader.GetTag() == chip::TLV::AnonymousTag())
            {
                if ((reader.GetLengthRead() - writer.GetLengthWritten()) < ((writer.GetRemainingFreeLength() + TLV_CLOSING_BYTE)))
                {
                    err = writer.CopyElement(reader);
                    if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
                    {
                        ChipLogProgress(DeviceLayer, "Buffer too small to occupy current element");
                        break;
                    }
                }
                else
                {
                    ChipLogProgress(DeviceLayer, "Buffer too small to occupy current TLV");
                    break;
                }
                VerifyOrReturnError(err == CHIP_NO_ERROR, err, ChipLogError(DeviceLayer, "Failed to copy TLV element"));
            }
            else
            {
                ChipLogError(DeviceLayer, "Unexpected TLV element type or tag in outer container");
            }
        }

        ReturnErrorOnFailure(writer.EndContainer(outWriterContainer));
        ReturnErrorOnFailure(writer.Finalize());
        span.reduce_size(writer.GetLengthWritten());
        ChipLogProgress(DeviceLayer, "---------------Total Retrieved bytes : %ld----------------\n", writer.GetLengthWritten());
        return CHIP_NO_ERROR;
    }

    bool IsEmptyBuffer() override { return DataLength() == 0; }

    uint32_t GetDataSize() override { return DataLength(); }

private:
    CircularDiagnosticBuffer() : chip::TLV::TLVCircularBuffer(nullptr, 0) {}
    ~CircularDiagnosticBuffer() override = default;
};

} // namespace Diagnostics
} // namespace Tracing
} // namespace chip

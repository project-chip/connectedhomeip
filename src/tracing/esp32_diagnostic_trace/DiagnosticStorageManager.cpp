
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

#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>
#include <tracing/esp32_diagnostic_trace/DiagnosticStorageManager.h>

namespace chip {
namespace Tracing {

namespace Diagnostics {
DiagnosticStorageImpl::DiagnosticStorageImpl(uint8_t * buffer, size_t bufferSize)
    : mEndUserCircularBuffer(buffer, bufferSize) {}

DiagnosticStorageImpl & DiagnosticStorageImpl::GetInstance(uint8_t * buffer, size_t bufferSize) {
    static DiagnosticStorageImpl instance(buffer, bufferSize);
    return instance;
}

DiagnosticStorageImpl::~DiagnosticStorageImpl() {}

CHIP_ERROR DiagnosticStorageImpl::Store(DiagnosticEntry & diagnostic)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    CircularTLVWriter writer;
    writer.Init(mEndUserCircularBuffer);

    // Start a TLV structure container (Anonymous)
    TLVType outerContainer;
    err = writer.StartContainer(AnonymousTag(), kTLVType_Structure, outerContainer);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        ChipLogError(DeviceLayer, "Failed to start TLV container for metric : %s", ErrorStr(err)));

    err = diagnostic.Encode(writer);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to encode diagnostic data : %s", ErrorStr(err));
        err = CHIP_ERROR_INVALID_ARGUMENT;
        writer.EndContainer(outerContainer);
        writer.Finalize();
        return err;
    }
    err = writer.EndContainer(outerContainer);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        ChipLogError(DeviceLayer, "Failed to end TLV container for metric : %s", ErrorStr(err)));

    err = writer.Finalize();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, ChipLogError(DeviceLayer, "Failed to finalize TLV writing"));
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticStorageImpl::Retrieve(MutableByteSpan & payload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CircularTLVReader reader;
    reader.Init(mEndUserCircularBuffer);

    TLVWriter writer;
    writer.Init(payload);

    TLVType outWriterContainer;
    err = writer.StartContainer(AnonymousTag(), kTLVType_List, outWriterContainer);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, ChipLogError(DeviceLayer, "Failed to start container"));

    while (true)
    {
        err = reader.Next();
        if (err == CHIP_ERROR_END_OF_TLV)
        {
            ChipLogProgress(DeviceLayer, "No more data to read");
            break;
        }
        VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                            ChipLogError(DeviceLayer, "Failed to read next TLV element: %s", ErrorStr(err)));

        if (reader.GetType() == kTLVType_Structure && reader.GetTag() == AnonymousTag())
        {
            TLVType outerReaderContainer;
            err = reader.EnterContainer(outerReaderContainer);
            VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                                ChipLogError(DeviceLayer, "Failed to enter outer TLV container: %s", ErrorStr(err)));

            err = reader.Next();
            VerifyOrReturnError(err == CHIP_NO_ERROR, err, ChipLogError(DeviceLayer, "Failed to read next TLV element in outer container: %s", ErrorStr(err)));

            // Check if the current element is a METRIC or TRACE container
            if ((reader.GetType() == kTLVType_Structure) &&
                (reader.GetTag() == ContextTag(DIAGNOSTICS_TAG::METRIC) || reader.GetTag() == ContextTag(DIAGNOSTICS_TAG::TRACE) || reader.GetTag() == ContextTag(DIAGNOSTICS_TAG::COUNTER)))
            {
                if ((reader.GetLengthRead() - writer.GetLengthWritten()) < writer.GetRemainingFreeLength()) {
                    err = writer.CopyElement(reader);
                    if (err == CHIP_ERROR_BUFFER_TOO_SMALL) {
                        ChipLogProgress(DeviceLayer, "Buffer too small to occupy current element");
                        break;
                    }
                    VerifyOrReturnError(err == CHIP_NO_ERROR, err, ChipLogError(DeviceLayer, "Failed to copy TLV element"));
                }
                else {
                    ChipLogProgress(DeviceLayer, "Buffer too small to occupy current TLV");
                    break;
                }
            }
            else
            {
                ChipLogError(DeviceLayer, "Unexpected TLV element in outer container");
                reader.ExitContainer(outerReaderContainer);
                return CHIP_ERROR_WRONG_TLV_TYPE;
            }
            // Exit the outer container
            err = reader.ExitContainer(outerReaderContainer);
            VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                                ChipLogError(DeviceLayer, "Failed to exit outer TLV container: %s", ErrorStr(err)));
        }
        else
        {
            ChipLogError(DeviceLayer, "Unexpected TLV element type or tag in outer container");
        }
    }

    err = writer.EndContainer(outWriterContainer);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, ChipLogError(DeviceLayer, "Failed to close outer container"));
    // Finalize the writing process
    err = writer.Finalize();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, ChipLogError(DeviceLayer, "Failed to finalize TLV writing"));
    payload.reduce_size(writer.GetLengthWritten());
    ChipLogProgress(DeviceLayer, "---------------Total written bytes successfully : %ld----------------\n", writer.GetLengthWritten());
    ChipLogProgress(DeviceLayer, "Retrieval successful");
    return CHIP_NO_ERROR;
}

bool DiagnosticStorageImpl::IsEmptyBuffer()
{
    return mEndUserCircularBuffer.DataLength() == 0;
}
} // namespace Diagnostics
} // namespace Tracing
} // namespace chip

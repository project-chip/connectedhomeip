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
#pragma once
#include <lib/core/TLVCircularBuffer.h>
#include <lib/support/Span.h>
#include <tracing/esp32_diagnostics/DiagnosticEntry.h>

namespace chip {
namespace Tracing {
namespace Diagnostics {

/**
 * @brief Diagnostic storage class
 */
class CircularDiagnosticBuffer : public chip::TLV::TLVCircularBuffer
{
public:
    CircularDiagnosticBuffer(uint8_t * buffer, uint32_t bufferLength) : chip::TLV::TLVCircularBuffer(buffer, bufferLength) {}

    /**
     * @brief Stores a diagnostic entry in the diagnostic storage buffer.
     * @param diagnostic  A reference to a `DiagnosticEntry` object that contains
     *                    the diagnostic data to be stored.
     * @return CHIP_ERROR  Returns `CHIP_NO_ERROR` if the data is successfully stored,
     *                     or an appropriate error code in case of failure.
     */
    CHIP_ERROR Store(const DiagnosticEntry & diagnostic);

    /**
     * @brief Copies diagnostic data from the storage buffer to a payload.
     *
     * This method retrieves the stored diagnostic data and copies it into the
     * provided `payload` buffer. If the buffer is too small to hold all the data,
     * still method returns the successfully copied entries along with an error code
     * indicating that the buffer was insufficient.
     *
     * @param payload       A reference to a `MutableByteSpan` where the retrieved
     *                      diagnostic data will be copied.
     * @param read_entries  A reference to an integer that will hold the total
     *                      number of successfully read diagnostic entries.
     *
     * @retval CHIP_NO_ERROR             If the operation succeeded and all data was copied.
     * @retval CHIP_ERROR                 If any other failure occurred during the operation.
     */
    CHIP_ERROR Retrieve(MutableByteSpan & payload, uint32_t & read_entries);

    /**
     * @brief Checks if the diagnostic storage buffer is empty.
     *
     * This method checks whether the buffer contains any stored diagnostic data.
     *
     * @return bool  Returns `true` if the buffer contains no stored data,
     *               or `false` if the buffer has data.
     */
    bool IsBufferEmpty();

    /**
     * @brief Retrieves the size of the data currently stored in the diagnostic buffer.
     *
     * This method returns the total size (in bytes) of all diagnostic data that is
     * currently stored in the buffer.
     *
     * @return uint32_t  The size (in bytes) of the stored diagnostic data.
     */
    uint32_t GetDataSize();

    /**
     * @brief Clears entire buffer
     */
    CHIP_ERROR ClearBuffer();

    /**
     * @brief Clears buffer up to the specified number of entries
     */
    CHIP_ERROR ClearBuffer(uint32_t entries);

private:
    chip::TLV::CircularTLVReader mReader;
    chip::TLV::CircularTLVWriter mWriter;
};

} // namespace Diagnostics
} // namespace Tracing
} // namespace chip

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

#include <lib/core/TLVCircularBuffer.h>

namespace chip {
namespace Tracing {

namespace Diagnostics {

// Diagnostic TAGs
enum class DiagTag : uint8_t
{
    TIMESTAMP = 0,
    LABEL,
    VALUE
};

/**
 * @class DiagnosticEntry
 * @brief Abstract base class for encoding diagnostic entries into TLV format.
 *
 */
class DiagnosticEntry
{
public:
    /**
     * @brief Virtual destructor for proper cleanup in derived classes.
     */
    virtual ~DiagnosticEntry() = default;

    /**
     * @brief Pure virtual method to encode diagnostic data into a TLV structure.
     *
     * @param writer A reference to the `chip::TLV::CircularTLVWriter` instance
     *               used to encode the TLV data.
     * @return CHIP_ERROR Returns an error code indicating the success or
     *                    failure of the encoding operation.
     */
    virtual CHIP_ERROR Encode(chip::TLV::CircularTLVWriter & writer) const = 0;
};

template <typename T>
class Diagnostic : public DiagnosticEntry
{
public:
    Diagnostic(const char * label, T value, uint32_t timestamp) : label_(label), value_(value), timestamp_(timestamp) {}

    CHIP_ERROR Encode(chip::TLV::CircularTLVWriter & writer) const override
    {
        chip::TLV::TLVType DiagnosticOuterContainer = chip::TLV::kTLVType_NotSpecified;
        ReturnErrorOnFailure(
            writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, DiagnosticOuterContainer));
        ReturnErrorOnFailure(writer.Put(chip::TLV::ContextTag(DiagTag::TIMESTAMP), timestamp_));
        ReturnErrorOnFailure(writer.PutString(chip::TLV::ContextTag(DiagTag::LABEL), label_));
        if constexpr (std::is_same_v<T, const char *>)
        {
            ReturnErrorOnFailure(writer.PutString(chip::TLV::ContextTag(DiagTag::VALUE), value_));
        }
        else
        {
            ReturnErrorOnFailure(writer.Put(chip::TLV::ContextTag(DiagTag::VALUE), value_));
        }
        ReturnErrorOnFailure(writer.EndContainer(DiagnosticOuterContainer));
        ReturnErrorOnFailure(writer.Finalize());
        ChipLogProgress(DeviceLayer, "Diagnostic Value written to storage successfully. label: %s\n", label_);
        return CHIP_NO_ERROR;
    }

private:
    const char * label_;
    T value_;
    uint32_t timestamp_;
};

/**
 * @brief Interface for storing and retrieving diagnostic data.
 */
class DiagnosticStorageInterface
{
public:
    /**
     * @brief Virtual destructor for the interface.
     */
    virtual ~DiagnosticStorageInterface() = default;

    /**
     * @brief Stores a diagnostic entry.
     * @param diagnostic  Reference to a DiagnosticEntry object containing the
     *                    diagnostic data to store.
     * @return CHIP_ERROR Returns CHIP_NO_ERROR on success, or an appropriate error code on failure.
     */
    virtual CHIP_ERROR Store(const DiagnosticEntry & diagnostic) = 0;

    /**
     * @brief Retrieves diagnostic data as a payload.
     * @param payload  Reference to a MutableByteSpan where the retrieved
     *                 diagnostic data will be stored.
     * @return CHIP_ERROR Returns CHIP_NO_ERROR on success, or an appropriate error code on failure.
     */
    virtual CHIP_ERROR Retrieve(MutableByteSpan & payload, uint32_t & read_entries) = 0;

    /**
     * @brief Checks if the storage buffer is empty.
     * @return bool true if the buffer contains no stored diagnostic data, otherwise false.
     */
    virtual bool IsBufferEmpty() = 0;

    /**
     * @brief Retrieves the size of the data currently stored in the buffer.
     * @return uint32_t The size (in bytes) of the stored diagnostic data.
     */
    virtual uint32_t GetDataSize() = 0;
};

} // namespace Diagnostics
} // namespace Tracing
} // namespace chip

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
#define kMaxStringValueSize 128

namespace chip {
namespace Tracing {

namespace Diagnostics {

// Diagnostic TAGs
enum class DiagTag : uint8_t
{
    TIMESTAMP = 0,
    LABEL,
    VALUE,
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

    virtual CHIP_ERROR Decode(chip::TLV::TLVReader & reader) = 0;
};

template <typename T>
class Diagnostic : public DiagnosticEntry
{
public:
    Diagnostic(char * label = nullptr, T value = T{}, uint32_t timestamp = 0) : label_(label), value_(value), timestamp_(timestamp)
    {}

    CHIP_ERROR Encode(chip::TLV::CircularTLVWriter & writer) const override
    {
        chip::TLV::TLVType DiagnosticOuterContainer = chip::TLV::kTLVType_NotSpecified;
        ReturnErrorOnFailure(
            writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, DiagnosticOuterContainer));

        // Write timestamp
        ReturnErrorOnFailure(writer.Put(chip::TLV::ContextTag(DiagTag::TIMESTAMP), timestamp_));

        // Write label
        if (strlen(label_) > kMaxStringValueSize)
        {
            char labelBuffer[kMaxStringValueSize + 1];
            memcpy(labelBuffer, label_, kMaxStringValueSize);
            labelBuffer[kMaxStringValueSize] = '\0';
            ReturnErrorOnFailure(writer.PutString(chip::TLV::ContextTag(DiagTag::LABEL), labelBuffer));
        }
        else
        {
            ReturnErrorOnFailure(writer.PutString(chip::TLV::ContextTag(DiagTag::LABEL), label_));
        }

        // Write value
        if constexpr (std::is_same_v<T, char *>)
        {
            if (strlen(value_) > kMaxStringValueSize)
            {
                char valueBuffer[kMaxStringValueSize + 1];
                memcpy(valueBuffer, value_, kMaxStringValueSize);
                valueBuffer[kMaxStringValueSize] = '\0';
                ReturnErrorOnFailure(writer.PutString(chip::TLV::ContextTag(DiagTag::VALUE), valueBuffer));
            }
            else
            {
                ReturnErrorOnFailure(writer.PutString(chip::TLV::ContextTag(DiagTag::VALUE), value_));
            }
        }
        else if constexpr (std::is_same_v<T, uint32_t>)
        {
            ReturnErrorOnFailure(writer.Put(chip::TLV::ContextTag(DiagTag::VALUE), value_));
        }
        else if constexpr (std::is_same_v<T, int32_t>)
        {
            ReturnErrorOnFailure(writer.Put(chip::TLV::ContextTag(DiagTag::VALUE), value_));
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        ReturnErrorOnFailure(writer.EndContainer(DiagnosticOuterContainer));
        ReturnErrorOnFailure(writer.Finalize());
        ChipLogProgress(DeviceLayer, "Diagnostic Value written to storage successfully. label: %s\n", label_);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Decode(chip::TLV::TLVReader & reader) override
    {
        TLV::TLVType containerType;
        ReturnErrorOnFailure(reader.EnterContainer(containerType));
        // Read timestamp
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(DiagTag::TIMESTAMP)));
        ReturnErrorOnFailure(reader.Get(timestamp_));

        // Read label
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(DiagTag::LABEL)));
        uint32_t labelSize = reader.GetLength();
        char labelBuffer[kMaxStringValueSize + 1];
        ReturnErrorOnFailure(reader.GetString(labelBuffer, kMaxStringValueSize + 1));
        memcpy(label_, labelBuffer, labelSize + 1);

        // Read value
        ReturnErrorOnFailure(reader.Next());
        if constexpr (std::is_same_v<T, char *>)
        {
            uint32_t valueSize = reader.GetLength();
            char valueBuffer[kMaxStringValueSize + 1];
            ReturnErrorOnFailure(reader.GetString(valueBuffer, kMaxStringValueSize + 1));
            memcpy(value_, valueBuffer, valueSize + 1);
        }
        else if constexpr (std::is_same_v<T, uint32_t>)
        {
            ReturnErrorOnFailure(reader.Get(value_));
        }
        else if constexpr (std::is_same_v<T, int32_t>)
        {
            ReturnErrorOnFailure(reader.Get(value_));
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        ReturnErrorOnFailure(reader.ExitContainer(containerType));
        return CHIP_NO_ERROR;
    }

    // Getters
    char * GetLabel() const { return label_; }
    T GetValue() const { return value_; }
    uint32_t GetTimestamp() const { return timestamp_; }

private:
    char * label_ = nullptr;
    T value_{};
    uint32_t timestamp_ = 0;
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
     * @brief Stores a diagnostic entry in the diagnostic storage buffer.
     * @param diagnostic  A reference to a `DiagnosticEntry` object that contains
     *                    the diagnostic data to be stored.
     * @return CHIP_ERROR  Returns `CHIP_NO_ERROR` if the data is successfully stored,
     *                     or an appropriate error code in case of failure.
     */
    virtual CHIP_ERROR Store(const DiagnosticEntry & diagnostic) = 0;

    /**
     * @brief Copies diagnostic data from the storage buffer to a payload.
     *
     * This method retrieves the stored diagnostic data and copies it into the
     * provided `payload` buffer. If the buffer is too small to hold all the data,
     * the method returns the successfully copied entries along with an error code
     * indicating that the buffer was insufficient.
     *
     * @param payload       A reference to a `MutableByteSpan` where the retrieved
     *                      diagnostic data will be copied.
     * @param read_entries  A reference to an integer that will hold the total
     *                      number of successfully read diagnostic entries.
     *
     * @retval CHIP_NO_ERROR             If the operation succeeded and all data was copied.
     * @retval CHIP_ERROR_BUFFER_TOO_SMALL If the buffer was not large enough to hold all data.
     * @retval CHIP_ERROR                 If any other failure occurred during the operation.
     */
    virtual CHIP_ERROR Retrieve(MutableByteSpan & payload, uint32_t & read_entries) = 0;

    /**
     * @brief Checks if the diagnostic storage buffer is empty.
     *
     * This method checks whether the buffer contains any stored diagnostic data.
     *
     * @return bool  Returns `true` if the buffer contains no stored data,
     *               or `false` if the buffer has data.
     */
    virtual bool IsBufferEmpty() = 0;

    /**
     * @brief Retrieves the size of the data currently stored in the diagnostic buffer.
     *
     * This method returns the total size (in bytes) of all diagnostic data that is
     * currently stored in the buffer.
     *
     * @return uint32_t  The size (in bytes) of the stored diagnostic data.
     */
    virtual uint32_t GetDataSize() = 0;

    /**
     * @brief Clears entire buffer
     */
    virtual CHIP_ERROR ClearBuffer() = 0;

    /**
     * @brief Clears buffer up to the specified number of entries
     */
    virtual CHIP_ERROR ClearBuffer(uint32_t entries) = 0;
};

} // namespace Diagnostics
} // namespace Tracing
} // namespace chip

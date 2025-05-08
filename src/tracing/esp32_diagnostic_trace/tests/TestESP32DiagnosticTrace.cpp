/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>
#include <tracing/esp32_diagnostic_trace/DiagnosticEntry.h>
#include <tracing/esp32_diagnostic_trace/DiagnosticStorage.h>

using namespace chip;
using namespace chip::Tracing;
using namespace chip::Tracing::Diagnostics;
using namespace chip::TLV;

namespace {

// Test for Encode and Decode in DiagnosticEntry
TEST(DiagnosticEntryTest, EncodeDecode)
{
    DiagnosticEntry original;
    strncpy(original.label, "TestLabel", kMaxStringValueSize);
    strncpy(original.stringValue, "TestValue", kMaxStringValueSize);
    original.type                     = ValueType::kCharString;
    original.timestamps_ms_since_boot = 12345;

    uint8_t buffer[256];
    TLVWriter writer;
    writer.Init(buffer, sizeof(buffer));

    CHIP_ERROR err = Encode(writer, original);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());

    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

    DiagnosticEntry decoded;
    err = Decode(reader, decoded);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_STREQ(decoded.label, original.label);
    EXPECT_STREQ(decoded.stringValue, original.stringValue);
    EXPECT_EQ(decoded.type, original.type);
    EXPECT_EQ(decoded.timestamps_ms_since_boot, original.timestamps_ms_since_boot);
}

// Test for Store and Retrieve in CircularDiagnosticBuffer
TEST(DiagnosticStorageTest, StoreRetrieve)
{
    uint8_t buffer[512];
    CircularDiagnosticBuffer circularBuffer(buffer, sizeof(buffer));

    DiagnosticEntry entry1;
    strncpy(entry1.label, "Entry1", kMaxStringValueSize);
    strncpy(entry1.stringValue, "Value1", kMaxStringValueSize);
    entry1.type                     = ValueType::kCharString;
    entry1.timestamps_ms_since_boot = 1000U;

    CHIP_ERROR err = circularBuffer.Store(entry1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    uint8_t retrieveBuffer[256];
    MutableByteSpan span(retrieveBuffer, sizeof(retrieveBuffer));
    uint32_t readEntries = 0;

    err = circularBuffer.Retrieve(span, readEntries);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(readEntries, 1U);
}

// Test for encoding, storing, retrieving, and decoding 5 entries
TEST(DiagnosticStorageTest, StoreRetrieveMultipleEntries)
{
    uint8_t buffer[1024];
    CircularDiagnosticBuffer circularBuffer(buffer, sizeof(buffer));

    DiagnosticEntry entries[5];
    strncpy(entries[0].label, "TestCharStringValue", kMaxStringValueSize);
    strncpy(entries[0].stringValue, "TestValue1", kMaxStringValueSize);
    entries[0].type                     = ValueType::kCharString;
    entries[0].timestamps_ms_since_boot = 1000U;

    strncpy(entries[1].label, "TestNegativeSignedIntegerValue", kMaxStringValueSize);
    entries[1].intValue                 = -123456;
    entries[1].type                     = ValueType::kSignedInteger;
    entries[1].timestamps_ms_since_boot = 2000U;

    strncpy(entries[2].label, "TestSignedIntegerValue", kMaxStringValueSize);
    entries[2].intValue                 = 123456;
    entries[2].type                     = ValueType::kSignedInteger;
    entries[2].timestamps_ms_since_boot = 3000U;

    strncpy(entries[3].label, "TestUnsignedIntegerValue", kMaxStringValueSize);
    entries[3].uintValue                = 100U;
    entries[3].type                     = ValueType::kUnsignedInteger;
    entries[3].timestamps_ms_since_boot = 4000U;

    strncpy(entries[4].label, "MaxLengthLabel", kMaxStringValueSize);
    char longString[kMaxStringValueSize + 1];
    memset(longString, 'A', kMaxStringValueSize - 1);
    longString[kMaxStringValueSize - 1] = '\0';
    strncpy(entries[4].stringValue, longString, kMaxStringValueSize);
    entries[4].type                     = ValueType::kCharString;
    entries[4].timestamps_ms_since_boot = 5000U;

    ASSERT_TRUE(circularBuffer.DataLength() == 0);

    for (int i = 0; i < 5; ++i)
    {
        CHIP_ERROR err = circularBuffer.Store(entries[i]);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    ASSERT_FALSE(circularBuffer.IsBufferEmpty());

    uint8_t retrieveBuffer[1024];
    MutableByteSpan span(retrieveBuffer, sizeof(retrieveBuffer));
    uint32_t readEntries = 0;

    CHIP_ERROR err = circularBuffer.Retrieve(span, readEntries);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(readEntries, 5U);

    TLVReader reader;
    reader.Init(retrieveBuffer, span.size());

    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

        DiagnosticEntry decoded;
        err = Decode(reader, decoded);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        EXPECT_STREQ(decoded.label, entries[i].label);
        EXPECT_EQ(decoded.type, entries[i].type);
        EXPECT_EQ(decoded.timestamps_ms_since_boot, entries[i].timestamps_ms_since_boot);

        switch (decoded.type)
        {
        case ValueType::kCharString: {
            size_t decodedLen  = strlen(decoded.stringValue);
            size_t originalLen = strlen(entries[i].stringValue);
            EXPECT_EQ(decodedLen, originalLen);
            EXPECT_LE(decodedLen, kMaxStringValueSize - 1);
            EXPECT_STREQ(decoded.stringValue, entries[i].stringValue);
            break;
        }
        case ValueType::kSignedInteger:
            EXPECT_EQ(decoded.intValue, entries[i].intValue);
            break;
        case ValueType::kUnsignedInteger:
            EXPECT_EQ(decoded.uintValue, entries[i].uintValue);
            break;
        default:
            ADD_FAILURE() << "Unexpected value type";
            break;
        }
    }
}

// Test for retrieving buffer underrun scenario
TEST(DiagnosticStorageTest, RetrieveBufferUnderrun)
{
    uint8_t buffer[256];
    CircularDiagnosticBuffer circularBuffer(buffer, sizeof(buffer));

    const uint32_t entryCount = 10U;
    DiagnosticEntry entries[entryCount];

    for (uint32_t i = 0U; i < entryCount; ++i)
    {
        snprintf(entries[i].label, kMaxStringValueSize, "Entry%" PRIu32, i);
        snprintf(entries[i].stringValue, kMaxStringValueSize, "Value%" PRIu32, i);
        entries[i].type                     = ValueType::kCharString;
        entries[i].timestamps_ms_since_boot = 1000U + i;

        CHIP_ERROR err = circularBuffer.Store(entries[i]);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    const uint32_t chunkCount = 2U;
    for (uint32_t chunk = 0U; chunk < chunkCount; ++chunk)
    {
        uint8_t retrieveBuffer[128];
        MutableByteSpan span(retrieveBuffer, sizeof(retrieveBuffer));
        uint32_t readEntries = 0U;

        CHIP_ERROR err = circularBuffer.Retrieve(span, readEntries);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_GE(readEntries, 0U);

        TLVReader reader;
        reader.Init(retrieveBuffer, span.size());

        for (uint32_t i = 0U; i < readEntries; ++i)
        {
            EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

            DiagnosticEntry decoded;
            err = Decode(reader, decoded);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            const uint32_t index = i + (chunk * 5U);

            ASSERT_LT(index, entryCount);

            EXPECT_STREQ(decoded.label, entries[index].label);
            EXPECT_STREQ(decoded.stringValue, entries[index].stringValue);
            EXPECT_EQ(decoded.type, entries[index].type);
            EXPECT_EQ(decoded.timestamps_ms_since_boot, entries[index].timestamps_ms_since_boot);
        }

        err = circularBuffer.ClearBuffer(readEntries);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }
}

} // namespace

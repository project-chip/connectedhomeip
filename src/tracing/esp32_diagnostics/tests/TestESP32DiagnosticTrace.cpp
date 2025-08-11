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
#include <tracing/esp32_diagnostics/DiagnosticEntry.h>
#include <tracing/esp32_diagnostics/DiagnosticStorage.h>
#include <tracing/esp32_diagnostics/DiagnosticTracing.h>

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

// Test for ESP32Diagnostics Filter Functionality
TEST(ESP32DiagnosticsTest, DefaultFiltersInitialization)
{
    uint8_t buffer[512];
    CircularDiagnosticBuffer storageBuffer(buffer, sizeof(buffer));
    ESP32Diagnostics diagnostics(&storageBuffer);

    // Test that default filters are enabled
    EXPECT_TRUE(diagnostics.IsEnabled("PASESession"));
    EXPECT_TRUE(diagnostics.IsEnabled("CASESession"));
    EXPECT_TRUE(diagnostics.IsEnabled("NetworkCommissioning"));
    EXPECT_TRUE(diagnostics.IsEnabled("GeneralCommissioning"));
    EXPECT_TRUE(diagnostics.IsEnabled("OperationalCredentials"));
    EXPECT_TRUE(diagnostics.IsEnabled("CASEServer"));
    EXPECT_TRUE(diagnostics.IsEnabled("Fabric"));

    // Test that non-default scopes are not enabled
    EXPECT_FALSE(diagnostics.IsEnabled("CustomScope"));
    EXPECT_FALSE(diagnostics.IsEnabled("AnotherScope"));
}

TEST(ESP32DiagnosticsTest, AddFilter)
{
    uint8_t buffer[512];
    CircularDiagnosticBuffer storageBuffer(buffer, sizeof(buffer));
    ESP32Diagnostics diagnostics(&storageBuffer);

    // Clear default filters for clean testing
    diagnostics.ClearFilters();

    // Initially all scopes should be enabled when no filters are set
    EXPECT_TRUE(diagnostics.IsEnabled("TestScope"));

    // Add a filter
    CHIP_ERROR err = diagnostics.AddFilter("TestScope");
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Now only the added scope should be enabled
    EXPECT_TRUE(diagnostics.IsEnabled("TestScope"));
    EXPECT_FALSE(diagnostics.IsEnabled("AnotherScope"));

    // Add another filter
    err = diagnostics.AddFilter("AnotherScope");
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Both scopes should now be enabled
    EXPECT_TRUE(diagnostics.IsEnabled("TestScope"));
    EXPECT_TRUE(diagnostics.IsEnabled("AnotherScope"));
    EXPECT_FALSE(diagnostics.IsEnabled("ThirdScope"));
}

TEST(ESP32DiagnosticsTest, AddFilterDuplicate)
{
    uint8_t buffer[512];
    CircularDiagnosticBuffer storageBuffer(buffer, sizeof(buffer));
    ESP32Diagnostics diagnostics(&storageBuffer);

    diagnostics.ClearFilters();

    // Add a filter
    CHIP_ERROR err = diagnostics.AddFilter("TestScope");
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Add the same filter again - should succeed without error
    err = diagnostics.AddFilter("TestScope");
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Scope should still be enabled
    EXPECT_TRUE(diagnostics.IsEnabled("TestScope"));
}

TEST(ESP32DiagnosticsTest, AddFilterInvalidArguments)
{
    uint8_t buffer[512];
    CircularDiagnosticBuffer storageBuffer(buffer, sizeof(buffer));
    ESP32Diagnostics diagnostics(&storageBuffer);

    // Test null scope
    CHIP_ERROR err = diagnostics.AddFilter(nullptr);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);

    // Test empty scope
    err = diagnostics.AddFilter("");
    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(ESP32DiagnosticsTest, RemoveFilter)
{
    uint8_t buffer[512];
    CircularDiagnosticBuffer storageBuffer(buffer, sizeof(buffer));
    ESP32Diagnostics diagnostics(&storageBuffer);

    diagnostics.ClearFilters();

    // Add some filters
    CHIP_ERROR err = diagnostics.AddFilter("TestScope1");
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = diagnostics.AddFilter("TestScope2");
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Verify both are enabled
    EXPECT_TRUE(diagnostics.IsEnabled("TestScope1"));
    EXPECT_TRUE(diagnostics.IsEnabled("TestScope2"));

    // Remove one filter
    err = diagnostics.RemoveFilter("TestScope1");
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Only the remaining scope should be enabled
    EXPECT_FALSE(diagnostics.IsEnabled("TestScope1"));
    EXPECT_TRUE(diagnostics.IsEnabled("TestScope2"));

    // Remove the last filter
    err = diagnostics.RemoveFilter("TestScope2");
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Now all scopes should be enabled (no filters set)
    EXPECT_TRUE(diagnostics.IsEnabled("TestScope1"));
    EXPECT_TRUE(diagnostics.IsEnabled("TestScope2"));
    EXPECT_TRUE(diagnostics.IsEnabled("AnyScope"));
}

TEST(ESP32DiagnosticsTest, RemoveFilterNegativeTest)
{
    uint8_t buffer[512];
    CircularDiagnosticBuffer storageBuffer(buffer, sizeof(buffer));
    ESP32Diagnostics diagnostics(&storageBuffer);

    // Test null scope
    CHIP_ERROR err = diagnostics.RemoveFilter(nullptr);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);

    // Test empty scope
    err = diagnostics.RemoveFilter("");
    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);

    // Test removing non-existent filter
    diagnostics.ClearFilters();
    err = diagnostics.AddFilter("ExistingScope");
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = diagnostics.RemoveFilter("NonExistentScope");
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
}

TEST(ESP32DiagnosticsTest, ClearFilters)
{
    uint8_t buffer[512];
    CircularDiagnosticBuffer storageBuffer(buffer, sizeof(buffer));
    ESP32Diagnostics diagnostics(&storageBuffer);

    // Add some filters
    CHIP_ERROR err = diagnostics.AddFilter("TestScope1");
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = diagnostics.AddFilter("TestScope2");
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Verify filters are working
    EXPECT_TRUE(diagnostics.IsEnabled("TestScope1"));
    EXPECT_TRUE(diagnostics.IsEnabled("TestScope2"));
    EXPECT_FALSE(diagnostics.IsEnabled("AnotherScope"));

    // Clear all filters
    diagnostics.ClearFilters();

    // Now all scopes should be enabled
    EXPECT_TRUE(diagnostics.IsEnabled("TestScope1"));
    EXPECT_TRUE(diagnostics.IsEnabled("TestScope2"));
    EXPECT_TRUE(diagnostics.IsEnabled("AnotherScope"));
    EXPECT_TRUE(diagnostics.IsEnabled("AnyScope"));
}

TEST(ESP32DiagnosticsTest, IsEnabledWithNoFilters)
{
    uint8_t buffer[512];
    CircularDiagnosticBuffer storageBuffer(buffer, sizeof(buffer));
    ESP32Diagnostics diagnostics(&storageBuffer);

    // Clear all filters
    diagnostics.ClearFilters();

    // When no filters are set, all scopes should be enabled
    EXPECT_TRUE(diagnostics.IsEnabled("AnyScope"));
    EXPECT_TRUE(diagnostics.IsEnabled("TestScope"));
    EXPECT_TRUE(diagnostics.IsEnabled("RandomScope"));
}

TEST(ESP32DiagnosticsTest, FilteringInTraceOperations)
{
    uint8_t buffer[1024];
    CircularDiagnosticBuffer storageBuffer(buffer, sizeof(buffer));
    ESP32Diagnostics diagnostics(&storageBuffer);

    // Clear filters and add specific ones
    diagnostics.ClearFilters();
    CHIP_ERROR err = diagnostics.AddFilter("EnabledScope");
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Check if filter was added correctly
    EXPECT_TRUE(diagnostics.IsEnabled("EnabledScope"));
    EXPECT_FALSE(diagnostics.IsEnabled("DisabledScope"));

    // Clear storage to ensure clean test
    storageBuffer.ClearBuffer();
    EXPECT_TRUE(storageBuffer.IsBufferEmpty());

    // Test TraceBegin with enabled scope
    diagnostics.TraceBegin("TestLabel1", "EnabledScope");

    // Test TraceBegin with disabled scope
    diagnostics.TraceBegin("TestLabel2", "DisabledScope");

    // Test TraceInstant with enabled scope
    diagnostics.TraceInstant("InstantLabel1", "EnabledScope");

    // Test TraceInstant with disabled scope
    diagnostics.TraceInstant("InstantLabel2", "DisabledScope");

    // Verify storage contains only enabled scope entries
    EXPECT_FALSE(storageBuffer.IsBufferEmpty());

    uint8_t retrieveBuffer[1024];
    MutableByteSpan span(retrieveBuffer, sizeof(retrieveBuffer));
    uint32_t readEntries = 0;

    err = storageBuffer.Retrieve(span, readEntries);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Should have 2 entries (both from EnabledScope)
    EXPECT_EQ(readEntries, 2U);

    // Decode and verify entries
    TLVReader reader;
    reader.Init(retrieveBuffer, span.size());

    for (uint32_t i = 0; i < readEntries; ++i)
    {
        EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

        DiagnosticEntry decoded;
        err = Decode(reader, decoded);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // Both entries should have "EnabledScope" as their string value
        EXPECT_STREQ(decoded.stringValue, "EnabledScope");
        EXPECT_EQ(decoded.type, ValueType::kCharString);
    }
}

TEST(ESP32DiagnosticsTest, CaseInsensitiveFiltering)
{
    uint8_t buffer[512];
    CircularDiagnosticBuffer storageBuffer(buffer, sizeof(buffer));
    ESP32Diagnostics diagnostics(&storageBuffer);

    diagnostics.ClearFilters();

    // Add filter with specific case
    CHIP_ERROR err = diagnostics.AddFilter("TestScope");
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // The filtering should be case-insensitive (based on MurmurHash implementation using tolower())
    EXPECT_TRUE(diagnostics.IsEnabled("TestScope"));
    EXPECT_TRUE(diagnostics.IsEnabled("testscope"));
    EXPECT_TRUE(diagnostics.IsEnabled("TESTSCOPE"));
    EXPECT_TRUE(diagnostics.IsEnabled("TestSCOPE"));
}

} // namespace

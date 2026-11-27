/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <platform/OpenThread/TxtEntries.h>

#include <pw_unit_test/framework.h>

#include <cstdint>
#include <cstring>
#include <vector>

using namespace chip;
using namespace chip::DeviceLayer::Internal;

namespace {

/**
 * Returns the entries of `sEntries` for any TXT data, so that the tests do not depend on OpenThread.
 */
struct FakeTxtIterator
{
    struct Entry
    {
        const char * mKey;
        const uint8_t * mValue;
        uint16_t mValueLength;
    };
    using Iterator = size_t;

    static void Init(Iterator & iterator, const uint8_t *, uint16_t) { iterator = 0; }

    static bool Next(Iterator & iterator, Entry & entry)
    {
        VerifyOrReturnValue(iterator < sEntries.size(), false);
        entry = sEntries[iterator++];
        return true;
    }

    static Span<const Entry> sEntries;
};

Span<const FakeTxtIterator::Entry> FakeTxtIterator::sEntries;

FakeTxtIterator::Entry MakeEntry(const char * key, const char * value)
{
    if (value == nullptr)
    {
        return { key, nullptr, 0 };
    }
    return { key, Uint8::from_const_char(value), static_cast<uint16_t>(strlen(value)) };
}

void ExpectEntry(const Dnssd::TextEntry & entry, const char * key, const char * value)
{
    EXPECT_STREQ(entry.mKey, key);
    EXPECT_TRUE(ByteSpan(entry.mData, entry.mDataSize).data_equal(ByteSpan(Uint8::from_const_char(value), strlen(value))));
}

bool IsInBuffer(const void * ptr, ByteSpan buffer)
{
    const uint8_t * bytes = static_cast<const uint8_t *>(ptr);
    return bytes >= buffer.data() && bytes < buffer.data() + buffer.size();
}

struct TestTxtEntries : public ::testing::Test
{
    void TearDown() override { FakeTxtIterator::sEntries = Span<const FakeTxtIterator::Entry>(); }
};

TEST_F(TestTxtEntries, CopiesKeysAndValuesIntoBuffer)
{
    const FakeTxtIterator::Entry txtEntries[] = { MakeEntry("SII", "5000"), MakeEntry("SAI", "300"), MakeEntry("DN", "") };
    FakeTxtIterator::sEntries                 = Span<const FakeTxtIterator::Entry>(txtEntries);
    Dnssd::TextEntry entryStorage[4];
    uint8_t buffer[64];
    Span<Dnssd::TextEntry> entries(entryStorage);

    EXPECT_EQ(ParseTxtEntries<FakeTxtIterator>(ByteSpan(), entries, MutableByteSpan(buffer)), CHIP_NO_ERROR);

    ASSERT_EQ(entries.size(), 3u);
    EXPECT_EQ(entries.data(), entryStorage);
    ExpectEntry(entries[0], "SII", "5000");
    ExpectEntry(entries[1], "SAI", "300");
    ExpectEntry(entries[2], "DN", "");

    for (const Dnssd::TextEntry & entry : entries)
    {
        EXPECT_TRUE(IsInBuffer(entry.mKey, ByteSpan(buffer)));
        EXPECT_TRUE(IsInBuffer(entry.mData, ByteSpan(buffer)));
    }
}

TEST_F(TestTxtEntries, SkipsEntriesWithoutKeyOrValue)
{
    const FakeTxtIterator::Entry txtEntries[] = { MakeEntry("ICD", nullptr), MakeEntry("CM", "1"), MakeEntry(nullptr, "1"),
                                                  MakeEntry("T", "0") };
    FakeTxtIterator::sEntries                 = Span<const FakeTxtIterator::Entry>(txtEntries);
    Dnssd::TextEntry entryStorage[2];
    uint8_t buffer[64];
    Span<Dnssd::TextEntry> entries(entryStorage);

    EXPECT_EQ(ParseTxtEntries<FakeTxtIterator>(ByteSpan(), entries, MutableByteSpan(buffer)), CHIP_NO_ERROR);

    ASSERT_EQ(entries.size(), 2u);
    ExpectEntry(entries[0], "CM", "1");
    ExpectEntry(entries[1], "T", "0");
}

TEST_F(TestTxtEntries, StopsWhenEntriesAreFull)
{
    constexpr size_t kEntryCapacity = 3;

    const FakeTxtIterator::Entry txtEntries[] = { MakeEntry("k0", "v"), MakeEntry("k1", "v"), MakeEntry("k2", "v"),
                                                  MakeEntry("k3", "v") };
    FakeTxtIterator::sEntries                 = Span<const FakeTxtIterator::Entry>(txtEntries);

    // The entry after the span checks that nothing is written past the span.
    Dnssd::TextEntry entryStorage[kEntryCapacity + 1] = {};
    uint8_t buffer[64];
    Span<Dnssd::TextEntry> entries(entryStorage, kEntryCapacity);

    EXPECT_EQ(ParseTxtEntries<FakeTxtIterator>(ByteSpan(), entries, MutableByteSpan(buffer)), CHIP_NO_ERROR);

    ASSERT_EQ(entries.size(), kEntryCapacity);
    ExpectEntry(entries[0], "k0", "v");
    ExpectEntry(entries[1], "k1", "v");
    ExpectEntry(entries[2], "k2", "v");
    EXPECT_EQ(entryStorage[kEntryCapacity].mKey, nullptr);
    EXPECT_EQ(entryStorage[kEntryCapacity].mData, nullptr);
    EXPECT_EQ(entryStorage[kEntryCapacity].mDataSize, 0u);
}

TEST_F(TestTxtEntries, FailsWhenBufferIsTooSmall)
{
    const FakeTxtIterator::Entry txtEntries[] = { MakeEntry("SII", "5000") };
    FakeTxtIterator::sEntries                 = Span<const FakeTxtIterator::Entry>(txtEntries);
    Dnssd::TextEntry entryStorage[4];
    uint8_t buffer[6];
    Span<Dnssd::TextEntry> entries(entryStorage);

    EXPECT_EQ(ParseTxtEntries<FakeTxtIterator>(ByteSpan(), entries, MutableByteSpan(buffer)), CHIP_ERROR_BUFFER_TOO_SMALL);
    EXPECT_EQ(entries.size(), 0u);
}

TEST_F(TestTxtEntries, FailsWhenBufferIsFullAfterSomeEntries)
{
    const FakeTxtIterator::Entry txtEntries[] = { MakeEntry("CM", "1"), MakeEntry("SII", "5000") };
    FakeTxtIterator::sEntries                 = Span<const FakeTxtIterator::Entry>(txtEntries);
    Dnssd::TextEntry entryStorage[4];
    uint8_t buffer[8];
    Span<Dnssd::TextEntry> entries(entryStorage);

    EXPECT_EQ(ParseTxtEntries<FakeTxtIterator>(ByteSpan(), entries, MutableByteSpan(buffer)), CHIP_ERROR_BUFFER_TOO_SMALL);
    EXPECT_EQ(entries.size(), 0u);
}

TEST_F(TestTxtEntries, RejectsDataLongerThanUint16Max)
{
    std::vector<uint8_t> txtData(UINT16_MAX + 1, 0);
    Dnssd::TextEntry entryStorage[4];
    uint8_t buffer[16];
    Span<Dnssd::TextEntry> entries(entryStorage);

    EXPECT_EQ(ParseTxtEntries<FakeTxtIterator>(ByteSpan(txtData.data(), txtData.size()), entries, MutableByteSpan(buffer)),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(entries.size(), 0u);
}

TEST_F(TestTxtEntries, AcceptsNoEntries)
{
    Dnssd::TextEntry entryStorage[4];
    uint8_t buffer[16];
    Span<Dnssd::TextEntry> entries(entryStorage);

    EXPECT_EQ(ParseTxtEntries<FakeTxtIterator>(ByteSpan(), entries, MutableByteSpan(buffer)), CHIP_NO_ERROR);
    EXPECT_EQ(entries.size(), 0u);
}

TEST_F(TestTxtEntries, ParseSetsServiceTextEntries)
{
    const FakeTxtIterator::Entry txtEntries[] = { MakeEntry("SII", "5000"), MakeEntry("T", "0") };
    FakeTxtIterator::sEntries                 = Span<const FakeTxtIterator::Entry>(txtEntries);
    TxtEntries<FakeTxtIterator, 4, 64> storage;
    Dnssd::DnssdService service;

    EXPECT_EQ(storage.Parse(ByteSpan(), service), CHIP_NO_ERROR);

    EXPECT_EQ(service.mTextEntries, storage.mTxtEntries);
    ASSERT_EQ(service.mTextEntrySize, 2u);
    ExpectEntry(service.mTextEntries[0], "SII", "5000");
    ExpectEntry(service.mTextEntries[1], "T", "0");
}

TEST_F(TestTxtEntries, ParseKeepsAtMostMaxEntries)
{
    constexpr size_t kMaxEntries = 3;

    const FakeTxtIterator::Entry txtEntries[] = { MakeEntry("k0", "v"), MakeEntry("k1", "v"), MakeEntry("k2", "v"),
                                                  MakeEntry("k3", "v") };
    FakeTxtIterator::sEntries                 = Span<const FakeTxtIterator::Entry>(txtEntries);

    // The entry after the storage checks that nothing is written past it.
    struct
    {
        TxtEntries<FakeTxtIterator, kMaxEntries, 64> storage;
        Dnssd::TextEntry after;
    } entries = {};
    Dnssd::DnssdService service;

    EXPECT_EQ(entries.storage.Parse(ByteSpan(), service), CHIP_NO_ERROR);

    ASSERT_EQ(service.mTextEntrySize, kMaxEntries);
    ExpectEntry(service.mTextEntries[0], "k0", "v");
    ExpectEntry(service.mTextEntries[1], "k1", "v");
    ExpectEntry(service.mTextEntries[2], "k2", "v");
    EXPECT_EQ(entries.after.mKey, nullptr);
}

TEST_F(TestTxtEntries, ParseClearsServiceTextEntriesOnFailure)
{
    const FakeTxtIterator::Entry txtEntries[] = { MakeEntry("SII", "5000") };
    FakeTxtIterator::sEntries                 = Span<const FakeTxtIterator::Entry>(txtEntries);
    TxtEntries<FakeTxtIterator, 4, 6> storage;
    Dnssd::DnssdService service;
    service.mTextEntrySize = 1;

    EXPECT_EQ(storage.Parse(ByteSpan(), service), CHIP_ERROR_BUFFER_TOO_SMALL);
    EXPECT_EQ(service.mTextEntrySize, 0u);
}

TEST_F(TestTxtEntries, ParseAcceptsNoEntries)
{
    TxtEntries<FakeTxtIterator, 4, 16> storage;
    Dnssd::DnssdService service;
    service.mTextEntrySize = 1;

    EXPECT_EQ(storage.Parse(ByteSpan(), service), CHIP_NO_ERROR);
    EXPECT_EQ(service.mTextEntrySize, 0u);
}

} // namespace

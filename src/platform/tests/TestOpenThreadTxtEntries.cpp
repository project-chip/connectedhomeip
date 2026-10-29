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
#include <lib/support/Span.h>
#include <platform/OpenThread/OpenThreadTxtEntries.h>

#include <pw_unit_test/framework.h>

#include <cstring>
#include <string>
#include <vector>

using namespace chip;
using namespace chip::DeviceLayer::Internal;

namespace {

void AppendTxtString(std::vector<uint8_t> & txtData, const std::string & str)
{
    txtData.push_back(static_cast<uint8_t>(str.size()));
    txtData.insert(txtData.end(), str.begin(), str.end());
}

std::vector<uint8_t> MakeTxtData(std::initializer_list<std::string> strings)
{
    std::vector<uint8_t> txtData;
    for (const std::string & str : strings)
    {
        AppendTxtString(txtData, str);
    }
    return txtData;
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

TEST(TestOpenThreadTxtEntries, CopiesKeysAndValuesIntoBuffer)
{
    std::vector<uint8_t> txtData = MakeTxtData({ "SII=5000", "SAI=300", "DN=" });
    Dnssd::TextEntry entryStorage[4];
    uint8_t buffer[64];
    Span<Dnssd::TextEntry> entries(entryStorage);

    EXPECT_EQ(ParseOpenThreadTxtEntries(ByteSpan(txtData.data(), txtData.size()), entries, MutableByteSpan(buffer)), CHIP_NO_ERROR);

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

TEST(TestOpenThreadTxtEntries, SkipsStringsWithoutKeyOrValue)
{
    const std::string longKey(OT_DNS_TXT_KEY_ITER_MAX_LENGTH + 1, 'k');
    std::vector<uint8_t> txtData = MakeTxtData({ "ICD", "CM=1", longKey + "=1", "=2", "T=0" });
    Dnssd::TextEntry entryStorage[2];
    uint8_t buffer[64];
    Span<Dnssd::TextEntry> entries(entryStorage);

    EXPECT_EQ(ParseOpenThreadTxtEntries(ByteSpan(txtData.data(), txtData.size()), entries, MutableByteSpan(buffer)), CHIP_NO_ERROR);

    ASSERT_EQ(entries.size(), 2u);
    ExpectEntry(entries[0], "CM", "1");
    ExpectEntry(entries[1], "T", "0");
}

TEST(TestOpenThreadTxtEntries, StopsWhenEntriesAreFull)
{
    constexpr size_t kEntryCapacity = 15;

    std::vector<uint8_t> txtData;
    for (size_t i = 0; i <= kEntryCapacity; i++)
    {
        AppendTxtString(txtData, "k" + std::to_string(i) + "=v");
    }

    // The entry after the span checks that nothing is written past the span.
    Dnssd::TextEntry entryStorage[kEntryCapacity + 1] = {};
    uint8_t buffer[256];
    Span<Dnssd::TextEntry> entries(entryStorage, kEntryCapacity);

    EXPECT_EQ(ParseOpenThreadTxtEntries(ByteSpan(txtData.data(), txtData.size()), entries, MutableByteSpan(buffer)), CHIP_NO_ERROR);

    ASSERT_EQ(entries.size(), kEntryCapacity);
    for (size_t i = 0; i < kEntryCapacity; i++)
    {
        ExpectEntry(entries[i], ("k" + std::to_string(i)).c_str(), "v");
    }
    EXPECT_EQ(entryStorage[kEntryCapacity].mKey, nullptr);
    EXPECT_EQ(entryStorage[kEntryCapacity].mData, nullptr);
    EXPECT_EQ(entryStorage[kEntryCapacity].mDataSize, 0u);
}

TEST(TestOpenThreadTxtEntries, FailsWhenBufferIsTooSmall)
{
    std::vector<uint8_t> txtData = MakeTxtData({ "SII=5000" });
    Dnssd::TextEntry entryStorage[4];
    uint8_t buffer[6];
    Span<Dnssd::TextEntry> entries(entryStorage);

    EXPECT_EQ(ParseOpenThreadTxtEntries(ByteSpan(txtData.data(), txtData.size()), entries, MutableByteSpan(buffer)),
              CHIP_ERROR_BUFFER_TOO_SMALL);
}

TEST(TestOpenThreadTxtEntries, StopsAtStringPastEndOfData)
{
    std::vector<uint8_t> txtData = MakeTxtData({ "CM=1", "T=0" });
    AppendTxtString(txtData, "D=840");
    txtData.pop_back();
    Dnssd::TextEntry entryStorage[4];
    uint8_t buffer[64];
    Span<Dnssd::TextEntry> entries(entryStorage);

    EXPECT_EQ(ParseOpenThreadTxtEntries(ByteSpan(txtData.data(), txtData.size()), entries, MutableByteSpan(buffer)), CHIP_NO_ERROR);

    ASSERT_EQ(entries.size(), 2u);
    ExpectEntry(entries[0], "CM", "1");
    ExpectEntry(entries[1], "T", "0");
}

TEST(TestOpenThreadTxtEntries, RejectsDataLongerThanOpenThreadAccepts)
{
    std::vector<uint8_t> txtData(UINT16_MAX + 1, 0);
    Dnssd::TextEntry entryStorage[4];
    uint8_t buffer[16];
    Span<Dnssd::TextEntry> entries(entryStorage);

    EXPECT_EQ(ParseOpenThreadTxtEntries(ByteSpan(txtData.data(), txtData.size()), entries, MutableByteSpan(buffer)),
              CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(TestOpenThreadTxtEntries, AcceptsEmptyData)
{
    Dnssd::TextEntry entryStorage[4];
    uint8_t buffer[16];
    Span<Dnssd::TextEntry> entries(entryStorage);

    EXPECT_EQ(ParseOpenThreadTxtEntries(ByteSpan(), entries, MutableByteSpan(buffer)), CHIP_NO_ERROR);
    EXPECT_EQ(entries.size(), 0u);
}

} // namespace

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

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPError.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLVCommon.h>
#include <lib/core/TLVTags.h>
#include <lib/core/TLVVectorWriter.h>
#include <lib/support/Span.h>
#include <lib/support/UnitTestUtils.h>

using namespace chip;
using namespace chip::TLV;

/**
 * context
 */

struct TestTLVContext
{
    int mEvictionCount     = 0;
    uint32_t mEvictedBytes = 0;
};

class TestTLVVectorWriter : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestTLVVectorWriter, InitAndFinalizeWithNoData)
{
    std::vector<uint8_t> buffer;
    TlvVectorWriter writer(buffer);

    // Init and finalize but write not data.
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);
    EXPECT_TRUE(buffer.empty());
}

TEST_F(TestTLVVectorWriter, SingleSmallDataFitsInOriginalBuffer)
{
    std::vector<uint8_t> buffer;
    TlvVectorWriter writer(buffer);
    TLVReader reader;

    EXPECT_EQ(writer.Put(AnonymousTag(), true), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    reader.Init(buffer.data(), buffer.size());
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_EQ(reader.GetTag(), AnonymousTag());

    bool value = false;
    EXPECT_EQ(reader.Get(value), CHIP_NO_ERROR);
    EXPECT_EQ(value, true);
}

TEST_F(TestTLVVectorWriter, SingleLargeDataRequiresNewBufferAllocation)
{
    std::vector<uint8_t> buffer;
    TlvVectorWriter writer(buffer);
    TLVReader reader;
    static constexpr size_t kStringSize = 10000;

    const std::string bytes(kStringSize, 'a');
    CHIP_ERROR error = writer.PutString(AnonymousTag(), bytes.data());
    EXPECT_EQ(error, CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    reader.Init(buffer.data(), buffer.size());
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_EQ(reader.GetTag(), AnonymousTag());

    CharSpan span;
    error = reader.Get(span);
    EXPECT_EQ(error, CHIP_NO_ERROR);
    EXPECT_EQ(std::string(span.data(), span.size()), bytes);
}

TEST_F(TestTLVVectorWriter, ManySmallDataRequiresNewBufferAllocation)
{
    std::vector<uint8_t> buffer;
    TlvVectorWriter writer(buffer);
    TLVReader reader;

    for (int i = 0; i < 10000; i++)
    {
        EXPECT_EQ(writer.Put(AnonymousTag(), true), CHIP_NO_ERROR);
    }
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    reader.Init(buffer.data(), buffer.size());
    for (int i = 0; i < 10000; i++)
    {
        EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
        EXPECT_EQ(reader.GetTag(), AnonymousTag());

        bool value       = false;
        CHIP_ERROR error = reader.Get(value);

        EXPECT_EQ(error, CHIP_NO_ERROR);
        EXPECT_EQ(value, true);
    }
    EXPECT_EQ(reader.Next(), CHIP_END_OF_TLV);
}

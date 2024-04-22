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

#include <lib/core/TLVVectorWriter.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <lib/core/CHIPError.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/TLVCommon.h>
#include <lib/core/TLVTags.h>
#include <lib/support/Span.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestExtendedAssertions.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/UnitTestUtils.h>

using namespace chip;
using namespace chip::TLV;

/**
 * context
 */

struct TestTLVContext
{
    nlTestSuite * mSuite   = nullptr;
    int mEvictionCount     = 0;
    uint32_t mEvictedBytes = 0;

    TestTLVContext(nlTestSuite * suite) : mSuite(suite) {}
};

void InitAndFinalizeWithNoData(nlTestSuite * inSuite, void * inContext)
{
    std::vector<uint8_t> buffer;
    TlvVectorWriter writer(buffer);

    // Init and finalize but write not data.
    NL_TEST_ASSERT(inSuite, writer.Finalize() == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, buffer.empty());
}

void SingleSmallDataFitsInOriginalBuffer(nlTestSuite * inSuite, void * inContext)
{
    std::vector<uint8_t> buffer;
    TlvVectorWriter writer(buffer);
    TLVReader reader;

    NL_TEST_ASSERT(inSuite, writer.Put(AnonymousTag(), true) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, writer.Finalize() == CHIP_NO_ERROR);

    reader.Init(buffer.data(), buffer.size());
    NL_TEST_ASSERT(inSuite, reader.Next() == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, reader.GetTag() == AnonymousTag());

    bool value = false;
    NL_TEST_ASSERT(inSuite, reader.Get(value) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, value == true);
}

void SingleLargeDataRequiresNewBufferAllocation(nlTestSuite * inSuite, void * inContext)
{
    std::vector<uint8_t> buffer;
    TlvVectorWriter writer(buffer);
    TLVReader reader;
    static constexpr size_t kStringSize = 10000;

    const std::string bytes(kStringSize, 'a');
    CHIP_ERROR error = writer.PutString(AnonymousTag(), bytes.data());
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, writer.Finalize() == CHIP_NO_ERROR);

    reader.Init(buffer.data(), buffer.size());
    NL_TEST_ASSERT(inSuite, reader.Next() == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, reader.GetTag() == AnonymousTag());

    CharSpan span;
    error = reader.Get(span);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, std::string(span.data(), span.size()) == bytes);
}

void ManySmallDataRequiresNewBufferAllocation(nlTestSuite * inSuite, void * inContext)
{
    std::vector<uint8_t> buffer;
    TlvVectorWriter writer(buffer);
    TLVReader reader;

    for (int i = 0; i < 10000; i++)
    {
        NL_TEST_ASSERT(inSuite, writer.Put(AnonymousTag(), true) == CHIP_NO_ERROR);
    }
    NL_TEST_ASSERT(inSuite, writer.Finalize() == CHIP_NO_ERROR);

    reader.Init(buffer.data(), buffer.size());
    for (int i = 0; i < 10000; i++)
    {
        NL_TEST_ASSERT(inSuite, reader.Next() == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, reader.GetTag() == AnonymousTag());

        bool value       = false;
        CHIP_ERROR error = reader.Get(value);

        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, value == true);
    }
    NL_TEST_ASSERT(inSuite, reader.Next() == CHIP_END_OF_TLV);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Verify behavior on init and finalize without data manipulation", InitAndFinalizeWithNoData),
    NL_TEST_DEF("Ensure correct write/read operations within Inet buffer constraints", SingleSmallDataFitsInOriginalBuffer),
    NL_TEST_DEF("Handle cases where a single large data input exceeds buffer capacity", SingleLargeDataRequiresNewBufferAllocation),
    NL_TEST_DEF("Validate output formatting for multiple small data inputs requiring additional buffer space", ManySmallDataRequiresNewBufferAllocation),
    NL_TEST_SENTINEL()
};
// clang-format on

/**
 *  Set up the test suite.
 */
int TestTLVVectorWriter_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestTLVVectorWriter_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

int TestTLVVectorWriter()
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "chip-tlv",
        &sTests[0],
        TestTLVVectorWriter_Setup,
        TestTLVVectorWriter_Teardown
    };
    // clang-format on

    return chip::ExecuteTestsWithContext<TestTLVContext>(&theSuite, &theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestTLVVectorWriter)

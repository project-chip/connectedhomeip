/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "TestSupport.h"

#include <support/CHIPMem.h>
#include <support/SparseArray.h>
#include <support/TestUtils.h>

#include <nlunit-test.h>

namespace {

void TestSparseArray(nlTestSuite * inSuite, void * inContext)
{
    chip::SparseU64Array<8> array;
    NL_TEST_ASSERT(inSuite, array.Find(123) == 8);

    NL_TEST_ASSERT(inSuite, array.Insert(123) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, array.Find(123) == 0);

    NL_TEST_ASSERT(inSuite, array.Insert(123) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, array.Find(123) == 0);

    array.Remove(123);
    NL_TEST_ASSERT(inSuite, array.Find(123) == 8);

    for (uint64_t i = 1; i <= 8; i++)
    {
        NL_TEST_ASSERT(inSuite, array.Insert(i) == CHIP_NO_ERROR);
    }

    NL_TEST_ASSERT(inSuite, array.Insert(9) != CHIP_NO_ERROR);

    for (uint64_t i = 1; i <= 8; i++)
    {
        NL_TEST_ASSERT(inSuite, array.Find(i) != 8);
    }

    size_t size = array.SerializedSize();
    NL_TEST_ASSERT(inSuite, array.MaxSerializedSize() == size);

    for (uint64_t i = 1; i <= 7; i++)
    {
        array.Remove(i);
        NL_TEST_ASSERT(inSuite, array.SerializedSize() == size);
    }

    array.Remove(8);
    NL_TEST_ASSERT(inSuite, array.SerializedSize() == 0);
}

void TestSparseArraySerialize(nlTestSuite * inSuite, void * inContext)
{
    chip::SparseU64Array<8> array;

    for (uint64_t i = 1; i <= 6; i++)
    {
        NL_TEST_ASSERT(inSuite, array.Insert(i) == CHIP_NO_ERROR);
    }

    char * buf    = nullptr;
    uint16_t size = 0;

    NL_TEST_ASSERT(inSuite, array.SerializeBase64(buf, size) == nullptr);
    NL_TEST_ASSERT(inSuite, size != 0);

    char buf1[size];
    NL_TEST_ASSERT(inSuite, array.SerializeBase64(buf1, size) == buf1);
    NL_TEST_ASSERT(inSuite, size != 0);

    uint16_t size2 = static_cast<uint16_t>(2 * size);
    char buf2[size2];
    NL_TEST_ASSERT(inSuite, array.SerializeBase64(buf2, size2) == buf2);
    NL_TEST_ASSERT(inSuite, size2 == size);

    chip::SparseU64Array<8> array2;
    NL_TEST_ASSERT(inSuite, array2.DeserializeBase64(buf2, size2) == CHIP_NO_ERROR);

    for (uint64_t i = 1; i <= 6; i++)
    {
        NL_TEST_ASSERT(inSuite, array2.Find(i) != 8);
    }
    NL_TEST_ASSERT(inSuite, array2.Find(7) == 8);
}

int Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

int Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

} // namespace

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {
    NL_TEST_DEF_FN(TestSparseArray),          //
    NL_TEST_DEF_FN(TestSparseArraySerialize), //
    NL_TEST_SENTINEL()                        //
};

int TestSparseArray(void)
{
    nlTestSuite theSuite = { "CHIP Sparse Array tests", &sTests[0], Setup, Teardown };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestBufBound)

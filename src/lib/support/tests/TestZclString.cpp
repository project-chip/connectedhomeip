/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *  This file implements a unit test suite for ZclString
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/ZclString.h>

using namespace chip;
using namespace chip::Logging;
using namespace chip::Platform;

class TestZclString : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

bool allCharactersSame(uint8_t zclString[])
{
    int n = zclString[0];
    for (int i = 1; i < n; i++)
        if (zclString[i] != zclString[1])
            return false;

    return true;
}

TEST_F(TestZclString, TestZclStringWhenBufferIsZero)
{
    uint8_t bufferMemory[1];
    MutableByteSpan zclString(bufferMemory);
    chip::Platform::ScopedMemoryBuffer<char> cString1;
    EXPECT_TRUE(cString1.Calloc(1024));
    memset(cString1.Get(), 'A', 1);

    CHIP_ERROR err = MakeZclCharString(zclString, cString1.Get());

    EXPECT_EQ(err, CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG);
    EXPECT_EQ(zclString.data()[0], 0);
}

TEST_F(TestZclString, TestZclStringLessThanMaximumSize_Length_64)
{
    uint8_t bufferMemory[256];
    MutableByteSpan zclString(bufferMemory);
    chip::Platform::ScopedMemoryBuffer<char> cString64;
    EXPECT_TRUE(cString64.Calloc(1024));
    memset(cString64.Get(), 'A', 64);

    CHIP_ERROR err = MakeZclCharString(zclString, cString64.Get());

    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(zclString.data()[0], 64);
    EXPECT_TRUE(allCharactersSame(zclString.data()));
}

TEST_F(TestZclString, TestZclStringEqualsMaximumSize)
{
    uint8_t bufferMemory[256];
    MutableByteSpan zclString(bufferMemory);
    chip::Platform::ScopedMemoryBuffer<char> cString254;
    EXPECT_TRUE(cString254.Calloc(1024));
    memset(cString254.Get(), 'A', 254);

    CHIP_ERROR err = MakeZclCharString(zclString, cString254.Get());

    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(zclString.data()[0], 254);
    EXPECT_TRUE(allCharactersSame(zclString.data()));
}

TEST_F(TestZclString, TestSizeZclStringBiggerThanMaximumSize_Length_255)
{
    uint8_t bufferMemory[256];
    MutableByteSpan zclString(bufferMemory);
    chip::Platform::ScopedMemoryBuffer<char> cString255;
    EXPECT_TRUE(cString255.Calloc(1024));
    memset(cString255.Get(), 'A', 255);

    CHIP_ERROR err = MakeZclCharString(zclString, cString255.Get());

    EXPECT_EQ(err, CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG);
    EXPECT_EQ(zclString.data()[0], 0);
}

TEST_F(TestZclString, TestSizeZclStringBiggerThanMaximumSize_Length_256)
{
    uint8_t bufferMemory[256];
    MutableByteSpan zclString(bufferMemory);
    chip::Platform::ScopedMemoryBuffer<char> cString256;
    EXPECT_TRUE(cString256.Calloc(1024));
    memset(cString256.Get(), 'A', 256);

    CHIP_ERROR err = MakeZclCharString(zclString, cString256.Get());

    EXPECT_EQ(err, CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG);
    EXPECT_EQ(zclString.data()[0], 0);
}

TEST_F(TestZclString, TestZclStringBiggerThanMaximumSize_Length_257)
{
    uint8_t bufferMemory[257];
    MutableByteSpan zclString(bufferMemory);
    chip::Platform::ScopedMemoryBuffer<char> cString257;
    EXPECT_TRUE(cString257.Calloc(1024));
    memset(cString257.Get(), 'A', 257);

    CHIP_ERROR err = MakeZclCharString(zclString, cString257.Get());

    EXPECT_EQ(err, CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG);
    EXPECT_EQ(zclString.data()[0], 0);
}

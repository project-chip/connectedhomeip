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

/**
 *    @file
 *      This file implements a unit test suite for CHIP BufBound
 *
 */

#include "TestSupport.h"

#include <support/BufBound.h>

#include <nlunit-test.h>

using namespace chip;

class BBTest : public BufBound
{
public:
    static const size_t kLen    = 64;
    static const uint8_t kGuard = 0xfe;
    uint8_t mBuf[kLen];
    size_t mLen;

    BBTest(size_t len) : BufBound(mBuf + 1, len), mLen(len) { memset(mBuf, kGuard, kLen); }

    bool expect(const void * val, size_t written, size_t available)
    {
        // check guards
        for (size_t i = mLen + 1; i < sizeof(mBuf); i++)
        {
            if (mBuf[i] != kGuard)
            {
                return false;
            }
        }
        if (mBuf[0] != kGuard)
        {
            return false;
        }

        if ((mLen < written && Fit()) || (mLen >= written && !Fit()))
        {
            printf("fit is wrong mLen == %zu, written == %zu, Fit() == %s\n", mLen, written, Fit() ? "true" : "false");
            return false;
        }

        // check everything else
        if (memcmp(mBuf + 1, val, written < mLen ? written : mLen) != 0)
        {
            return false;
        }

        return Available() == available && Written() == written;
    }
};

static void TestBufBound_Str(nlTestSuite * inSuite, void * inContext)
{
    {
        (void) inContext;
        BBTest bb(2);

        bb.Put("hi");

        NL_TEST_ASSERT(inSuite, bb.expect("hi", 2, 0));
    }
    {
        (void) inContext;
        BBTest bb(1);
        bb.Put("hi");

        NL_TEST_ASSERT(inSuite, bb.expect("hi", 2, 0));
    }
}

static void TestBufBound_Buf(nlTestSuite * inSuite, void * inContext)
{
    {
        (void) inContext;
        BBTest bb(2);
        bb.Put("hi", 2);

        NL_TEST_ASSERT(inSuite, bb.expect("hi", 2, 0));
    }
    {
        (void) inContext;
        BBTest bb(1);
        bb.Put("hi", 2);

        NL_TEST_ASSERT(inSuite, bb.expect("hi", 2, 0));
    }
}

static void TestBufBound_PutLE(nlTestSuite * inSuite, void * inContext)
{
    (void) inContext;
    {
        BBTest bb(2);

        bb.PutLE16('h' + 'i' * 256);

        NL_TEST_ASSERT(inSuite, bb.expect("hi", 2, 0));
    }
    {
        BBTest bb(4);

        bb.PutLE32(0x01020304);

        NL_TEST_ASSERT(inSuite, bb.expect("\x04\x03\x02\x01", 4, 0));
    }

    {
        BBTest bb(8);

        bb.PutLE64(0x0102030405060708);

        NL_TEST_ASSERT(inSuite, bb.expect("\x08\x07\x06\x05\x04\x03\x02\x01", 8, 0));
    }

    {
        BBTest bb(3);

        bb.PutLE(0x0102030405060708u, 3);

        NL_TEST_ASSERT(inSuite, bb.expect("\x08\x07\x06", 3, 0));
    }
}

static void TestBufBound_PutBE(nlTestSuite * inSuite, void * inContext)
{
    (void) inContext;
    {
        BBTest bb(2);

        bb.PutBE16('i' + 'h' * 256);

        NL_TEST_ASSERT(inSuite, bb.expect("hi", 2, 0));
    }

    {
        BBTest bb(4);

        bb.PutBE32(0x01020304);

        NL_TEST_ASSERT(inSuite, bb.expect("\x01\x02\x03\x04", 4, 0));
    }

    {
        BBTest bb(8);

        bb.PutBE64(0x0102030405060708);

        NL_TEST_ASSERT(inSuite, bb.expect("\x01\x02\x03\x04\x05\x06\x07\x08", 8, 0));
    }

    {
        BBTest bb(3);

        bb.PutBE(0x0102030405060708u, 3);

        NL_TEST_ASSERT(inSuite, bb.expect("\x06\x07\x08", 3, 0));
    }
}

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF_FN(TestBufBound_Str), NL_TEST_DEF_FN(TestBufBound_Buf),
                                 NL_TEST_DEF_FN(TestBufBound_PutLE), NL_TEST_DEF_FN(TestBufBound_PutBE), NL_TEST_SENTINEL() };

int TestBufBound(void)
{
    nlTestSuite theSuite = { "CHIP BufBound tests", &sTests[0], NULL, NULL };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}

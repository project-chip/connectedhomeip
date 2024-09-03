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
 *      This file implements a unit test suite for CHIP Memory Management
 *      code functionality.
 *
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

using namespace chip;
using namespace chip::Logging;
using namespace chip::Platform;

// =================================
//      Unit tests
// =================================

class TestCHIPMem : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestCHIPMem, TestMemAlloc_Malloc)
{
    char * p1 = nullptr;
    char * p2 = nullptr;
    char * p3 = nullptr;

    // Verify long-term allocation
    p1 = static_cast<char *>(MemoryAlloc(64));
    EXPECT_NE(p1, nullptr);

    p2 = static_cast<char *>(MemoryAlloc(256));
    EXPECT_NE(p2, nullptr);

    chip::Platform::MemoryFree(p1);
    chip::Platform::MemoryFree(p2);

    // Verify short-term allocation
    p1 = static_cast<char *>(MemoryAlloc(256));
    EXPECT_NE(p1, nullptr);

    p2 = static_cast<char *>(MemoryAlloc(256));
    EXPECT_NE(p2, nullptr);

    p3 = static_cast<char *>(MemoryAlloc(256));
    EXPECT_NE(p3, nullptr);

    chip::Platform::MemoryFree(p1);
    chip::Platform::MemoryFree(p2);
    chip::Platform::MemoryFree(p3);
}

TEST_F(TestCHIPMem, TestMemAlloc_Calloc)
{
    char * p = static_cast<char *>(MemoryCalloc(128, sizeof(char)));
    ASSERT_NE(p, nullptr);

    for (int i = 0; i < 128; i++)
        EXPECT_EQ(p[i], 0);

    chip::Platform::MemoryFree(p);
}

TEST_F(TestCHIPMem, TestMemAlloc_Realloc)
{
    char * pa = static_cast<char *>(MemoryAlloc(128));
    EXPECT_NE(pa, nullptr);

    char * pb = static_cast<char *>(MemoryRealloc(pa, 256));
    EXPECT_NE(pb, nullptr);

    chip::Platform::MemoryFree(pb);
}

TEST_F(TestCHIPMem, TestMemAlloc_UniquePtr)
{
    // UniquePtr is a wrapper of std::unique_ptr for platform allocators, we just check if we created a correct wrapper here.
    int constructorCalled = 0;
    int destructorCalled  = 0;

    class Cls
    {
    public:
        Cls(int * constructCtr, int * desctructorCtr) : mpDestructorCtr(desctructorCtr) { (*constructCtr)++; }
        ~Cls() { (*mpDestructorCtr)++; }

    private:
        int * mpDestructorCtr;
    };

    {
        auto ptr = MakeUnique<Cls>(&constructorCalled, &destructorCalled);
        EXPECT_EQ(constructorCalled, 1);
        EXPECT_EQ(destructorCalled, 0);
        IgnoreUnusedVariable(ptr);
    }

    EXPECT_TRUE(destructorCalled);
}

TEST_F(TestCHIPMem, TestMemAlloc_SharedPtr)
{
    // SharedPtr is a wrapper of std::shared_ptr for platform allocators.
    int instanceConstructorCalled      = 0;
    int instanceDestructorCalled       = 0;
    int otherInstanceConstructorCalled = 0;
    int otherInstanceDestructorCalled  = 0;

    class Cls
    {
    public:
        Cls(int * constructCtr, int * desctructorCtr) : mpDestructorCtr(desctructorCtr) { (*constructCtr)++; }
        ~Cls() { (*mpDestructorCtr)++; }

    private:
        int * mpDestructorCtr;
    };

    // Check constructor call for a block-scoped variable and share our
    // reference to a function-scoped variable.
    SharedPtr<Cls> otherReference;
    {
        auto ptr = MakeShared<Cls>(&instanceConstructorCalled, &instanceDestructorCalled);
        EXPECT_EQ(instanceConstructorCalled, 1);
        // Capture a shared reference so we aren't destructed when we leave this scope.
        otherReference = ptr;
    }

    // Verify that by sharing to otherReference, we weren't destructed.
    EXPECT_EQ(instanceDestructorCalled, 0);

    // Now drop our reference.
    otherReference = MakeShared<Cls>(&otherInstanceConstructorCalled, &otherInstanceDestructorCalled);

    // Verify that the new instance was constructed and the first instance was
    // destructed, and that we retain a reference to the new instance.
    EXPECT_EQ(instanceConstructorCalled, 1);
    EXPECT_EQ(instanceDestructorCalled, 1);
    EXPECT_EQ(otherInstanceConstructorCalled, 1);
    EXPECT_EQ(otherInstanceDestructorCalled, 0);
}

/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This file implements a test for  CHIP core library reference counted object.
 *
 */

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include <pw_unit_test/framework.h>

#include <lib/core/ReferenceCounted.h>
#include <lib/core/StringBuilderAdapters.h>

using namespace chip;

class TestReferenceCounted : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

class TestClass : public ReferenceCounted<TestClass>
{
};

TEST_F(TestReferenceCounted, TestRetainRelease)
{
    TestClass * testObj = chip::Platform::New<TestClass>();
    EXPECT_EQ(testObj->GetReferenceCount(), 1u);
    testObj->Retain();
    EXPECT_EQ(testObj->GetReferenceCount(), 2u);
    testObj->Release();
    EXPECT_EQ(testObj->GetReferenceCount(), 1u);
    testObj->Release();
}

class TestClassNonHeap;
class Deletor
{
public:
    static void Release(TestClassNonHeap * obj);
};

class TestClassNonHeap : public ReferenceCounted<TestClassNonHeap, Deletor>
{
public:
    bool deleted;
};

void Deletor::Release(TestClassNonHeap * obj)
{
    obj->deleted = true;
}

TEST_F(TestReferenceCounted, TestRetainReleaseNonHeap)
{
    TestClassNonHeap testObj;
    testObj.deleted = false;
    EXPECT_EQ(testObj.GetReferenceCount(), 1u);
    EXPECT_EQ(testObj.deleted, false);
    testObj.Retain();
    EXPECT_EQ(testObj.GetReferenceCount(), 2u);
    EXPECT_EQ(testObj.deleted, false);
    testObj.Release();
    EXPECT_EQ(testObj.GetReferenceCount(), 1u);
    EXPECT_EQ(testObj.deleted, false);
    testObj.Release();
    EXPECT_EQ(testObj.GetReferenceCount(), 0u);
    EXPECT_EQ(testObj.deleted, true);
}

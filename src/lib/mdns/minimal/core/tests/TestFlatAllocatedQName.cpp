/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include <lib/mdns/minimal/core/FlatAllocatedQName.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace mdns::Minimal;

class AutoFreeBuffer
{
public:
    AutoFreeBuffer(size_t n) { mBuffer = malloc(n); }
    ~AutoFreeBuffer() { free(mBuffer); }

    AutoFreeBuffer(const AutoFreeBuffer &) = delete;
    AutoFreeBuffer & operator=(const AutoFreeBuffer &) = delete;

    void * Buffer() { return mBuffer; }

private:
    void * mBuffer;
};

void TestFlatAllocatedQName(nlTestSuite * inSuite, void * inContext)
{
    AutoFreeBuffer buffer(128);

    NL_TEST_ASSERT(inSuite, FlatAllocatedQName::RequiredStorageSize("some", "test") == (sizeof(char * [2]) + 5 + 5));

    {
        FullQName built            = FlatAllocatedQName::Build(buffer.Buffer(), "some", "test");
        const QNamePart expected[] = { "some", "test" };

        NL_TEST_ASSERT(inSuite, FullQName(expected) == built);
    }

    {
        FullQName built            = FlatAllocatedQName::Build(buffer.Buffer(), "1", "2", "3");
        const QNamePart expected[] = { "1", "2", "3" };

        NL_TEST_ASSERT(inSuite, FullQName(expected) == built);
    }
}

void SizeCompare(nlTestSuite * inSuite, void * inContext)
{
    const char kThis[]    = "this";
    const char kIs[]      = "is";
    const char kA[]       = "a";
    const char kTest[]    = "test";
    const char kVest[]    = "vest";
    const char kBee[]     = "bee";
    const char kRobbery[] = "robbery";
    const char kExtra[]   = "extra";

    const char * kSameArraySameSize[4]        = { kThis, kIs, kA, kTest };
    const char * kDifferentArraySameSize[4]   = { kThis, kIs, kA, kVest };
    const char * kDifferenArrayLongerWord[4]  = { kThis, kIs, kA, kRobbery };
    const char * kDifferenArrayShorterWord[4] = { kThis, kIs, kA, kBee };
    const char * kSameArrayExtraWord[5]       = { kThis, kIs, kA, kTest, kExtra };
    const char * kShorterArray[3]             = { kThis, kIs, kA };

    const size_t kTestStorageSize = FlatAllocatedQName::RequiredStorageSize(kThis, kIs, kA, kTest);

    NL_TEST_ASSERT(inSuite, kTestStorageSize == FlatAllocatedQName::RequiredStorageSizeFromArray(kSameArraySameSize, 4));
    NL_TEST_ASSERT(inSuite, kTestStorageSize == FlatAllocatedQName::RequiredStorageSizeFromArray(kDifferentArraySameSize, 4));
    NL_TEST_ASSERT(inSuite, kTestStorageSize < FlatAllocatedQName::RequiredStorageSizeFromArray(kDifferenArrayLongerWord, 4));
    NL_TEST_ASSERT(inSuite, kTestStorageSize > FlatAllocatedQName::RequiredStorageSizeFromArray(kDifferenArrayShorterWord, 4));

    // Although the size of the array is larger, if we tell the function there are only 4 words, it should still work.
    NL_TEST_ASSERT(inSuite, kTestStorageSize == FlatAllocatedQName::RequiredStorageSizeFromArray(kSameArrayExtraWord, 4));
    // If we add the extra word, the sizes should not match
    NL_TEST_ASSERT(inSuite, kTestStorageSize < FlatAllocatedQName::RequiredStorageSizeFromArray(kSameArrayExtraWord, 5));

    NL_TEST_ASSERT(inSuite, kTestStorageSize > FlatAllocatedQName::RequiredStorageSizeFromArray(kShorterArray, 3));
    NL_TEST_ASSERT(inSuite,
                   FlatAllocatedQName::RequiredStorageSizeFromArray(kSameArraySameSize, 3) ==
                       FlatAllocatedQName::RequiredStorageSizeFromArray(kShorterArray, 3));
}

void BuildCompare(nlTestSuite * inSuite, void * inContext)
{
    const char kThis[]  = "this";
    const char kIs[]    = "is";
    const char kA[]     = "a";
    const char kTest[]  = "test";
    const char kExtra[] = "extra";

    const char * kSameArraySameSize[4]  = { kThis, kIs, kA, kTest };
    const char * kSameArrayExtraWord[5] = { kThis, kIs, kA, kTest, kExtra };
    const char * kShorterArray[3]       = { kThis, kIs, kA };

    uint8_t storage[256];

    const FullQName kTestQName = FlatAllocatedQName::Build(storage, kThis, kIs, kA, kTest);

    NL_TEST_ASSERT(inSuite, kTestQName == FlatAllocatedQName::BuildFromArray(storage, kSameArraySameSize, 4));

    // Although the size of the array is larger, if we tell the function there are only 4 words, it should still work.
    NL_TEST_ASSERT(inSuite, kTestQName == FlatAllocatedQName::BuildFromArray(storage, kSameArrayExtraWord, 4));
    // If we add the extra word, the names
    NL_TEST_ASSERT(inSuite, kTestQName != FlatAllocatedQName::BuildFromArray(storage, kSameArrayExtraWord, 5));

    NL_TEST_ASSERT(inSuite, kTestQName != FlatAllocatedQName::BuildFromArray(storage, kShorterArray, 3));
    NL_TEST_ASSERT(inSuite,
                   FlatAllocatedQName::BuildFromArray(storage, kSameArraySameSize, 3) ==
                       FlatAllocatedQName::BuildFromArray(storage, kShorterArray, 3));
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestFlatAllocatedQName", TestFlatAllocatedQName),   //
    NL_TEST_DEF("TestFlatAllocatedQNameRequiredSizes", SizeCompare), //
    NL_TEST_DEF("TestFlatAllocatedQNameBuild", BuildCompare),        //
    NL_TEST_SENTINEL()                                               //
};

} // namespace

int TestFlatAllocatedQName(void)
{
    nlTestSuite theSuite = { "FlatAllocatedQName", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestFlatAllocatedQName)

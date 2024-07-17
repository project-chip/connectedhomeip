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

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/minimal_mdns/core/FlatAllocatedQName.h>

namespace {

using namespace mdns::Minimal;

class AutoFreeBuffer
{
public:
    AutoFreeBuffer(size_t n) { mBuffer = malloc(n); }
    ~AutoFreeBuffer() { free(mBuffer); }

    AutoFreeBuffer(const AutoFreeBuffer &)             = delete;
    AutoFreeBuffer & operator=(const AutoFreeBuffer &) = delete;

    void * Buffer() { return mBuffer; }

private:
    void * mBuffer;
};

TEST(TestFlatAllocatedQName, TestFlatAllocatedQName)
{
    AutoFreeBuffer buffer(128);

    EXPECT_EQ(FlatAllocatedQName::RequiredStorageSize("some", "test"), (sizeof(char * [2]) + 5 + 5));

    {
        FullQName built            = FlatAllocatedQName::Build(buffer.Buffer(), "some", "test");
        const QNamePart expected[] = { "some", "test" };

        EXPECT_EQ(FullQName(expected), built);
    }

    {
        FullQName built            = FlatAllocatedQName::Build(buffer.Buffer(), "1", "2", "3");
        const QNamePart expected[] = { "1", "2", "3" };

        EXPECT_EQ(FullQName(expected), built);
    }
}

TEST(TestFlatAllocatedQName, SizeCompare)
{
    static const char kThis[]    = "this";
    static const char kIs[]      = "is";
    static const char kA[]       = "a";
    static const char kTest[]    = "test";
    static const char kVest[]    = "vest";
    static const char kBee[]     = "bee";
    static const char kRobbery[] = "robbery";
    static const char kExtra[]   = "extra";

    const char * kSameArraySameSize[4]        = { kThis, kIs, kA, kTest };
    const char * kDifferentArraySameSize[4]   = { kThis, kIs, kA, kVest };
    const char * kDifferenArrayLongerWord[4]  = { kThis, kIs, kA, kRobbery };
    const char * kDifferenArrayShorterWord[4] = { kThis, kIs, kA, kBee };
    const char * kSameArrayExtraWord[5]       = { kThis, kIs, kA, kTest, kExtra };
    const char * kShorterArray[3]             = { kThis, kIs, kA };

    const size_t kTestStorageSize = FlatAllocatedQName::RequiredStorageSize(kThis, kIs, kA, kTest);

    EXPECT_EQ(kTestStorageSize, FlatAllocatedQName::RequiredStorageSizeFromArray(kSameArraySameSize, 4));
    EXPECT_EQ(kTestStorageSize, FlatAllocatedQName::RequiredStorageSizeFromArray(kDifferentArraySameSize, 4));
    EXPECT_LT(kTestStorageSize, FlatAllocatedQName::RequiredStorageSizeFromArray(kDifferenArrayLongerWord, 4));
    EXPECT_GT(kTestStorageSize, FlatAllocatedQName::RequiredStorageSizeFromArray(kDifferenArrayShorterWord, 4));

    // Although the size of the array is larger, if we tell the function there are only 4 words, it should still work.
    EXPECT_EQ(kTestStorageSize, FlatAllocatedQName::RequiredStorageSizeFromArray(kSameArrayExtraWord, 4));
    // If we add the extra word, the sizes should not match
    EXPECT_LT(kTestStorageSize, FlatAllocatedQName::RequiredStorageSizeFromArray(kSameArrayExtraWord, 5));

    EXPECT_GT(kTestStorageSize, FlatAllocatedQName::RequiredStorageSizeFromArray(kShorterArray, 3));
    EXPECT_EQ(FlatAllocatedQName::RequiredStorageSizeFromArray(kSameArraySameSize, 3),
              FlatAllocatedQName::RequiredStorageSizeFromArray(kShorterArray, 3));
}

TEST(TestFlatAllocatedQName, BuildCompare)
{
    static const char kThis[]  = "this";
    static const char kIs[]    = "is";
    static const char kA[]     = "a";
    static const char kTest[]  = "test";
    static const char kExtra[] = "extra";

    const char * kSameArraySameSize[4]  = { kThis, kIs, kA, kTest };
    const char * kSameArrayExtraWord[5] = { kThis, kIs, kA, kTest, kExtra };
    const char * kShorterArray[3]       = { kThis, kIs, kA };

    uint8_t storage[256];

    const FullQName kTestQName = FlatAllocatedQName::Build(storage, kThis, kIs, kA, kTest);

    EXPECT_EQ(kTestQName, FlatAllocatedQName::BuildFromArray(storage, kSameArraySameSize, 4));

    // Although the size of the array is larger, if we tell the function there are only 4 words, it should still work.
    EXPECT_EQ(kTestQName, FlatAllocatedQName::BuildFromArray(storage, kSameArrayExtraWord, 4));
    // If we add the extra word, the names
    EXPECT_NE(kTestQName, FlatAllocatedQName::BuildFromArray(storage, kSameArrayExtraWord, 5));

    EXPECT_NE(kTestQName, FlatAllocatedQName::BuildFromArray(storage, kShorterArray, 3));
    EXPECT_EQ(FlatAllocatedQName::BuildFromArray(storage, kSameArraySameSize, 3),
              FlatAllocatedQName::BuildFromArray(storage, kShorterArray, 3));
}
} // namespace

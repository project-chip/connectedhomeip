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
#include <mdns/minimal/core/FlatAllocatedQName.h>
#include <support/UnitTestRegistration.h>

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

const nlTest sTests[] = {
    NL_TEST_DEF("TestFlatAllocatedQName", TestFlatAllocatedQName), //
    NL_TEST_SENTINEL()                                             //
};

} // namespace

int TestFlatAllocatedQName(void)
{
    nlTestSuite theSuite = { "FlatAllocatedQName", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestFlatAllocatedQName)

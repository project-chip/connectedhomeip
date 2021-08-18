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

#include <mdns/Discovery_ImplPlatform.h>
#include <support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace chip::Mdns;

void TestStub(nlTestSuite * inSuite, void * inContext)
{
    DiscoveryImplPlatform & mdnsPlatform = DiscoveryImplPlatform::GetInstance();
    NL_TEST_ASSERT(inSuite, mdnsPlatform.Init() == CHIP_NO_ERROR);
    OperationalAdvertisingParameters params;
    NL_TEST_ASSERT(inSuite, mdnsPlatform.Advertise(params) == CHIP_ERROR_NOT_IMPLEMENTED);
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestStub", TestStub), //
    NL_TEST_SENTINEL()                 //
};

} // namespace

int TestMdnsPlatform(void)
{
    nlTestSuite theSuite = { "MdnsPlatform", &sTests[0], nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestMdnsPlatform)

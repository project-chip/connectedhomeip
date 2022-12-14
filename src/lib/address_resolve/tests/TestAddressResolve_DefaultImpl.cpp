/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <lib/address_resolve/AddressResolve_DefaultImpl.h>

#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

void TestSingleResolveResult(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, true);
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestSingleResolveResult", TestSingleResolveResult), //
    NL_TEST_SENTINEL()                                               //
};

} // namespace

int TestAddressResolve_DefaultImpl()
{
    nlTestSuite theSuite = { "AddressResolve_DefaultImpl", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestAddressResolve_DefaultImpl)

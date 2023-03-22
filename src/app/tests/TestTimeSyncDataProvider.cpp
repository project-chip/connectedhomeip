/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/clusters/time-synchronization-server/TimeSyncDataProvider.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::DeviceLayer;

namespace {

void TestTrustedTimeSource(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    TimeSyncDataProvider timeSyncDataProv;
    timeSyncDataProv.Init(persistentStorage);

    NL_TEST_ASSERT(inSuite, true);
}

void TestDefaultNtp(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, true);
}

void TestTimeZone(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, true);
}

void TestTimeZoneEmpty(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, true);
}

void TestDSTOffset(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, true);
}

void TestDSTOffsetEmpty(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, true);
}

const nlTest sTests[] = { NL_TEST_DEF("Test TrustedTimeSource", TestTrustedTimeSource),
                          NL_TEST_DEF("Test default NTP", TestDefaultNtp),
                          NL_TEST_DEF("Test time zone", TestTimeZone),
                          NL_TEST_DEF("Test time zone (empty list)", TestTimeZoneEmpty),
                          NL_TEST_DEF("Test DSTOffset", TestDSTOffset),
                          NL_TEST_DEF("Test DSTOffset (empty list)", TestDSTOffsetEmpty),
                          NL_TEST_SENTINEL() };

int TestSetup(void * inContext)
{
    return SUCCESS;
}

int TestTearDown(void * inContext)
{
    return SUCCESS;
}

} // namespace

int TestTimeSyncDataProvider()
{
    nlTestSuite theSuite = { "Time Sync data provider tests", &sTests[0], TestSetup, TestTearDown };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestTimeSyncDataProvider)

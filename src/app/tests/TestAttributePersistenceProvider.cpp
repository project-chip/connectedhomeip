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
 *    @file
 *      This file implements unit tests for AttributePersistenceProvider
 *
 */

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/MessageDef/AttributeDataIB.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <app/DefaultAttributePersistenceProvider.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using namespace chip;
using namespace chip::app;
using namespace chip::TLV;

namespace {

// These values are easier to be recognized in the encoded buffer
/**
 *  Set up the test suite.
 */
int Test_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    VerifyOrReturnError(error == CHIP_NO_ERROR, FAILURE);
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int Test_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

void TestStorageAndRetrivalUint8(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider persistenceProvider;

    // todo Failure before Init

    // Init
    CHIP_ERROR err = persistenceProvider.Init(&storageDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Store bytespan
    uint8_t testArray[1] = {0x42};
    ByteSpan testValue(testArray, sizeof(testArray));
    err = persistenceProvider.WriteValue(ConcreteAttributePath(1, 1, 1), testValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    uint8_t receiveArray[1];
    MutableByteSpan receiveValue(receiveArray, sizeof(receiveArray));
    err = persistenceProvider.ReadValue(ConcreteAttributePath(1, 1, 1), 0x20, 1, receiveValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, receiveArray[0] == 8);


    // todo Store uint8
    uint8_t testValue8 = 8;
    err = persistenceProvider.WriteValue(ConcreteAttributePath(1, 1, 1), testValue8);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Finishing
    persistenceProvider.Shutdown();
}


} // anonymous namespace

namespace {
const nlTest sTests[] = { NL_TEST_DEF("Test AttributePersistenceProvider: Storage and retrival of uint8_t", TestStorageAndRetrivalUint8),
                          NL_TEST_SENTINEL() };
}

int TestAttributePersistenceProvider()
{
    nlTestSuite theSuite = { "AttributePersistenceProvider", &sTests[0], Test_Setup, Test_Teardown };

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestAttributePersistenceProvider)

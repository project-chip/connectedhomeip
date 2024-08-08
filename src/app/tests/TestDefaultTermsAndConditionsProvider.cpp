/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#define CHIP_CONFIG_TC_REQUIRED 1                          // Enable conditional feature for testing
#define CHIP_CONFIG_TC_REQUIRED_ACKNOWLEDGEMENTS 0         // Dummy define to satisfy #error check
#define CHIP_CONFIG_TC_REQUIRED_ACKNOWLEDGEMENTS_VERSION 0 // Dummy define to satisfy #error check

#include "app/server/DefaultTermsAndConditionsProvider.h"

#include <lib/core/CHIPError.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <pw_unit_test/framework.h>

TEST(DefaultTermsAndConditionsProvider, TestInitSuccess)
{
    CHIP_ERROR err;

    chip::TestPersistentStorageDelegate storageDelegate;
    chip::app::DefaultTermsAndConditionsProvider tncProvider;

    uint16_t requiredAcknowledgements        = 1;
    uint16_t requiredAcknowledgementsVersion = 1;
    err = tncProvider.Init(&storageDelegate, requiredAcknowledgements, requiredAcknowledgementsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
}

TEST(DefaultTermsAndConditionsProvider, TestNoRequirementsGetRequirementsSuccess)
{
    CHIP_ERROR err;

    chip::TestPersistentStorageDelegate storageDelegate;
    chip::app::DefaultTermsAndConditionsProvider tncProvider;

    uint16_t requiredAcknowledgements        = 0;
    uint16_t requiredAcknowledgementsVersion = 0;
    err = tncProvider.Init(&storageDelegate, requiredAcknowledgements, requiredAcknowledgementsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    uint16_t outAcceptance;
    uint16_t outAcknowledgementsVersion;
    err = tncProvider.GetAcceptance(outAcceptance, outAcknowledgementsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(0 == outAcceptance);
    EXPECT_TRUE(0 == outAcknowledgementsVersion);
}

TEST(DefaultTermsAndConditionsProvider, TestNeverAcceptanceGetAcceptanceSuccess)
{
    CHIP_ERROR err;

    chip::TestPersistentStorageDelegate storageDelegate;
    chip::app::DefaultTermsAndConditionsProvider tncProvider;

    uint16_t requiredAcknowledgements        = 0b1111'1111'1111'1111;
    uint16_t requiredAcknowledgementsVersion = 1;
    err = tncProvider.Init(&storageDelegate, requiredAcknowledgements, requiredAcknowledgementsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    uint16_t outAcceptance;
    uint16_t outAcknowledgementsVersion;
    err = tncProvider.GetAcceptance(outAcceptance, outAcknowledgementsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(0 == outAcceptance);
    EXPECT_TRUE(0 == outAcknowledgementsVersion);
}

TEST(DefaultTermsAndConditionsProvider, TestTermsAcceptedPersistsSuccess)
{
    CHIP_ERROR err;

    chip::TestPersistentStorageDelegate storageDelegate;
    chip::app::DefaultTermsAndConditionsProvider tncProvider;
    chip::app::DefaultTermsAndConditionsProvider anotherTncProvider;

    uint16_t requiredAcknowledgements        = 1;
    uint16_t requiredAcknowledgementsVersion = 1;
    err = tncProvider.Init(&storageDelegate, requiredAcknowledgements, requiredAcknowledgementsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    uint16_t acceptedTermsAndConditions        = 1;
    uint16_t acceptedTermsAndConditionsVersion = 1;
    err = tncProvider.SetAcceptance(acceptedTermsAndConditions, acceptedTermsAndConditionsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    uint16_t outAcceptance;
    uint16_t outAcknowledgementsVersion;
    err = tncProvider.GetAcceptance(outAcceptance, outAcknowledgementsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(1 == outAcceptance);
    EXPECT_TRUE(1 == outAcknowledgementsVersion);

    err = anotherTncProvider.Init(&storageDelegate, requiredAcknowledgements, requiredAcknowledgementsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    err = anotherTncProvider.GetAcceptance(outAcceptance, outAcknowledgementsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(1 == outAcceptance);
    EXPECT_TRUE(1 == outAcknowledgementsVersion);
}

TEST(DefaultTermsAndConditionsProvider, TestTermsRequiredGetRequirementsSuccess)
{
    CHIP_ERROR err;

    chip::TestPersistentStorageDelegate storageDelegate;
    chip::app::DefaultTermsAndConditionsProvider tncProvider;

    uint16_t initialRequiredAcknowledgements        = 1;
    uint16_t initialRequiredAcknowledgementsVersion = 1;
    err = tncProvider.Init(&storageDelegate, initialRequiredAcknowledgements, initialRequiredAcknowledgementsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    uint16_t outRequiredAcknowledgements;
    uint16_t outRequiredAcknowledgementsVersion;
    err = tncProvider.GetRequirements(outRequiredAcknowledgements, outRequiredAcknowledgementsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(1 == outRequiredAcknowledgements);
    EXPECT_TRUE(1 == outRequiredAcknowledgementsVersion);
}

TEST(DefaultTermsAndConditionsProvider, TestSetAcceptanceGetAcceptanceSuccess)
{
    CHIP_ERROR err;

    chip::TestPersistentStorageDelegate storageDelegate;
    chip::app::DefaultTermsAndConditionsProvider tncProvider;

    uint16_t requiredAcknowledgements        = 1;
    uint16_t requiredAcknowledgementsVersion = 1;
    err = tncProvider.Init(&storageDelegate, requiredAcknowledgements, requiredAcknowledgementsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    uint16_t acceptedTermsAndConditions        = 1;
    uint16_t acceptedTermsAndConditionsVersion = 1;
    err = tncProvider.SetAcceptance(acceptedTermsAndConditions, acceptedTermsAndConditionsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    uint16_t outAcceptance;
    uint16_t outAcknowledgementsVersion;
    err = tncProvider.GetAcceptance(outAcceptance, outAcknowledgementsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(1 == outAcceptance);
    EXPECT_TRUE(1 == outAcknowledgementsVersion);
}

TEST(DefaultTermsAndConditionsProvider, TestClearAcceptanceGetAcceptanceSuccess)
{
    CHIP_ERROR err;

    chip::TestPersistentStorageDelegate storageDelegate;
    chip::app::DefaultTermsAndConditionsProvider tncProvider;

    uint16_t requiredAcknowledgements        = 1;
    uint16_t requiredAcknowledgementsVersion = 1;
    err = tncProvider.Init(&storageDelegate, requiredAcknowledgements, requiredAcknowledgementsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    uint16_t acceptedTermsAndConditions        = 1;
    uint16_t acceptedTermsAndConditionsVersion = 1;
    err = tncProvider.SetAcceptance(acceptedTermsAndConditions, acceptedTermsAndConditionsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    uint16_t outAcceptance;
    uint16_t outAcknowledgementsVersion;
    err = tncProvider.GetAcceptance(outAcceptance, outAcknowledgementsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(1 == outAcceptance);
    EXPECT_TRUE(1 == outAcknowledgementsVersion);

    err = tncProvider.ClearAcceptance();
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    uint16_t outAcceptance2;
    uint16_t outAcknowledgementsVersion2;
    err = tncProvider.GetAcceptance(outAcceptance2, outAcknowledgementsVersion2);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(0 == outAcceptance2);
    EXPECT_TRUE(0 == outAcknowledgementsVersion2);
}

TEST(DefaultTermsAndConditionsProvider, TestAcceptanceRequiredTermsMissingFailure)
{
    CHIP_ERROR err;

    chip::TestPersistentStorageDelegate storageDelegate;
    chip::app::DefaultTermsAndConditionsProvider tncProvider;

    uint16_t requiredAcknowledgements        = 1;
    uint16_t requiredAcknowledgementsVersion = 1;
    err = tncProvider.Init(&storageDelegate, requiredAcknowledgements, requiredAcknowledgementsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    uint16_t acceptedTermsAndConditions        = 1;
    uint16_t acceptedTermsAndConditionsVersion = 1;
    err = tncProvider.SetAcceptance(acceptedTermsAndConditions, acceptedTermsAndConditionsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    uint16_t outAcceptance;
    uint16_t outAcknowledgementsVersion;
    err = tncProvider.GetAcceptance(outAcceptance, outAcknowledgementsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(1 == outAcceptance);
    EXPECT_TRUE(1 == outAcknowledgementsVersion);

    err = tncProvider.ClearAcceptance();
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    uint16_t outRequiredAcknowledgements;
    uint16_t outRequiredAcknowledgementsVersion;
    err = tncProvider.GetRequirements(outRequiredAcknowledgements, outRequiredAcknowledgementsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(1 == outRequiredAcknowledgements);
    EXPECT_TRUE(1 == outRequiredAcknowledgementsVersion);
}

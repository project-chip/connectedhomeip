/*
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

#include "app/server/DefaultEnhancedSetupFlowProvider.h"

#include <lib/core/CHIPError.h>
#include <pw_unit_test/framework.h>

class FakeTermsAndConditionsProvider : public chip::app::TermsAndConditionsProvider
{
public:
    FakeTermsAndConditionsProvider(uint16_t inAcceptedAcknowledgements, uint16_t inAcceptedAcknowledgementsVersion,
                                   uint16_t inRequiredAcknowledgements, uint16_t inRequiredAcknowledgementsVersion) :
        mAcceptedAcknowledgements(inAcceptedAcknowledgements),
        mAcceptedAcknowledgementsVersion(inAcceptedAcknowledgementsVersion), mRequiredAcknowledgements(inRequiredAcknowledgements),
        mRequiredAcknowledgementsVersion(inRequiredAcknowledgementsVersion)
    {}

    CHIP_ERROR GetAcceptance(uint16_t & outAcknowledgements, uint16_t & outAcknowledgementsVersion) const override
    {
        outAcknowledgements        = mAcceptedAcknowledgements;
        outAcknowledgementsVersion = mAcceptedAcknowledgementsVersion;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetRequirements(uint16_t & outAcknowledgements, uint16_t & outAcknowledgementsVersion) const override
    {
        outAcknowledgements        = mRequiredAcknowledgements;
        outAcknowledgementsVersion = mRequiredAcknowledgementsVersion;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetAcceptance(uint16_t inAcknowledgements, uint16_t inAcknowledgementsVersion) override
    {
        mAcceptedAcknowledgements        = inAcknowledgements;
        mAcceptedAcknowledgementsVersion = inAcknowledgementsVersion;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ClearAcceptance() override
    {
        mAcceptedAcknowledgements        = 0;
        mAcceptedAcknowledgementsVersion = 0;
        return CHIP_NO_ERROR;
    }

private:
    uint16_t mAcceptedAcknowledgements;
    uint16_t mAcceptedAcknowledgementsVersion;
    uint16_t mRequiredAcknowledgements;
    uint16_t mRequiredAcknowledgementsVersion;
};

TEST(DefaultEnhancedSetupFlowProvider, TestNoAcceptanceRequiredCheckAcknowledgementsAcceptedSuccess)
{
    CHIP_ERROR err;
    bool hasTermsBeenAccepted;

    FakeTermsAndConditionsProvider tncProvider(0, 0, 0, 0);
    chip::app::DefaultEnhancedSetupFlowProvider esfProvider;

    err = esfProvider.Init(&tncProvider);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    err = esfProvider.HasTermsAndConditionsRequiredAcknowledgementsBeenAccepted(hasTermsBeenAccepted);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(hasTermsBeenAccepted);
}

TEST(DefaultEnhancedSetupFlowProvider, TestNoAcceptanceRequiredCheckAcknowledgementsVersionAcceptedSuccess)
{
    CHIP_ERROR err;
    bool hasTermsVersionBeenAccepted;

    FakeTermsAndConditionsProvider tncProvider(0, 0, 0, 0);
    chip::app::DefaultEnhancedSetupFlowProvider esfProvider;

    err = esfProvider.Init(&tncProvider);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    err = esfProvider.HasTermsAndConditionsRequiredAcknowledgementsVersionBeenAccepted(hasTermsVersionBeenAccepted);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(hasTermsVersionBeenAccepted);
}

TEST(DefaultEnhancedSetupFlowProvider, TestAcceptanceRequiredNoTermsAcceptedCheckAcknowledgementsAcceptedFailure)
{
    CHIP_ERROR err;
    bool hasTermsBeenAccepted;

    FakeTermsAndConditionsProvider tncProvider(0, 0, 1, 1);
    chip::app::DefaultEnhancedSetupFlowProvider esfProvider;

    err = esfProvider.Init(&tncProvider);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    err = esfProvider.HasTermsAndConditionsRequiredAcknowledgementsBeenAccepted(hasTermsBeenAccepted);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(!hasTermsBeenAccepted);
}

TEST(DefaultEnhancedSetupFlowProvider,
     TestAcceptanceRequiredTermsAcceptedTermsVersionOutdatedCheckAcknowledgementsVersionAcceptedFailure)
{
    CHIP_ERROR err;
    bool hasTermsBeenAccepted;
    bool hasTermsVersionBeenAccepted;

    FakeTermsAndConditionsProvider tncProvider(0, 0, 1, 1);
    chip::app::DefaultEnhancedSetupFlowProvider esfProvider;

    err = esfProvider.Init(&tncProvider);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    err = esfProvider.SetTermsAndConditionsAcceptance(1, 0);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    err = esfProvider.HasTermsAndConditionsRequiredAcknowledgementsBeenAccepted(hasTermsBeenAccepted);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(hasTermsBeenAccepted);

    err = esfProvider.HasTermsAndConditionsRequiredAcknowledgementsVersionBeenAccepted(hasTermsVersionBeenAccepted);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(!hasTermsVersionBeenAccepted);
}

TEST(DefaultEnhancedSetupFlowProvider, TestAcceptanceRequiredTermsAcceptedFutureVersionCheckAcknowledgementsAcceptedSuccess)
{
    CHIP_ERROR err;
    bool hasTermsBeenAccepted;
    bool hasTermsVersionBeenAccepted;

    uint16_t acceptedTerms        = 1;
    uint16_t requiredTerms        = 1;
    uint16_t acceptedTermsVersion = 2;
    uint16_t requiredTermsVersion = 1;

    FakeTermsAndConditionsProvider tncProvider(acceptedTerms, acceptedTermsVersion, requiredTerms, requiredTermsVersion);
    chip::app::DefaultEnhancedSetupFlowProvider esfProvider;

    err = esfProvider.Init(&tncProvider);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    err = esfProvider.HasTermsAndConditionsRequiredAcknowledgementsBeenAccepted(hasTermsBeenAccepted);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(hasTermsBeenAccepted);

    err = esfProvider.HasTermsAndConditionsRequiredAcknowledgementsVersionBeenAccepted(hasTermsVersionBeenAccepted);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(hasTermsVersionBeenAccepted);
}

TEST(DefaultEnhancedSetupFlowProvider, TestAcceptanceRequiredTermsAcceptedSuccess)
{
    CHIP_ERROR err;
    bool hasTermsBeenAccepted;
    bool hasTermsVersionBeenAccepted;

    FakeTermsAndConditionsProvider tncProvider(0, 0, 1, 1);
    chip::app::DefaultEnhancedSetupFlowProvider esfProvider;

    err = esfProvider.Init(&tncProvider);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    err = esfProvider.SetTermsAndConditionsAcceptance(1, 1);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    err = esfProvider.HasTermsAndConditionsRequiredAcknowledgementsBeenAccepted(hasTermsBeenAccepted);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(hasTermsBeenAccepted);

    err = esfProvider.HasTermsAndConditionsRequiredAcknowledgementsVersionBeenAccepted(hasTermsVersionBeenAccepted);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(hasTermsVersionBeenAccepted);
}

TEST(DefaultEnhancedSetupFlowProvider, TestAcceptanceRequiredTermsMissingFailure)
{
    CHIP_ERROR err;
    bool hasTermsBeenAccepted;
    bool hasTermsVersionBeenAccepted;

    uint16_t acceptedTerms        = 0b0111'1111'1111'1111;
    uint16_t requiredTerms        = 0b1111'1111'1111'1111;
    uint16_t acceptedTermsVersion = 1;
    uint16_t requiredTermsVersion = 1;

    FakeTermsAndConditionsProvider tncProvider(acceptedTerms, acceptedTermsVersion, requiredTerms, requiredTermsVersion);
    chip::app::DefaultEnhancedSetupFlowProvider esfProvider;

    err = esfProvider.Init(&tncProvider);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    err = esfProvider.HasTermsAndConditionsRequiredAcknowledgementsBeenAccepted(hasTermsBeenAccepted);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(!hasTermsBeenAccepted);

    err = esfProvider.HasTermsAndConditionsRequiredAcknowledgementsVersionBeenAccepted(hasTermsVersionBeenAccepted);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(hasTermsVersionBeenAccepted);
}

TEST(DefaultEnhancedSetupFlowProvider, TestAcceptanceRequiredAllTermsAcceptedCheckAcknowledgementsAcceptedSuccess)
{
    CHIP_ERROR err;
    bool hasTermsBeenAccepted;
    bool hasTermsVersionBeenAccepted;

    uint16_t acceptedTerms        = 0b1111'1111'1111'1111;
    uint16_t requiredTerms        = 0b1111'1111'1111'1111;
    uint16_t acceptedTermsVersion = 1;
    uint16_t requiredTermsVersion = 1;

    FakeTermsAndConditionsProvider tncProvider(acceptedTerms, acceptedTermsVersion, requiredTerms, requiredTermsVersion);
    chip::app::DefaultEnhancedSetupFlowProvider esfProvider;

    err = esfProvider.Init(&tncProvider);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    err = esfProvider.HasTermsAndConditionsRequiredAcknowledgementsBeenAccepted(hasTermsBeenAccepted);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(hasTermsBeenAccepted);

    err = esfProvider.HasTermsAndConditionsRequiredAcknowledgementsVersionBeenAccepted(hasTermsVersionBeenAccepted);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(hasTermsVersionBeenAccepted);
}

TEST(DefaultEnhancedSetupFlowProvider, TestClearAcceptanceRetainsRequirements)
{
    CHIP_ERROR err;

    uint16_t initialAcceptedTermsAndConditions        = 0;
    uint16_t initialRequiredTermsAndConditions        = 0b1111'1111'1111'1111;
    uint16_t initialAcceptedTermsAndConditionsVersion = 0;
    uint16_t initialRequiredTermsAndConditionsVersion = 1;

    uint16_t outAcceptedTermsAndConditions;
    uint16_t outRequiredTermsAndConditions;
    uint16_t outAcceptedTermsAndConditionsVersion;
    uint16_t outRequiredTermsAndConditionsVersion;

    uint16_t updatedAcceptedTermsAndConditions        = 0b1111'1111'1111'1111;
    uint16_t updatedAcceptedTermsAndConditionsVersion = 1;

    FakeTermsAndConditionsProvider tncProvider(initialAcceptedTermsAndConditions, initialAcceptedTermsAndConditionsVersion,
                                               initialRequiredTermsAndConditions, initialRequiredTermsAndConditionsVersion);

    chip::app::DefaultEnhancedSetupFlowProvider esfProvider;

    err = esfProvider.Init(&tncProvider);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    err = esfProvider.SetTermsAndConditionsAcceptance(updatedAcceptedTermsAndConditions, updatedAcceptedTermsAndConditionsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    err = esfProvider.GetTermsAndConditionsRequiredAcknowledgements(outRequiredTermsAndConditions);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(outRequiredTermsAndConditions == initialRequiredTermsAndConditions);

    err = esfProvider.GetTermsAndConditionsRequiredAcknowledgementsVersion(outRequiredTermsAndConditionsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(outRequiredTermsAndConditionsVersion == initialRequiredTermsAndConditionsVersion);

    err = esfProvider.GetTermsAndConditionsAcceptedAcknowledgements(outAcceptedTermsAndConditions);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(outAcceptedTermsAndConditions == updatedAcceptedTermsAndConditions);

    err = esfProvider.GetTermsAndConditionsAcceptedAcknowledgementsVersion(outAcceptedTermsAndConditionsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(outAcceptedTermsAndConditionsVersion == updatedAcceptedTermsAndConditionsVersion);

    err = esfProvider.ClearTermsAndConditionsAcceptance();
    EXPECT_TRUE(CHIP_NO_ERROR == err);

    err = esfProvider.GetTermsAndConditionsRequiredAcknowledgements(outRequiredTermsAndConditions);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(outRequiredTermsAndConditions == initialRequiredTermsAndConditions);

    err = esfProvider.GetTermsAndConditionsRequiredAcknowledgementsVersion(outRequiredTermsAndConditionsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(outRequiredTermsAndConditionsVersion == initialRequiredTermsAndConditionsVersion);

    err = esfProvider.GetTermsAndConditionsAcceptedAcknowledgements(outAcceptedTermsAndConditions);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(outAcceptedTermsAndConditions == 0);

    err = esfProvider.GetTermsAndConditionsAcceptedAcknowledgementsVersion(outAcceptedTermsAndConditionsVersion);
    EXPECT_TRUE(CHIP_NO_ERROR == err);
    EXPECT_TRUE(outAcceptedTermsAndConditionsVersion == 0);
}

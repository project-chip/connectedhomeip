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

#include "app/server/DefaultTermsAndConditionsProvider.h"
#include "app/server/TermsAndConditionsProvider.h"
#include "pw_unit_test/framework.h"

#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <pw_unit_test/framework.h>

class TestTermsAndConditionsStorageDelegate : public chip::app::TermsAndConditionsStorageDelegate
{
public:
    TestTermsAndConditionsStorageDelegate(chip::Optional<chip::app::TermsAndConditions> & initialTermsAndConditions) :
        mTermsAndConditions(initialTermsAndConditions)
    {}

    CHIP_ERROR Delete()
    {
        mTermsAndConditions.ClearValue();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Get(chip::Optional<chip::app::TermsAndConditions> & outTermsAndConditions)
    {
        outTermsAndConditions = mTermsAndConditions;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Set(const chip::app::TermsAndConditions & inTermsAndConditions)
    {
        mTermsAndConditions = chip::Optional<chip::app::TermsAndConditions>(inTermsAndConditions);
        return CHIP_NO_ERROR;
    }

private:
    chip::Optional<chip::app::TermsAndConditions> & mTermsAndConditions;
};

TEST(DefaultTermsAndConditionsProvider, TestInitSuccess)
{
    CHIP_ERROR err;

    chip::TestPersistentStorageDelegate testPersistentStorageDelegate;
    chip::app::DefaultTermsAndConditionsStorageDelegate defaultTermsAndConditionsStorageDelegate;
    err = defaultTermsAndConditionsStorageDelegate.Init(&testPersistentStorageDelegate);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::app::DefaultTermsAndConditionsProvider defaultTermsAndConditionsProvider;
    chip::Optional<chip::app::TermsAndConditions> requiredTermsAndConditions =
        chip::Optional<chip::app::TermsAndConditions>(chip::app::TermsAndConditions(1, 1));
    err = defaultTermsAndConditionsProvider.Init(&defaultTermsAndConditionsStorageDelegate, requiredTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);
}

TEST(DefaultTermsAndConditionsProvider, TestNoRequirementsGetRequirementsSuccess)
{
    CHIP_ERROR err;

    chip::TestPersistentStorageDelegate testPersistentStorageDelegate;
    chip::app::DefaultTermsAndConditionsStorageDelegate defaultTermsAndConditionsStorageDelegate;
    err = defaultTermsAndConditionsStorageDelegate.Init(&testPersistentStorageDelegate);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::app::DefaultTermsAndConditionsProvider defaultTermsAndConditionsProvider;
    chip::Optional<chip::app::TermsAndConditions> requiredTermsAndConditions = chip::Optional<chip::app::TermsAndConditions>();
    err = defaultTermsAndConditionsProvider.Init(&defaultTermsAndConditionsStorageDelegate, requiredTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::Optional<chip::app::TermsAndConditions> outTermsAndConditions;
    err = defaultTermsAndConditionsProvider.GetAcceptance(outTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);
    EXPECT_FALSE(outTermsAndConditions.HasValue());
}

TEST(DefaultTermsAndConditionsProvider, TestNeverAcceptanceGetAcceptanceSuccess)
{
    CHIP_ERROR err;

    chip::TestPersistentStorageDelegate testPersistentStorageDelegate;
    chip::app::DefaultTermsAndConditionsStorageDelegate defaultTermsAndConditionsStorageDelegate;
    err = defaultTermsAndConditionsStorageDelegate.Init(&testPersistentStorageDelegate);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::app::DefaultTermsAndConditionsProvider defaultTermsAndConditionsProvider;
    chip::Optional<chip::app::TermsAndConditions> requiredTermsAndConditions =
        chip::Optional<chip::app::TermsAndConditions>(chip::app::TermsAndConditions(0b1111'1111'1111'1111, 1));
    err = defaultTermsAndConditionsProvider.Init(&defaultTermsAndConditionsStorageDelegate, requiredTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::Optional<chip::app::TermsAndConditions> outTermsAndConditions;
    err = defaultTermsAndConditionsProvider.GetAcceptance(outTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);
    EXPECT_FALSE(outTermsAndConditions.HasValue());
}

TEST(DefaultTermsAndConditionsProvider, TestTermsAcceptedPersistsSuccess)
{
    CHIP_ERROR err;

    chip::TestPersistentStorageDelegate testPersistentStorageDelegate;
    chip::app::DefaultTermsAndConditionsStorageDelegate defaultTermsAndConditionsStorageDelegate;
    err = defaultTermsAndConditionsStorageDelegate.Init(&testPersistentStorageDelegate);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::app::DefaultTermsAndConditionsProvider defaultTermsAndConditionsProvider;
    chip::Optional<chip::app::TermsAndConditions> requiredTermsAndConditions =
        chip::Optional<chip::app::TermsAndConditions>(chip::app::TermsAndConditions(1, 1));
    err = defaultTermsAndConditionsProvider.Init(&defaultTermsAndConditionsStorageDelegate, requiredTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::Optional<chip::app::TermsAndConditions> newTermsAndConditions =
        chip::Optional<chip::app::TermsAndConditions>(chip::app::TermsAndConditions(1, 1));

    err = defaultTermsAndConditionsProvider.SetAcceptance(newTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::Optional<chip::app::TermsAndConditions> outTermsAndConditions;
    err = defaultTermsAndConditionsProvider.GetAcceptance(outTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);
    EXPECT_EQ(1, outTermsAndConditions.Value().GetValue());
    EXPECT_EQ(1, outTermsAndConditions.Value().GetVersion());

    err = defaultTermsAndConditionsProvider.CommitAcceptance();
    err = defaultTermsAndConditionsProvider.GetAcceptance(outTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);
    EXPECT_EQ(1, outTermsAndConditions.Value().GetValue());
    EXPECT_EQ(1, outTermsAndConditions.Value().GetVersion());

    chip::app::DefaultTermsAndConditionsProvider anotherTncProvider;
    err = anotherTncProvider.Init(&defaultTermsAndConditionsStorageDelegate, requiredTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    err = anotherTncProvider.GetAcceptance(outTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);
    EXPECT_EQ(1, outTermsAndConditions.Value().GetValue());
    EXPECT_EQ(1, outTermsAndConditions.Value().GetVersion());
}

TEST(DefaultTermsAndConditionsProvider, TestTermsRequiredGetRequirementsSuccess)
{
    CHIP_ERROR err;

    chip::TestPersistentStorageDelegate testPersistentStorageDelegate;
    chip::app::DefaultTermsAndConditionsStorageDelegate defaultTermsAndConditionsStorageDelegate;
    err = defaultTermsAndConditionsStorageDelegate.Init(&testPersistentStorageDelegate);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::app::DefaultTermsAndConditionsProvider defaultTermsAndConditionsProvider;
    chip::Optional<chip::app::TermsAndConditions> requiredTermsAndConditions =
        chip::Optional<chip::app::TermsAndConditions>(chip::app::TermsAndConditions(1, 1));
    err = defaultTermsAndConditionsProvider.Init(&defaultTermsAndConditionsStorageDelegate, requiredTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::Optional<chip::app::TermsAndConditions> outTermsAndConditions;
    err = defaultTermsAndConditionsProvider.GetRequirements(outTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);
    EXPECT_EQ(1, outTermsAndConditions.Value().GetValue());
    EXPECT_EQ(1, outTermsAndConditions.Value().GetVersion());
}

TEST(DefaultTermsAndConditionsProvider, TestSetAcceptanceGetAcceptanceSuccess)
{
    CHIP_ERROR err;

    chip::TestPersistentStorageDelegate testPersistentStorageDelegate;
    chip::app::DefaultTermsAndConditionsStorageDelegate defaultTermsAndConditionsStorageDelegate;
    err = defaultTermsAndConditionsStorageDelegate.Init(&testPersistentStorageDelegate);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::app::DefaultTermsAndConditionsProvider defaultTermsAndConditionsProvider;
    chip::Optional<chip::app::TermsAndConditions> requiredTermsAndConditions =
        chip::Optional<chip::app::TermsAndConditions>(chip::app::TermsAndConditions(1, 1));
    err = defaultTermsAndConditionsProvider.Init(&defaultTermsAndConditionsStorageDelegate, requiredTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::Optional<chip::app::TermsAndConditions> acceptedTermsAndConditions =
        chip::Optional<chip::app::TermsAndConditions>(chip::app::TermsAndConditions(1, 1));
    err = defaultTermsAndConditionsProvider.SetAcceptance(acceptedTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::Optional<chip::app::TermsAndConditions> outTermsAndConditions;
    err = defaultTermsAndConditionsProvider.GetRequirements(outTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);
    EXPECT_EQ(1, outTermsAndConditions.Value().GetValue());
    EXPECT_EQ(1, outTermsAndConditions.Value().GetVersion());
}

TEST(DefaultTermsAndConditionsProvider, TestRevertAcceptanceGetAcceptanceSuccess)
{
    CHIP_ERROR err;

    chip::TestPersistentStorageDelegate testPersistentStorageDelegate;
    chip::app::DefaultTermsAndConditionsStorageDelegate defaultTermsAndConditionsStorageDelegate;
    err = defaultTermsAndConditionsStorageDelegate.Init(&testPersistentStorageDelegate);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::app::DefaultTermsAndConditionsProvider defaultTermsAndConditionsProvider;
    chip::Optional<chip::app::TermsAndConditions> requiredTermsAndConditions =
        chip::Optional<chip::app::TermsAndConditions>(chip::app::TermsAndConditions(1, 1));
    err = defaultTermsAndConditionsProvider.Init(&defaultTermsAndConditionsStorageDelegate, requiredTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::Optional<chip::app::TermsAndConditions> acceptedTermsAndConditions =
        chip::Optional<chip::app::TermsAndConditions>(chip::app::TermsAndConditions(1, 1));
    err = defaultTermsAndConditionsProvider.SetAcceptance(acceptedTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::Optional<chip::app::TermsAndConditions> outTermsAndConditions;
    err = defaultTermsAndConditionsProvider.GetRequirements(outTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);
    EXPECT_EQ(1, outTermsAndConditions.Value().GetValue());
    EXPECT_EQ(1, outTermsAndConditions.Value().GetVersion());

    err = defaultTermsAndConditionsProvider.RevertAcceptance();
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::Optional<chip::app::TermsAndConditions> outAcceptance2;
    err = defaultTermsAndConditionsProvider.GetAcceptance(outAcceptance2);
    EXPECT_EQ(CHIP_NO_ERROR, err);
    EXPECT_FALSE(outAcceptance2.HasValue());
}

TEST(DefaultTermsAndConditionsProvider, TestAcceptanceRequiredTermsMissingFailure)
{
    CHIP_ERROR err;

    chip::TestPersistentStorageDelegate testPersistentStorageDelegate;
    chip::app::DefaultTermsAndConditionsStorageDelegate defaultTermsAndConditionsStorageDelegate;
    err = defaultTermsAndConditionsStorageDelegate.Init(&testPersistentStorageDelegate);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::app::DefaultTermsAndConditionsProvider defaultTermsAndConditionsProvider;
    chip::Optional<chip::app::TermsAndConditions> requiredTermsAndConditions =
        chip::Optional<chip::app::TermsAndConditions>(chip::app::TermsAndConditions(1, 1));
    err = defaultTermsAndConditionsProvider.Init(&defaultTermsAndConditionsStorageDelegate, requiredTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::Optional<chip::app::TermsAndConditions> acceptedTermsAndConditions =
        chip::Optional<chip::app::TermsAndConditions>(chip::app::TermsAndConditions(1, 1));
    err = defaultTermsAndConditionsProvider.SetAcceptance(acceptedTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::Optional<chip::app::TermsAndConditions> outAcknowledgementTermsAndConditions;
    err = defaultTermsAndConditionsProvider.GetAcceptance(outAcknowledgementTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);
    EXPECT_EQ(1, outAcknowledgementTermsAndConditions.Value().GetValue());
    EXPECT_EQ(1, outAcknowledgementTermsAndConditions.Value().GetVersion());

    err = defaultTermsAndConditionsProvider.RevertAcceptance();
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::Optional<chip::app::TermsAndConditions> outRequiredTermsAndConditions;
    err = defaultTermsAndConditionsProvider.GetRequirements(outRequiredTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);
    EXPECT_EQ(1, outRequiredTermsAndConditions.Value().GetValue());
    EXPECT_EQ(1, outRequiredTermsAndConditions.Value().GetVersion());
}

TEST(DefaultTermsAndConditionsProvider, TestAcceptanceCommitCheckSetRevertCheckExpectCommitValue)
{
    CHIP_ERROR err;

    chip::TestPersistentStorageDelegate testPersistentStorageDelegate;
    chip::app::DefaultTermsAndConditionsStorageDelegate defaultTermsAndConditionsStorageDelegate;
    err = defaultTermsAndConditionsStorageDelegate.Init(&testPersistentStorageDelegate);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    // Initialize unit under test
    chip::app::DefaultTermsAndConditionsProvider defaultTermsAndConditionsProvider;
    chip::Optional<chip::app::TermsAndConditions> requiredTermsAndConditions =
        chip::Optional<chip::app::TermsAndConditions>(chip::app::TermsAndConditions(1, 1));
    err = defaultTermsAndConditionsProvider.Init(&defaultTermsAndConditionsStorageDelegate, requiredTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    // Set acceptance
    chip::Optional<chip::app::TermsAndConditions> acceptedTermsAndConditions =
        chip::Optional<chip::app::TermsAndConditions>(chip::app::TermsAndConditions(0b1, 1));
    err = defaultTermsAndConditionsProvider.SetAcceptance(acceptedTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    // Commit value
    err = defaultTermsAndConditionsProvider.CommitAcceptance();
    EXPECT_EQ(CHIP_NO_ERROR, err);

    // Check commit value
    chip::Optional<chip::app::TermsAndConditions> outTermsAndConditions;
    err = defaultTermsAndConditionsProvider.GetAcceptance(outTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);
    EXPECT_TRUE(outTermsAndConditions.HasValue());
    EXPECT_EQ(outTermsAndConditions.Value().GetValue(), acceptedTermsAndConditions.Value().GetValue());
    EXPECT_EQ(outTermsAndConditions.Value().GetVersion(), acceptedTermsAndConditions.Value().GetVersion());

    // Set updated value
    chip::Optional<chip::app::TermsAndConditions> updatedTermsAndConditions =
        chip::Optional<chip::app::TermsAndConditions>(chip::app::TermsAndConditions(0b11, 2));
    err = defaultTermsAndConditionsProvider.SetAcceptance(updatedTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    // Check updated value
    err = defaultTermsAndConditionsProvider.GetAcceptance(outTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);
    EXPECT_TRUE(outTermsAndConditions.HasValue());
    EXPECT_EQ(outTermsAndConditions.Value().GetValue(), updatedTermsAndConditions.Value().GetValue());
    EXPECT_EQ(outTermsAndConditions.Value().GetVersion(), updatedTermsAndConditions.Value().GetVersion());

    // Revert updated value
    err = defaultTermsAndConditionsProvider.RevertAcceptance();
    EXPECT_EQ(CHIP_NO_ERROR, err);

    // Check committed value
    err = defaultTermsAndConditionsProvider.GetAcceptance(outTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);
    EXPECT_TRUE(outTermsAndConditions.HasValue());
    EXPECT_EQ(outTermsAndConditions.Value().GetValue(), acceptedTermsAndConditions.Value().GetValue());
    EXPECT_EQ(outTermsAndConditions.Value().GetVersion(), acceptedTermsAndConditions.Value().GetVersion());
}

TEST(DefaultTermsAndConditionsProvider, TestRevertAcceptanceWhileMissing)
{
    CHIP_ERROR err;

    chip::TestPersistentStorageDelegate testPersistentStorageDelegate;
    chip::app::DefaultTermsAndConditionsStorageDelegate defaultTermsAndConditionsStorageDelegate;
    chip::app::DefaultTermsAndConditionsProvider defaultTermsAndConditionsProvider;

    chip::Optional<chip::app::TermsAndConditions> requiredTermsAndConditions =
        chip::Optional<chip::app::TermsAndConditions>(chip::app::TermsAndConditions(1, 1));

    chip::Optional<chip::app::TermsAndConditions> outTermsAndConditions;

    err = defaultTermsAndConditionsStorageDelegate.Init(&testPersistentStorageDelegate);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    // Initialize unit under test [No conditions previously accepted]
    err = defaultTermsAndConditionsProvider.Init(&defaultTermsAndConditionsStorageDelegate, requiredTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    // [Fail-safe started] No conditions set during the fail-safe. No commit.
    err = defaultTermsAndConditionsProvider.GetAcceptance(outTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);
    EXPECT_FALSE(outTermsAndConditions.HasValue());

    // [Fail-safe expires] Revert is called.
    err = defaultTermsAndConditionsProvider.RevertAcceptance();
    EXPECT_EQ(CHIP_NO_ERROR, err);

    // [New fail safe started (to retry the commissioning operations)] Confirm acceptance returns previous values (empty)
    err = defaultTermsAndConditionsProvider.GetAcceptance(outTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);
    EXPECT_FALSE(outTermsAndConditions.HasValue());
}

TEST(DefaultTermsAndConditionsProvider, TestRevertAcceptanceWhenPreviouslyAccepted)
{
    CHIP_ERROR err;

    // Initialize unit under test [Conditions previously accepted]
    chip::Optional<chip::app::TermsAndConditions> initialTermsAndConditions =
        chip::Optional<chip::app::TermsAndConditions>(chip::app::TermsAndConditions(1, 1));
    chip::Optional<chip::app::TermsAndConditions> requiredTermsAndConditions =
        chip::Optional<chip::app::TermsAndConditions>(chip::app::TermsAndConditions(0b11, 2));
    TestTermsAndConditionsStorageDelegate testTermsAndConditionsStorageDelegate(initialTermsAndConditions);
    chip::app::DefaultTermsAndConditionsProvider defaultTermsAndConditionsProvider;
    err = defaultTermsAndConditionsProvider.Init(&testTermsAndConditionsStorageDelegate, requiredTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    // [Fail-safe started] No conditions set during the fail-safe. No commit.

    // [Fail-safe expires] Revert is called.
    err = defaultTermsAndConditionsProvider.RevertAcceptance();
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::Optional<chip::app::TermsAndConditions> outTermsAndConditions;

    // [New fail safe started (to retry the commissioning operations)] Confirm acceptance returns previous values (accepted)
    err = defaultTermsAndConditionsProvider.GetAcceptance(outTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);
    EXPECT_TRUE(outTermsAndConditions.HasValue());
    EXPECT_EQ(outTermsAndConditions.Value().GetValue(), 1);
    EXPECT_EQ(outTermsAndConditions.Value().GetVersion(), 1);
}

TEST(DefaultTermsAndConditionsProvider, TestRevertAcceptanceWhenPreviouslyAcceptedThenUpdatedUnderFailsafe)
{
    CHIP_ERROR err;

    // Initialize unit under test dependency
    chip::Optional<chip::app::TermsAndConditions> initiallyAcceptedTermsAndConditions =
        chip::Optional<chip::app::TermsAndConditions>(chip::app::TermsAndConditions(1, 1));
    TestTermsAndConditionsStorageDelegate testTermsAndConditionsStorageDelegate(initiallyAcceptedTermsAndConditions);

    // Initialize unit under test [Conditions previously accepted]
    chip::Optional<chip::app::TermsAndConditions> requiredTermsAndConditions =
        chip::Optional<chip::app::TermsAndConditions>(chip::app::TermsAndConditions(0b11, 2));
    chip::app::DefaultTermsAndConditionsProvider defaultTermsAndConditionsProvider;
    err = defaultTermsAndConditionsProvider.Init(&testTermsAndConditionsStorageDelegate, requiredTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    // [Fail-safe started] Acceptance updated.
    chip::Optional<chip::app::TermsAndConditions> updatedAcceptedTermsAndConditions =
        chip::Optional<chip::app::TermsAndConditions>(chip::app::TermsAndConditions(0b111, 3));
    err = defaultTermsAndConditionsProvider.SetAcceptance(updatedAcceptedTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    // [Fail-safe expires] Revert is called.
    err = defaultTermsAndConditionsProvider.RevertAcceptance();
    EXPECT_EQ(CHIP_NO_ERROR, err);

    chip::Optional<chip::app::TermsAndConditions> outTermsAndConditions;

    // [New fail safe started (to retry the commissioning operations)] Confirm acceptance returns previous values (accepted)
    err = defaultTermsAndConditionsProvider.GetAcceptance(outTermsAndConditions);
    EXPECT_EQ(CHIP_NO_ERROR, err);
    EXPECT_TRUE(outTermsAndConditions.HasValue());
    EXPECT_EQ(outTermsAndConditions.Value().GetValue(), initiallyAcceptedTermsAndConditions.Value().GetValue());
    EXPECT_EQ(outTermsAndConditions.Value().GetVersion(), initiallyAcceptedTermsAndConditions.Value().GetVersion());
}

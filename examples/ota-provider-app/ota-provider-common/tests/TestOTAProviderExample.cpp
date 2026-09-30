/*
 *    Copyright (c) 2026 Project CHIP Authors
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

// Unit tests for OTAProviderExample's QueryImageStatus persistence behaviour
// (the --persistQueryImageStatus option). The transition applied after each
// QueryImageResponse is isolated in ApplyQueryImageStatusAfterResponse(), so it
// can be verified here without a running server or a BDX transfer.

#include <ota-provider-common/OTAProviderExample.h>
#include <pw_unit_test/framework.h>

using chip::app::Clusters::OtaSoftwareUpdateProvider::OTAQueryStatus;

namespace {

// Default (persistence off): the configured status is used once, then reverts to
// UpdateAvailable and the DelayedActionTime is cleared for subsequent queries.
TEST(TestOTAProviderExample, DefaultRevertsToUpdateAvailableAfterResponse)
{
    OTAProviderExample provider;
    provider.SetQueryImageStatus(OTAQueryStatus::kBusy);
    provider.SetDelayedQueryActionTimeSec(60);

    provider.ApplyQueryImageStatusAfterResponse();

    EXPECT_EQ(provider.GetQueryImageStatus(), OTAQueryStatus::kUpdateAvailable);
    EXPECT_EQ(provider.GetDelayedQueryActionTimeSec(), 0u);
}

TEST(TestOTAProviderExample, DefaultRevertsFromNotAvailable)
{
    OTAProviderExample provider;
    provider.SetQueryImageStatus(OTAQueryStatus::kNotAvailable);

    provider.ApplyQueryImageStatusAfterResponse();

    EXPECT_EQ(provider.GetQueryImageStatus(), OTAQueryStatus::kUpdateAvailable);
}

// Once reverted, further responses stay UpdateAvailable.
TEST(TestOTAProviderExample, DefaultStaysUpdateAvailableAcrossResponses)
{
    OTAProviderExample provider;
    provider.SetQueryImageStatus(OTAQueryStatus::kBusy);

    provider.ApplyQueryImageStatusAfterResponse();
    provider.ApplyQueryImageStatusAfterResponse();

    EXPECT_EQ(provider.GetQueryImageStatus(), OTAQueryStatus::kUpdateAvailable);
}

// Persistence on: Busy (and its DelayedActionTime) is kept for every response.
TEST(TestOTAProviderExample, PersistKeepsBusyAndDelayedActionTime)
{
    OTAProviderExample provider;
    provider.SetPersistQueryImageStatus(true);
    provider.SetQueryImageStatus(OTAQueryStatus::kBusy);
    provider.SetDelayedQueryActionTimeSec(60);

    // Repeated responses must not change the configured status or delay.
    for (int i = 0; i < 3; i++)
    {
        provider.ApplyQueryImageStatusAfterResponse();
        EXPECT_EQ(provider.GetQueryImageStatus(), OTAQueryStatus::kBusy);
        EXPECT_EQ(provider.GetDelayedQueryActionTimeSec(), 60u);
    }
}

// Persistence on: NotAvailable is kept for every response.
TEST(TestOTAProviderExample, PersistKeepsNotAvailable)
{
    OTAProviderExample provider;
    provider.SetPersistQueryImageStatus(true);
    provider.SetQueryImageStatus(OTAQueryStatus::kNotAvailable);

    provider.ApplyQueryImageStatusAfterResponse();
    provider.ApplyQueryImageStatusAfterResponse();

    EXPECT_EQ(provider.GetQueryImageStatus(), OTAQueryStatus::kNotAvailable);
}

// Persistence on with UpdateAvailable is a no-op (already UpdateAvailable).
TEST(TestOTAProviderExample, PersistKeepsUpdateAvailable)
{
    OTAProviderExample provider;
    provider.SetPersistQueryImageStatus(true);
    provider.SetQueryImageStatus(OTAQueryStatus::kUpdateAvailable);

    provider.ApplyQueryImageStatusAfterResponse();

    EXPECT_EQ(provider.GetQueryImageStatus(), OTAQueryStatus::kUpdateAvailable);
}

} // namespace

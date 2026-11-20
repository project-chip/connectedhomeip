/*
 *
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

// DeviceCommissioner::CleanupCommissioning() when a commissioning is cancelled (StopPairing()).

#include <pw_unit_test/framework.h>

#include <app/DeviceProxy.h>
#include <controller/CHIPDeviceController.h>
#include <controller/CommissioningDelegate.h>
#include <controller/tests/DeviceCommissionerTestAccess.h>
#include <lib/core/StringBuilderAdapters.h>

using namespace chip;
using namespace chip::Controller;
using namespace chip::Testing;

namespace {

constexpr NodeId kNodeId = 0x1234;

// A device whose secure session is gone, e.g. evicted after a network change.
class SessionlessDeviceProxy : public DeviceProxy
{
public:
    void Disconnect() override {}
    NodeId GetDeviceId() const override { return kNodeId; }
    Messaging::ExchangeManager * GetExchangeManager() const override { return nullptr; }
    Optional<SessionHandle> GetSecureSession() const override { return NullOptional; }

private:
    bool IsSecureConnected() const override { return false; }
};

class RecordingCommissioningDelegate : public CommissioningDelegate
{
public:
    CHIP_ERROR SetCommissioningParameters(const CommissioningParameters & params) override
    {
        mParams = params;
        return CHIP_NO_ERROR;
    }
    const CommissioningParameters & GetCommissioningParameters() const override { return mParams; }
    void SetOperationalCredentialsDelegate(OperationalCredentialsDelegate *) override {}
    CHIP_ERROR StartCommissioning(DeviceCommissioner *, CommissioneeDeviceProxy *) override { return CHIP_ERROR_NOT_IMPLEMENTED; }

    CHIP_ERROR CommissioningStepFinished(CHIP_ERROR err, CommissioningReport report) override
    {
        mCompletions++;
        mLastError = err;
        mLastStage = report.stageCompleted;
        return CHIP_NO_ERROR;
    }

    int mCompletions              = 0;
    CHIP_ERROR mLastError         = CHIP_ERROR_INTERNAL;
    CommissioningStage mLastStage = kError;

private:
    CommissioningParameters mParams;
};

TEST(TestCommissioningCleanup, CancelledCleanupWithoutSessionResetsState)
{
    DeviceCommissioner commissioner;
    DeviceCommissionerTestAccess access(&commissioner);
    SessionlessDeviceProxy device;
    RecordingCommissioningDelegate delegate;

    access.SetCommissioningDelegate(&delegate);
    access.SetDeviceBeingCommissioned(&device);
    access.SetCommissioningStage(kCleanup);

    CompletionStatus status;
    status.err = CHIP_ERROR_CANCELLED;

    // There is no session to send the fail-safe disarm on. This must not abort, and the commissioner
    // must still finish the cleanup stage and let go of the device.
    access.CleanupCommissioning(&device, kNodeId, status);

    EXPECT_EQ(access.GetDeviceBeingCommissioned(), nullptr);
    EXPECT_EQ(delegate.mCompletions, 1);
    EXPECT_EQ(delegate.mLastError, CHIP_NO_ERROR);
    EXPECT_EQ(delegate.mLastStage, kCleanup);
}

} // namespace

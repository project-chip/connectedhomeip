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
#include <app/clusters/commissioning-proxy-server/CommissioningProxyScanAggregator.h>

#include <clusters/CommissioningProxy/Commands.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

namespace {
// Fallback so a sub-scan whose completion callback never fires cannot wedge the
// aggregator (and thus every future ProxyScanRequest) permanently.
constexpr uint16_t kScanWatchdogMarginSecs = 5;
} // namespace

void CommissioningProxyScanAggregator::Begin(app::CommandHandler * commandObj, const app::ConcreteCommandPath & path,
                                             uint8_t scanMaxTime)
{
    mHandle      = app::CommandHandler::Handle(commandObj);
    mPath        = path;
    mExpected    = 0;
    mReported    = 0;
    mScanMaxTime = scanMaxTime;
    mAddressStore.clear();
    mExtStore.clear();
    mResults.clear();
    mInProgress = true;

    CHIP_ERROR err = DeviceLayer::SystemLayer().StartTimer(
        System::Clock::Seconds16(static_cast<uint16_t>(scanMaxTime) + kScanWatchdogMarginSecs), WatchdogCallback, this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "CommissioningProxy: failed to arm scan watchdog: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void CommissioningProxyScanAggregator::AddPendingContributor()
{
    ++mExpected;
}

void CommissioningProxyScanAggregator::WatchdogCallback(System::Layer * /*layer*/, void * appState)
{
    auto * self = static_cast<CommissioningProxyScanAggregator *>(appState);
    if (self->mInProgress)
    {
        ChipLogError(Zcl, "CommissioningProxy: scan watchdog fired (a sub-scan never completed); emitting partial results");
        self->EmitCombinedResponse();
    }
}

void CommissioningProxyScanAggregator::EmitCombinedResponse()
{
    DeviceLayer::SystemLayer().CancelTimer(WatchdogCallback, this);

    if (app::CommandHandler * cmd = mHandle.Get())
    {
        Commands::ProxyScanResponse::Type response;
        response.proxyScanResult =
            DataModel::List<const ScanResultEntry>(Span<const ScanResultEntry>(mResults.data(), mResults.size()));
        response.numberOfResults = static_cast<uint8_t>(mResults.size());
        cmd->AddResponse(mPath, response);
        ChipLogProgress(Zcl, "CommissioningProxy: combined scan complete, %u result(s)", static_cast<unsigned>(mResults.size()));
    }

    mHandle.Release();
    mAddressStore.clear();
    mExtStore.clear();
    mResults.clear();
    mExpected   = 0;
    mReported   = 0;
    mInProgress = false;
}

void CommissioningProxyScanAggregator::Contribute(Span<const ScanResultEntry> results)
{
    if (!mInProgress)
    {
        ChipLogError(Zcl, "CommissioningProxy: no aggregate scan active; dropping %u result(s)",
                     static_cast<unsigned>(results.size()));
        return;
    }

    for (const auto & e : results)
    {
        // Copy scalar fields, then rebind the address / extendedData spans to point
        // into the aggregator's stable storage so they survive until AddResponse.
        // std::deque keeps element addresses stable across growth.
        ScanResultEntry r = e;
        if (!e.address.IsNull())
        {
            auto span = e.address.Value();
            mAddressStore.emplace_back(span.data(), span.data() + span.size());
            auto & addr = mAddressStore.back();
            r.address.SetNonNull(ByteSpan(addr.data(), addr.size()));
        }
        if (!e.extendedData.IsNull())
        {
            auto span = e.extendedData.Value();
            mExtStore.emplace_back(span.data(), span.data() + span.size());
            auto & ext = mExtStore.back();
            r.extendedData.SetNonNull(ByteSpan(ext.data(), ext.size()));
        }
        mResults.push_back(r);
    }

    // Always count the report. A transport that reports synchronously from within
    // Scan() (e.g. a test double) contributes before the cluster has finished
    // registering all contributors, so emit is gated on the count being known and
    // reached — either here (async transports) or via MaybeEmitIfComplete() once the
    // cluster has started every sub-scan.
    ++mReported;
    if (mExpected > 0 && mReported >= mExpected)
    {
        EmitCombinedResponse();
    }
}

void CommissioningProxyScanAggregator::MaybeEmitIfComplete()
{
    if (mInProgress && mExpected > 0 && mReported >= mExpected)
    {
        EmitCombinedResponse();
    }
}

void CommissioningProxyScanAggregator::Abort()
{
    if (!mInProgress)
    {
        return;
    }
    DeviceLayer::SystemLayer().CancelTimer(WatchdogCallback, this);
    mHandle.Release();
    mAddressStore.clear();
    mExtStore.clear();
    mResults.clear();
    mExpected   = 0;
    mReported   = 0;
    mInProgress = false;
}

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip

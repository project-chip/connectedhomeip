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
#include <system/SystemClock.h>

#include <algorithm>
#include <cstring>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

namespace {
// Fallback so a sub-scan whose completion callback never fires cannot wedge the
// aggregator (and thus every future ProxyScanRequest) permanently.
constexpr uint16_t kScanWatchdogMarginSecs = 5;
} // namespace

CHIP_ERROR CommissioningProxyScanAggregator::Begin(app::CommandHandler * commandObj, const app::ConcreteCommandPath & path,
                                                   uint8_t scanMaxTime)
{
    mHandle                    = app::CommandHandler::Handle(commandObj);
    mPath                      = path;
    mExpected                  = 0;
    mReported                  = 0;
    mScanMaxTime               = scanMaxTime;
    mResultCount               = 0;
    mInProgress                = true;
    mAllContributorsRegistered = false;

    CHIP_ERROR err =
        mTimerDelegate.StartTimer(this, System::Clock::Seconds16(static_cast<uint16_t>(scanMaxTime) + kScanWatchdogMarginSecs));
    if (err != CHIP_NO_ERROR)
    {
        // With no watchdog a sub-scan that never reports would leave mInProgress set
        // forever, making every later ProxyScanRequest return Busy.
        ChipLogError(Zcl, "CommissioningProxy: failed to arm scan watchdog: %" CHIP_ERROR_FORMAT, err.Format());
        Abort();
        return err;
    }

    return CHIP_NO_ERROR;
}

void CommissioningProxyScanAggregator::AddPendingContributor()
{
    ++mExpected;
}

void CommissioningProxyScanAggregator::TimerFired()
{
    if (mInProgress)
    {
        ChipLogError(Zcl, "CommissioningProxy: scan watchdog fired (a sub-scan never completed); emitting partial results");
        EmitCombinedResponse();
    }
}

void CommissioningProxyScanAggregator::EmitCombinedResponse()
{
    mTimerDelegate.CancelTimer(this);

    if (app::CommandHandler * cmd = mHandle.Get())
    {
        Commands::ProxyScanResponse::Type response;
        response.proxyScanResult = DataModel::List<const ScanResultEntry>(Span<const ScanResultEntry>(mResults, mResultCount));
        response.numberOfResults = mResultCount;
        cmd->AddResponse(mPath, response);
        ChipLogProgress(Zcl, "CommissioningProxy: combined scan complete, %u result(s)", mResultCount);
    }

    mHandle.Release();
    mResultCount               = 0;
    mExpected                  = 0;
    mReported                  = 0;
    mInProgress                = false;
    mAllContributorsRegistered = false;
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
        // Spec: ProxyScanResult is "max MaxCachedResults" entries.
        if (mResultCount >= kMaxResults)
        {
            ChipLogError(Zcl, "CommissioningProxy: scan result list full; dropping remaining results");
            break;
        }

        // Copy scalar fields, then rebind the address / extendedData spans to point at
        // this aggregator's own storage so they survive until AddResponse.
        ResultStore & store = mStore[mResultCount];
        ScanResultEntry r   = e;
        if (!e.address.IsNull())
        {
            auto span            = e.address.Value();
            const size_t copyLen = std::min(span.size(), kMaxAddressBytes);
            memcpy(store.address, span.data(), copyLen);
            store.addressLen = static_cast<uint8_t>(copyLen);
            r.address.SetNonNull(ByteSpan(store.address, store.addressLen));
        }
        if (!e.extendedData.IsNull())
        {
            auto span            = e.extendedData.Value();
            const size_t copyLen = std::min(span.size(), kMaxExtendedDataBytes);
            memcpy(store.extendedData, span.data(), copyLen);
            store.extendedDataLen = static_cast<uint8_t>(copyLen);
            r.extendedData.SetNonNull(ByteSpan(store.extendedData, store.extendedDataLen));
        }
        mResults[mResultCount] = r;
        mResultCount++;
    }

    // Always count the report. A transport that reports synchronously from within Scan()
    // contributes while mExpected is still growing, so emitting here would drop every
    // later transport's results; MaybeEmitIfComplete() finishes those aggregations once
    // the cluster has started all of the sub-scans.
    ++mReported;
    if (mAllContributorsRegistered && mExpected > 0 && mReported >= mExpected)
    {
        EmitCombinedResponse();
    }
}

void CommissioningProxyScanAggregator::MaybeEmitIfComplete()
{
    // The cluster has started every requested sub-scan, so mExpected is final now.
    mAllContributorsRegistered = true;
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
    mTimerDelegate.CancelTimer(this);
    mHandle.Release();
    mResultCount               = 0;
    mExpected                  = 0;
    mReported                  = 0;
    mInProgress                = false;
    mAllContributorsRegistered = false;
}

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip

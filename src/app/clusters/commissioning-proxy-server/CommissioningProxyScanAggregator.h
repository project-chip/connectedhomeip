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

#pragma once

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <clusters/CommissioningProxy/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <system/SystemLayer.h>

#include <cstdint>
#include <deque>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

/**
 * @brief Aggregates a multi-transport foreground ProxyScanRequest into one response.
 *
 * A ProxyScanRequest MAY select more than one transport. The cluster starts each
 * requested transport's scan in parallel, hands this aggregator the command handle
 * and the number of started sub-scans, and each transport reports its results via
 * Contribute() when its sub-scan finishes. Once every started sub-scan has reported
 * (or a watchdog fires), the aggregator emits the single combined ProxyScanResponse.
 *
 * Only one aggregation may be in flight at a time (matches a single outstanding
 * foreground scan). Entry points run on the Matter thread with the stack lock held.
 */
class CommissioningProxyScanAggregator
{
public:
    using ScanResultEntry = Structs::ScanResultStruct::Type;

    CommissioningProxyScanAggregator()  = default;
    ~CommissioningProxyScanAggregator() = default;

    /**
     * @brief Start a fresh aggregation. Takes ownership of the command handle for
     *        the combined response and arms a watchdog bounded by @p scanMaxTime
     *        seconds. The caller must have checked InProgress() == false first.
     *
     * The number of contributors is not known up front: the caller starts each
     * requested transport's scan and calls AddPendingContributor() for every one
     * that starts successfully, then Commit() (or Abort() if none started).
     */
    void Begin(app::CommandHandler * commandObj, const app::ConcreteCommandPath & path, uint8_t scanMaxTime);

    /// Register one successfully-started sub-scan (increments the expected count).
    void AddPendingContributor();

    /// Number of sub-scans registered via AddPendingContributor().
    uint8_t PendingContributors() const { return mExpected; }

    /**
     * @brief Report one transport's sub-scan results (empty span = found nothing).
     *        Entries are deep-copied, so the caller's backing storage need not
     *        outlive the call. Emits the combined ProxyScanResponse once every
     *        registered contributor has reported.
     */
    void Contribute(Span<const ScanResultEntry> results);

    /// Emit the combined response if every registered contributor has already
    /// reported. Call once after starting all sub-scans, to cover transports that
    /// contribute synchronously from within Scan() (real transports report later,
    /// via Contribute()).
    void MaybeEmitIfComplete();

    /// True while an aggregation is in flight.
    bool InProgress() const { return mInProgress; }

    /// Abandon the in-flight aggregation without sending a response (no sub-scan
    /// could be started, or cluster teardown).
    void Abort();

    /// Abandon any in-flight aggregation (cluster teardown).
    void Shutdown() { Abort(); }

private:
    static void WatchdogCallback(System::Layer * layer, void * appState);
    void EmitCombinedResponse();

    app::CommandHandler::Handle mHandle;
    app::ConcreteCommandPath mPath;
    bool mInProgress     = false;
    uint8_t mExpected    = 0;
    uint8_t mReported    = 0;
    uint8_t mScanMaxTime = 0;
    std::deque<std::vector<uint8_t>> mAddressStore; // keeps ByteSpan backing alive until emit
    std::deque<std::vector<uint8_t>> mExtStore;
    std::vector<ScanResultEntry> mResults;
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip

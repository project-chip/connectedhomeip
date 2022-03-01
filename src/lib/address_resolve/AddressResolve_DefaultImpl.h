/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/address_resolve/AddressResolve.h>
#include <lib/dnssd/Resolver.h>
#include <system/TimeSource.h>
#include <transport/raw/PeerAddress.h>

namespace chip {
namespace AddressResolve {
namespace Impl {

/// Action to take when some resolve data
/// has been received by an active lookup
enum class NodeLookupAction
{
    kKeepSearching, // Keep the lookup alive
    kStopSearching, // Lookup complete
};

/// An implementation of a node lookup handle
///
/// Keeps track of time requests as well as the current
/// "best" IP address found.
class NodeLookupHandle : public NodeLookupHandleBase
{
public:
    const NodeLookupRequest & GetRequest() const { return mRequest; }

    /// Sets up a request for a new lookup.
    /// Resets internal state (i.e. best address so far)
    void ResetForLookup(System::Clock::Timestamp now, const NodeLookupRequest & request);

    /// Mark that a specific IP address has been found
    void LookupResult(const ResolveResult & result);

    /// Called after timeouts or after a series of IP addresses have been
    /// marked as found.
    ///
    /// If sufficient data for a complete address resolution has been gathered,
    /// calls the underlying listener `OnNodeAddressResolved` and returns
    /// kStopSearching.
    ///
    /// Returns kKeepSearching if more data is acceptable (keep timeouts and
    /// any active searches)
    NodeLookupAction NextAction(System::Clock::Timestamp now);

    /// Return when the next timer (min or max lookup time) is required to
    /// be triggered for this lookup handle
    System::Clock::Timeout NextEventTimeout(System::Clock::Timestamp now);

private:
    System::Clock::Timestamp mRequestStartTime;
    NodeLookupRequest mRequest; // active request to process
    AddressResolve::ResolveResult mBestResult;
    unsigned mBestAddressScore = 0;
};

class Resolver : public ::chip::AddressResolve::Resolver, public Dnssd::OperationalResolveDelegate
{
public:
    virtual ~Resolver() = default;

    // AddressResolve::Resolver

    CHIP_ERROR Init(System::Layer * systemLayer) override;
    CHIP_ERROR LookupNode(const NodeLookupRequest & request, Impl::NodeLookupHandle & handle) override;

    // Dnssd::OperationalResolveDelegate

    void OnOperationalNodeResolved(const Dnssd::ResolvedNodeData & nodeData) override;
    void OnOperationalNodeResolutionFailed(const PeerId & peerId, CHIP_ERROR error) override;

private:
    static void OnResolveTimer(System::Layer * layer, void * context) { static_cast<Resolver *>(context)->HandleTimer(); }

    /// Timer on lookup node events: min and max search times.
    void HandleTimer();

    /// Sets up a system timer to the next closest timeout on one of the active
    /// lookup operations.
    ///
    /// Any existing timer is cancelled and then OnResolveTimer will be called
    /// on the closest event required for an active resolve.
    void ReArmTimer();

    System::Layer * mSystemLayer = nullptr;
    Time::TimeSource<Time::Source::kSystem> mTimeSource;
    IntrusiveList<NodeLookupHandle> mActiveLookups;
};

} // namespace Impl
} // namespace AddressResolve
} // namespace chip

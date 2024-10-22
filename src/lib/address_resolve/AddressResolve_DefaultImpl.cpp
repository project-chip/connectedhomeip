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

#include <lib/address_resolve/AddressResolve_DefaultImpl.h>

#include <lib/address_resolve/TracingStructs.h>
#include <tracing/macros.h>

namespace chip {
namespace AddressResolve {
namespace Impl {
namespace {

static constexpr System::Clock::Timeout kInvalidTimeout{ System::Clock::Timeout::max() };

} // namespace

void NodeLookupHandle::ResetForLookup(System::Clock::Timestamp now, const NodeLookupRequest & request)
{
    mRequestStartTime = now;
    mRequest          = request;
    mResults          = NodeLookupResults();
}

void NodeLookupHandle::LookupResult(const ResolveResult & result)
{
    MATTER_LOG_NODE_DISCOVERED(Tracing::DiscoveryInfoType::kIntermediateResult, &GetRequest().GetPeerId(), &result);

    auto score = Dnssd::IPAddressSorter::ScoreIpAddress(result.address.GetIPAddress(), result.address.GetInterface());
    [[maybe_unused]] bool success = mResults.UpdateResults(result, score);

#if CHIP_PROGRESS_LOGGING
    char addr_string[Transport::PeerAddress::kMaxToStringSize];
    result.address.ToString(addr_string);

    const PeerId peerId = GetRequest().GetPeerId();
    if (success)
    {
        ChipLogProgress(Discovery, "%s: new best score: %u (for " ChipLogFormatPeerId ")", addr_string, to_underlying(score),
                        ChipLogValuePeerId(peerId));
    }
    else
    {
        ChipLogProgress(Discovery, "%s: score has not improved: %u (for " ChipLogFormatPeerId ")", addr_string,
                        to_underlying(score), ChipLogValuePeerId(peerId));
    }
#endif
}

System::Clock::Timeout NodeLookupHandle::NextEventTimeout(System::Clock::Timestamp now)
{
    const System::Clock::Timestamp elapsed = now - mRequestStartTime;

    if (elapsed < mRequest.GetMinLookupTime())
    {
        return mRequest.GetMinLookupTime() - elapsed;
    }

    if (HasLookupResult())
    {
        // We can get here if we got our result before our min lookup time had
        // elapsed, but time has passed between then and this attempt to re-arm
        // the timer, such that now we are past our min lookup time.  For
        // example, this can happen because the timer is a bit delayed in firing
        // but is now being re-scheduled due to a cancellation of a lookup or
        // start of a new lookup.  Or it could happen because
        // OnOperationalNodeResolved got called close to our min lookup time,
        // and we crossed that line while going through mActiveLookups and
        // before we got to calling ReArmTimer.
        //
        // In this case, we should just fire the timer ASAP, since our min
        // lookup time has elapsed and we have results.
        return System::Clock::Timeout::zero();
    }

    if (elapsed < mRequest.GetMaxLookupTime())
    {
        return mRequest.GetMaxLookupTime() - elapsed;
    }

    ChipLogError(Discovery, "Unexpected timeout: lookup should have been cleaned already.");
    return System::Clock::Timeout::zero();
}

NodeLookupAction NodeLookupHandle::NextAction(System::Clock::Timestamp now)
{
    const System::Clock::Timestamp elapsed = now - mRequestStartTime;

    ChipLogProgress(Discovery, "Checking node lookup status for " ChipLogFormatPeerId " after %lu ms",
                    ChipLogValuePeerId(mRequest.GetPeerId()), static_cast<unsigned long>(elapsed.count()));

    // We are still within the minimal search time. Wait for more results.
    if (elapsed < mRequest.GetMinLookupTime())
    {
        ChipLogProgress(Discovery, "Keeping DNSSD lookup active");
        return NodeLookupAction::KeepSearching();
    }

    // Minimal time to search reached. If any IP available, ready to return it.
    if (HasLookupResult())
    {
        auto result = TakeLookupResult();
        return NodeLookupAction::Success(result);
    }

    // Give up if the maximum search time has been reached
    if (elapsed >= mRequest.GetMaxLookupTime())
    {
        return NodeLookupAction::Error(CHIP_ERROR_TIMEOUT);
    }

    return NodeLookupAction::KeepSearching();
}

bool NodeLookupResults::UpdateResults(const ResolveResult & result, const Dnssd::IPAddressSorter::IpScore newScore)
{
    uint8_t insertAtIndex = 0;
    for (; insertAtIndex < kNodeLookupResultsLen; insertAtIndex++)
    {
        if (insertAtIndex >= count)
        {
            // This is a new entry.
            break;
        }

        auto & oldAddress = results[insertAtIndex].address;
        auto oldScore     = Dnssd::IPAddressSorter::ScoreIpAddress(oldAddress.GetIPAddress(), oldAddress.GetInterface());
        if (newScore > oldScore)
        {
            // This is a score update, it will replace a previous entry.
            break;
        }
    }

    if (insertAtIndex == kNodeLookupResultsLen)
    {
        return false;
    }

    // Move the following valid entries one level down.
    for (auto i = count; i > insertAtIndex; i--)
    {
        if (i >= kNodeLookupResultsLen)
        {
            continue;
        }

        results[i] = results[i - 1];
    }

    // If the number of valid entries is less than the size of the array there is an additional entry.
    if (count < kNodeLookupResultsLen)
    {
        count++;
    }

    auto & updatedResult = results[insertAtIndex];
    updatedResult        = result;
    if (!updatedResult.address.GetIPAddress().IsIPv6LinkLocal())
    {
        // Only use the DNS-SD resolution's InterfaceID for addresses that are IPv6 LLA.
        // For all other addresses, we should rely on the device's routing table to route messages sent.
        // Forcing messages down an InterfaceId might fail. For example, in bridged networks like Thread,
        // mDNS advertisements are not usually received on the same interface the peer is reachable on.
        updatedResult.address.SetInterface(Inet::InterfaceId::Null());
        ChipLogDetail(Discovery, "Lookup clearing interface for non LL address");
    }

    return true;
}

CHIP_ERROR Resolver::LookupNode(const NodeLookupRequest & request, Impl::NodeLookupHandle & handle)
{
    MATTER_LOG_NODE_LOOKUP(&request);

    VerifyOrReturnError(mSystemLayer != nullptr, CHIP_ERROR_INCORRECT_STATE);

    handle.ResetForLookup(mTimeSource.GetMonotonicTimestamp(), request);
    auto & peerId = request.GetPeerId();
    ReturnErrorOnFailure(Dnssd::Resolver::Instance().ResolveNodeId(peerId));
    mActiveLookups.PushBack(&handle);
    ReArmTimer();
    ChipLogProgress(Discovery, "Lookup started for " ChipLogFormatPeerId, ChipLogValuePeerId(peerId));
    return CHIP_NO_ERROR;
}

CHIP_ERROR Resolver::TryNextResult(Impl::NodeLookupHandle & handle)
{
    VerifyOrReturnError(!mActiveLookups.Contains(&handle), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(handle.HasLookupResult(), CHIP_ERROR_NOT_FOUND);

    auto listener = handle.GetListener();
    auto peerId   = handle.GetRequest().GetPeerId();
    auto result   = handle.TakeLookupResult();

    MATTER_LOG_NODE_DISCOVERED(Tracing::DiscoveryInfoType::kRetryDifferent, &peerId, &result);

    listener->OnNodeAddressResolved(peerId, result);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Resolver::CancelLookup(Impl::NodeLookupHandle & handle, FailureCallback cancel_method)
{
    VerifyOrReturnError(handle.IsActive(), CHIP_ERROR_INVALID_ARGUMENT);
    mActiveLookups.Remove(&handle);
    Dnssd::Resolver::Instance().NodeIdResolutionNoLongerNeeded(handle.GetRequest().GetPeerId());

    // Adjust any timing updates.
    ReArmTimer();

    MATTER_LOG_NODE_DISCOVERY_FAILED(&handle.GetRequest().GetPeerId(), CHIP_ERROR_CANCELLED);

    if (cancel_method == FailureCallback::Call)
    {
        handle.GetListener()->OnNodeAddressResolutionFailed(handle.GetRequest().GetPeerId(), CHIP_ERROR_CANCELLED);
    }

    // TODO: There should be some form of cancel into Dnssd::Resolver::Instance()
    //       to stop any resolution mechanism if applicable.
    //
    // Current code just removes the internal list and any callbacks of resolution will
    // be ignored. This works from the perspective of the caller of this method,
    // but may be wasteful by letting dnssd still work in the background.

    return CHIP_NO_ERROR;
}

CHIP_ERROR Resolver::Init(System::Layer * systemLayer)
{
    mSystemLayer = systemLayer;
    Dnssd::Resolver::Instance().SetOperationalDelegate(this);
    return CHIP_NO_ERROR;
}

void Resolver::Shutdown()
{
    // mSystemLayer is set in ::Init, so if it's null that means the resolver
    // has not been initialized or has already been shut down.
    VerifyOrReturn(mSystemLayer != nullptr);

    while (mActiveLookups.begin() != mActiveLookups.end())
    {
        auto current = mActiveLookups.begin();

        const PeerId peerId     = current->GetRequest().GetPeerId();
        NodeListener * listener = current->GetListener();

        mActiveLookups.Erase(current);

        MATTER_LOG_NODE_DISCOVERY_FAILED(&peerId, CHIP_ERROR_SHUT_DOWN);

        Dnssd::Resolver::Instance().NodeIdResolutionNoLongerNeeded(peerId);
        // Failure callback only called after iterator was cleared:
        // This allows failure handlers to deallocate structures that may
        // contain the active lookup data as a member (intrusive lists members)
        listener->OnNodeAddressResolutionFailed(peerId, CHIP_ERROR_SHUT_DOWN);
    }

    // Re-arm of timer is expected to cancel any active timer as the
    // internal list of active lookups is empty at this point.
    ReArmTimer();

    mSystemLayer = nullptr;
    Dnssd::Resolver::Instance().SetOperationalDelegate(nullptr);
}

void Resolver::OnOperationalNodeResolved(const Dnssd::ResolvedNodeData & nodeData)
{
    auto it = mActiveLookups.begin();
    while (it != mActiveLookups.end())
    {
        auto current = it;
        it++;
        if (current->GetRequest().GetPeerId() != nodeData.operationalData.peerId)
        {
            continue;
        }

        ResolveResult result;

        result.address.SetPort(nodeData.resolutionData.port);
        result.address.SetInterface(nodeData.resolutionData.interfaceId);
        result.mrpRemoteConfig   = nodeData.resolutionData.GetRemoteMRPConfig();
        result.supportsTcpClient = nodeData.resolutionData.supportsTcpClient;
        result.supportsTcpServer = nodeData.resolutionData.supportsTcpServer;

        if (nodeData.resolutionData.isICDOperatingAsLIT.has_value())
        {
            result.isICDOperatingAsLIT = *(nodeData.resolutionData.isICDOperatingAsLIT);
        }

        for (size_t i = 0; i < nodeData.resolutionData.numIPs; i++)
        {
#if !INET_CONFIG_ENABLE_IPV4
            if (!nodeData.resolutionData.ipAddress[i].IsIPv6())
            {
                ChipLogError(Discovery, "Skipping IPv4 address during operational resolve.");
                continue;
            }
#endif
            result.address.SetIPAddress(nodeData.resolutionData.ipAddress[i]);
            current->LookupResult(result);
        }

        HandleAction(current);
    }

    ReArmTimer();
}

void Resolver::HandleAction(IntrusiveList<NodeLookupHandle>::Iterator & current)
{
    const NodeLookupAction action = current->NextAction(mTimeSource.GetMonotonicTimestamp());

    if (action.Type() == NodeLookupResult::kKeepSearching)
    {
        // No change in iterator
        return;
    }

    // final result, handle either success or failure
    const PeerId peerId     = current->GetRequest().GetPeerId();
    NodeListener * listener = current->GetListener();
    mActiveLookups.Erase(current);

    Dnssd::Resolver::Instance().NodeIdResolutionNoLongerNeeded(peerId);

    // ensure action is taken AFTER the current current lookup is marked complete
    // This allows failure handlers to deallocate structures that may
    // contain the active lookup data as a member (intrusive lists members)
    switch (action.Type())
    {
    case NodeLookupResult::kLookupError:
        MATTER_LOG_NODE_DISCOVERY_FAILED(&peerId, action.ErrorResult());
        listener->OnNodeAddressResolutionFailed(peerId, action.ErrorResult());
        break;
    case NodeLookupResult::kLookupSuccess:
        MATTER_LOG_NODE_DISCOVERED(Tracing::DiscoveryInfoType::kResolutionDone, &peerId, &action.ResolveResult());
        listener->OnNodeAddressResolved(peerId, action.ResolveResult());
        break;
    default:
        ChipLogError(Discovery, "Unexpected lookup state (not success or fail).");
        break;
    }
}

void Resolver::HandleTimer()
{
    auto it = mActiveLookups.begin();
    while (it != mActiveLookups.end())
    {
        auto current = it;
        it++;

        HandleAction(current);
    }

    ReArmTimer();
}

void Resolver::OnOperationalNodeResolutionFailed(const PeerId & peerId, CHIP_ERROR error)
{
    auto it = mActiveLookups.begin();
    while (it != mActiveLookups.end())
    {
        auto current = it;
        it++;
        if (current->GetRequest().GetPeerId() != peerId)
        {
            continue;
        }

        NodeListener * listener = current->GetListener();
        mActiveLookups.Erase(current);

        Dnssd::Resolver::Instance().NodeIdResolutionNoLongerNeeded(peerId);

        // Failure callback only called after iterator was cleared:
        // This allows failure handlers to deallocate structures that may
        // contain the active lookup data as a member (intrusive lists members)
        listener->OnNodeAddressResolutionFailed(peerId, error);
    }
    ReArmTimer();
}

void Resolver::ReArmTimer()
{
    mSystemLayer->CancelTimer(&OnResolveTimer, static_cast<void *>(this));

    System::Clock::Timestamp now = mTimeSource.GetMonotonicTimestamp();

    System::Clock::Timeout nextTimeout = kInvalidTimeout;
    for (auto & activeLookup : mActiveLookups)
    {
        System::Clock::Timeout timeout = activeLookup.NextEventTimeout(now);

        if (timeout < nextTimeout)
        {
            nextTimeout = timeout;
        }
    }

    if (nextTimeout == kInvalidTimeout)
    {
        // Generally this is only expected when no active lookups exist
        return;
    }

    CHIP_ERROR err = mSystemLayer->StartTimer(nextTimeout, &OnResolveTimer, static_cast<void *>(this));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Timer schedule error %s assumed permanent", err.AsString());

        // Clear out all active lookups: without timers there is no guarantee of success
        auto it = mActiveLookups.begin();
        while (it != mActiveLookups.end())
        {
            const PeerId peerId     = it->GetRequest().GetPeerId();
            NodeListener * listener = it->GetListener();

            mActiveLookups.Erase(it);
            it = mActiveLookups.begin();

            Dnssd::Resolver::Instance().NodeIdResolutionNoLongerNeeded(peerId);
            // Callback only called after active lookup is cleared
            // This allows failure handlers to deallocate structures that may
            // contain the active lookup data as a member (intrusive lists members)
            listener->OnNodeAddressResolutionFailed(peerId, err);
        }
    }
}

} // namespace Impl

Resolver & Resolver::Instance()
{
    static Impl::Resolver gResolver;
    return gResolver;
}

} // namespace AddressResolve
} // namespace chip

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

#include <lib/core/PeerId.h>
#include <lib/support/IntrusiveList.h>
#include <system/SystemClock.h>
#include <system/SystemLayer.h>
#include <transport/raw/PeerAddress.h>

namespace chip {
namespace AddressResolve {

/// Represents an object intersted in callbacks for a resolve operation.
class NodeListener
{
public:
    NodeListener()          = default;
    virtual ~NodeListener() = default;

    /// Callback executed once only for a lookup, when the final address of a
    /// node is considered  to be the best choice for reachability.
    ///
    /// The callback is expected to be executed within the chip event loop
    /// thread.
    virtual void OnNodeAddressResolved(const PeerId & peerId, const Transport::PeerAddress & address) = 0;

    /// Node resolution failues - occurs only once for a lookup, when an address
    /// could not be resolved - generally due to a timeout or due to DNSSD
    /// infrastructure returning an error.
    ///
    /// The callback is expected to be executed within the chip event loop
    /// thread.
    virtual void OnNodeAddressResolutionFailed(const PeerId & peerId, CHIP_ERROR reason) = 0;
};

/// Represents an active Address resolution lookup.
///
/// Implementations extend this class with implementation-specific data like
/// storing the 'last known good address' and 'scores' or any additional data
/// required to figure out when a resolve is ok.
class NodeLookupHandleBase : public IntrusiveListNodeBase
{
public:
    NodeLookupHandleBase() {}
    virtual ~NodeLookupHandleBase() {}

    // While active, resolve handles are maintain in an internal list
    // to be processed, so copying their values (i.e. pointers) is not
    // allowed.
    NodeLookupHandleBase(const NodeLookupHandleBase &) = delete;
    NodeLookupHandleBase & operator=(const NodeLookupHandleBase &) = delete;

    void SetListener(NodeListener * listener) { mListener = listener; }
    NodeListener * GetListener() { return mListener; }

protected:
    NodeListener * mListener = nullptr;
};

/// Represents a request to perform a single node lookup
/// Contains all the information that should be looked for as well as
/// extra timeout configurations.
class NodeLookupRequest
{
public:
    NodeLookupRequest() {}
    NodeLookupRequest(const PeerId & peerId) : mPeerId(peerId) {}

    NodeLookupRequest(const NodeLookupRequest &) = default;
    NodeLookupRequest & operator=(const NodeLookupRequest &) = default;

    const PeerId & GetPeerId() const { return mPeerId; }
    System::Clock::Milliseconds32 GetMinLookupTime() const { return mMinLookupTimeMs; }
    System::Clock::Milliseconds32 GetMaxLookupTime() const { return mMaxLookupTimeMs; }

    /// The minimum lookup time is how much to wait for additional DNSSD
    /// queries even if a reply has already been received or to allow for
    /// additional heuristics regarding node choice to succeed (e.g. for a
    /// ping6 response to be received, if an implementation goes that route)
    ///
    /// Implementations for DNSSD may choose to return responses one by one
    /// for addresses (e.g. Platform mdns does this at the time this was written)
    /// or different interfaces will return separate 'done resolving' calls.
    ///
    /// If the min lookup time is set to 0, implementations are expected to call
    /// 'OnNodeAddressResolved' as soon as the first DNSSD response is received.
    NodeLookupRequest & SetMinLookupTime(System::Clock::Milliseconds32 value)
    {
        mMinLookupTimeMs = value;
        return *this;
    }

    /// The maximum lookup time is how much to wait until a TIMEOUT error is
    /// declared.
    ///
    /// If a DNSSD response is received before this max timeout, then
    /// OnNodeAddressResolved will be called on listener objects (immediately)
    /// if the first DNSSD reply arrives after MinLookupTimeMs has expired.
    NodeLookupRequest & SetMaxLookupTime(System::Clock::Milliseconds32 value)
    {
        mMaxLookupTimeMs = value;
        return *this;
    }

private:
    static constexpr System::Clock::Milliseconds32 kMinLookupTimeMsDefault{ 200 };
    static constexpr System::Clock::Milliseconds32 kMaxLookupTimeMsDefault{ 3000 };

    PeerId mPeerId;
    System::Clock::Milliseconds32 mMinLookupTimeMs = kMinLookupTimeMsDefault;
    System::Clock::Milliseconds32 mMaxLookupTimeMs = kMaxLookupTimeMsDefault;
};

/// These things are expected to be defined by the implementation header.
namespace Impl {
class NodeLookupHandle; // MUST derive from NodeLookupHandleBase
}

class Resolver
{
public:
    virtual ~Resolver() = default;

    /// Expected to be called exactly once before the resolver is ever
    /// used.
    virtual CHIP_ERROR Init(System::Layer * systemLayer) = 0;

    /// Initiate a node lookup for a particular node and use the specified
    /// Lookup handle to keep track of node resolution
    ///
    /// If this returns CHIP_NO_ERROR, the following is expected:
    ///   - exactly one of the listener OnNodeAddressResolved
    ///     or OnNodeAddressResolutionFailed will be called at a later time
    ///   - handle must NOT be destroyed while the lookup is in progress (it
    ///     is part of an internal 'lookup list')
    ///   - handle must NOT be reused (the lookup is done on a per-node basis
    ///     and maintains lookup data internally while the operation is still
    ///     in progress)
    virtual CHIP_ERROR LookupNode(const NodeLookupRequest & request, Impl::NodeLookupHandle & handle) = 0;

    /// Expected to be provided by the implementation.
    static Resolver & Instance();
};

} // namespace AddressResolve
} // namespace chip

// outside the open space, include the required platform headers for the
// actual implementation.
// Expectations of this include:
//   - define the `Impl::NodeLookupHandle` derifing from NodeLookupHandle
//   - corresponding CPP file should provide a valid Resolver::Instance()
//     implementation
#include CHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER

namespace chip {
namespace AddressResolve {

// Make the code easy to read: do not reach into Impl.
using NodeLookupHandle = Impl::NodeLookupHandle;

} // namespace AddressResolve
} // namespace chip

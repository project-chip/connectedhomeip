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
#include <messaging/ReliableMessageProtocolConfig.h>
#include <system/SystemClock.h>
#include <system/SystemLayer.h>
#include <transport/raw/PeerAddress.h>

namespace chip {
namespace AddressResolve {

/// Contains resolve information received from nodes. Contains all information
/// bits that are considered useful but does not contain a full DNSSD data
/// structure since not all DNSSD data is useful during operational processing.
struct ResolveResult
{
    Transport::PeerAddress address;
    ReliableMessageProtocolConfig mrpRemoteConfig;
    bool supportsTcp = false;

    ResolveResult() : address(Transport::Type::kUdp), mrpRemoteConfig(GetDefaultMRPConfig()) {}
};

/// Represents an object interested in callbacks for a resolve operation.
class NodeListener
{
public:
    NodeListener()          = default;
    virtual ~NodeListener() = default;

    /// Callback executed once only for a lookup, when the final address of a
    /// node is considered to be the best choice for reachability.
    ///
    /// The callback is expected to be executed within the CHIP event loop
    /// thread.
    virtual void OnNodeAddressResolved(const PeerId & peerId, const ResolveResult & result) = 0;

    /// Node resolution failure - occurs only once for a lookup, when an address
    /// could not be resolved - generally due to a timeout or due to DNSSD
    /// infrastructure returning an error.
    ///
    /// The callback is expected to be executed within the CHIP event loop
    /// thread.
    virtual void OnNodeAddressResolutionFailed(const PeerId & peerId, CHIP_ERROR reason) = 0;
};

/// Represents an active Address resolution lookup.
///
/// Implementations extend this class with implementation-specific data like
/// storing the 'last known good address' and 'scores' or any additional data
/// required to figure out when a resolve is ok.
class NodeLookupHandleBase : public IntrusiveListNodeBase<>
{
public:
    NodeLookupHandleBase() {}
    virtual ~NodeLookupHandleBase() {}

    // While active, resolve handles are maintained in an internal list
    // to be processed, so copying their values (i.e. pointers) is not
    // allowed.
    NodeLookupHandleBase(const NodeLookupHandleBase &) = delete;
    NodeLookupHandleBase & operator=(const NodeLookupHandleBase &) = delete;

    void SetListener(NodeListener * listener) { mListener = listener; }
    NodeListener * GetListener() { return mListener; }

    /// Convenience method that is more readable than 'IsInList'
    inline bool IsActive() const { return IntrusiveListNodeBase::IsInList(); }

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
    /// additional heuristics regarding node choice to succeed.
    /// Example heuristics and considerations:
    ///   - ping/ping6 could be used as an indicator of reachability. NOTE that
    ///     not all devices may respond to ping, so this would only be an
    ///     additional signal to accept/increase suitability score of an address
    ///     and should NOT be used as a reject if no ping response
    ///   - At lookup time, if the source ip of a dns reply is contained in the
    ///     list of server ips, that is a great indication of routability and
    ///     this minlookuptime could be bypassed altogether.
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
    static constexpr uint32_t kMinLookupTimeMsDefault = 200;
    static constexpr uint32_t kMaxLookupTimeMsDefault = 45000;

    PeerId mPeerId;
    System::Clock::Milliseconds32 mMinLookupTimeMs{ kMinLookupTimeMsDefault };
    System::Clock::Milliseconds32 mMaxLookupTimeMs{ kMaxLookupTimeMsDefault };
};

/// These things are expected to be defined by the implementation header.
namespace Impl {

// The NodeLookup handle is a CONCRETE implementation that
// MUST derive from NodeLookupHandleBase
//
// The underlying reason is that this handle is used to hold memory for
// lookup metadata, so that resolvers do not need to maintain a likely unused
// pool of 'active lookup' metadata.
//
// The side-effect of this is that the Impl::NodeLookupHandle is exposed to
// clients for sizeof() memory purposes.
//
// Clients MUST only use the interface in NodeLookupHandleBase and assume all
// other methods/content is implementation defined.
class NodeLookupHandle;

} // namespace Impl

class Resolver
{
public:
    /// Enumeration defining how to handle cancel callbacks during operation
    /// cancellation.
    enum class FailureCallback
    {
        Call, // Call the failure callback
        Skip  // do not call the failure callback (generally silent operation)
    };

    virtual ~Resolver();

    /// Expected to be called at least once before the resolver is ever
    /// used.
    ///
    /// Expected to override global setting of DNSSD callback for addres resolution
    /// and may use the underlying system layer for timers and other functionality.
    ///
    /// If called multiple times, it is expected that the input systemLayer does
    /// not change.
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

    /// Inform the Lookup handle that the previous node lookup was not
    /// sufficient for the purpose of the caller (e.g establishing a session
    /// fails with the result of the previous lookup), and that more data is
    /// needed.
    ///
    /// This method must be called on a handle that is no longer active to
    /// succeed.
    ///
    /// If the handle is no longer active and has results that have not been
    /// delivered to the listener yet, the listener's OnNodeAddressResolved will
    /// be called synchronously before the method returns.  Note that depending
    /// on the listener implementation this can end up destroying the handle
    /// and/or the listener.
    ///
    /// This method will return CHIP_NO_ERROR if and only if it has called
    /// OnNodeAddressResolved.
    ///
    /// This method will return CHIP_ERROR_INCORRECT_STATE if the handle is
    /// still active.
    ///
    /// This method will return CHIP_ERROR_WELL_EMPTY if there are no more
    /// results.
    ///
    /// This method may return other errors in some cases.
    virtual CHIP_ERROR TryNextResult(Impl::NodeLookupHandle & handle) = 0;

    /// Stops an active lookup request.
    ///
    /// Caller controlls weather the `fail` callback of the handle is invoked or not by using
    /// the `cancel_method` argument.
    ///
    /// Note that there is no default cancel_method on purpose, so that the caller has to make
    /// a clear decision if the callback should or should not be invoked.
    virtual CHIP_ERROR CancelLookup(Impl::NodeLookupHandle & handle, FailureCallback cancel_method) = 0;

    /// Shut down any active resolves
    ///
    /// Will immediately fail any scheduled resolve calls and will refuse to register
    /// any new lookups until re-initialized.
    virtual void Shutdown() = 0;

    /// Expected to be provided by the implementation.
    static Resolver & Instance();
};

} // namespace AddressResolve
} // namespace chip

// outside the open space, include the required platform headers for the
// actual implementation.
// Expectations of this include:
//   - define the `Impl::NodeLookupHandle` deriving from NodeLookupHandleBase
//   - corresponding CPP file should provide a valid Resolver::Instance()
//     implementation
#include CHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER

namespace chip {
namespace AddressResolve {

// Make the code easy to read: do not reach into Impl.
using NodeLookupHandle = Impl::NodeLookupHandle;

} // namespace AddressResolve
} // namespace chip

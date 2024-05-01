/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <cstdint>
#include <limits>
#include <optional>
#include <utility>

#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <inet/UDPEndPoint.h>
#include <lib/core/CHIPError.h>
#include <lib/core/PeerId.h>
#include <lib/core/ReferenceCounted.h>
#include <lib/dnssd/Constants.h>
#include <lib/dnssd/Types.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace Dnssd {

/// Node resolution data common to both operational and commissionable discovery

/// Callbacks for resolving operational node resolution
class OperationalResolveDelegate
{
public:
    virtual ~OperationalResolveDelegate() = default;

    /// Called within the CHIP event loop after a successful node resolution.
    ///
    /// May be called multiple times: implementations may call this once per
    /// received packet and MDNS packets may arrive over different interfaces
    /// which will make nodeData have different content.
    virtual void OnOperationalNodeResolved(const ResolvedNodeData & nodeData) = 0;

    /// Notify a final failure for a node operational resolution.
    ///
    /// Called within the chip event loop if node resolution could not be performed.
    /// This may be due to internal errors or timeouts.
    ///
    /// This will be called only if 'OnOperationalNodeResolved' is never called.
    virtual void OnOperationalNodeResolutionFailed(const PeerId & peerId, CHIP_ERROR error) = 0;
};

/**
 * Node discovery context class.
 *
 * This class enables multiple clients of the global DNS-SD resolver to start simultaneous
 * discovery operations.
 *
 * An object of this class is shared between a resolver client and the concrete resolver
 * implementation. The client is responsible for allocating the context and passing it to
 * the resolver when initiating a discovery operation. The resolver, in turn, is supposed to retain
 * the context until the operation is finished. This allows the client to release the ownership of
 * the context at any time without putting the resolver at risk of using a deleted object.
 */
class DiscoveryContext : public ReferenceCounted<DiscoveryContext>
{
public:
    void SetBrowseIdentifier(intptr_t identifier) { mBrowseIdentifier.emplace(identifier); }
    void ClearBrowseIdentifier() { mBrowseIdentifier.reset(); }
    const std::optional<intptr_t> & GetBrowseIdentifier() const { return mBrowseIdentifier; }

    void SetDiscoveryDelegate(DiscoverNodeDelegate * delegate) { mDelegate = delegate; }
    void OnNodeDiscovered(const DiscoveredNodeData & nodeData)
    {
        if (mDelegate != nullptr)
        {
            mDelegate->OnNodeDiscovered(nodeData);
        }
        else
        {
            ChipLogError(Discovery, "Missing commissioning delegate. Data discarded");
        }
    }

private:
    DiscoverNodeDelegate * mDelegate = nullptr;
    std::optional<intptr_t> mBrowseIdentifier;
};

/**
 * Interface for resolving CHIP DNS-SD services
 */
class Resolver
{
public:
    virtual ~Resolver() {}

    /**
     * Initializes the resolver.
     *
     * The method must be called before other methods of this class.
     * If the resolver has already been initialized, the method exits immediately with no error.
     */
    virtual CHIP_ERROR Init(Inet::EndPointManager<Inet::UDPEndPoint> * endPointManager) = 0;

    /**
     * Returns whether the resolver has completed the initialization.
     *
     * Returns true if the resolver is ready to take node resolution and discovery requests.
     */
    virtual bool IsInitialized() = 0;

    /**
     * Shuts down the resolver if it has been initialized before.
     */
    virtual void Shutdown() = 0;

    /**
     * If nullptr is passed, the previously registered delegate is unregistered.
     */
    virtual void SetOperationalDelegate(OperationalResolveDelegate * delegate) = 0;

    /**
     * Requests resolution of the given operational node service.
     *
     * This will trigger a DNSSD query.
     *
     * When the operation succeeds or fails, and a resolver delegate has been registered,
     * the result of the operation is passed to the delegate's `OnOperationalNodeResolved` or
     * `OnOperationalNodeResolutionFailed` method, respectively.
     *
     * Multiple calls to ResolveNodeId may be coalesced by the implementation
     * and lead to just one call to
     * OnOperationalNodeResolved/OnOperationalNodeResolutionFailed, as long as
     * the later calls cause the underlying querying mechanism to re-query as if
     * there were no coalescing.
     *
     * A single call to ResolveNodeId may lead to multiple calls to
     * OnOperationalNodeResolved with different IP addresses.
     *
     * @see NodeIdResolutionNoLongerNeeded.
     */
    virtual CHIP_ERROR ResolveNodeId(const PeerId & peerId) = 0;

    /*
     * Notify the resolver that one of the consumers that called ResolveNodeId
     * successfully no longer needs the resolution result (e.g. because it got
     * the result via OnOperationalNodeResolved, or got an via
     * OnOperationalNodeResolutionFailed, or no longer cares about future
     * updates).
     *
     * There must be a NodeIdResolutionNoLongerNeeded call that matches every
     * successful ResolveNodeId call.  In particular, implementations of
     * OnOperationalNodeResolved and OnOperationalNodeResolutionFailed must call
     * NodeIdResolutionNoLongerNeeded once for each prior successful call to
     * ResolveNodeId for the relevant PeerId that has not yet had a matching
     * NodeIdResolutionNoLongerNeeded call made.
     */
    virtual void NodeIdResolutionNoLongerNeeded(const PeerId & peerId) = 0;

    /**
     * Finds all nodes of given type matching the given filter.
     *
     * Whenever a new matching node is found, the node information is passed to
     * the `OnNodeDiscovered` method of the discovery delegate configured
     * in the context object.
     *
     * This method is expected to increase the reference count of the context
     * object for as long as it takes to complete the discovery request.
     */
    virtual CHIP_ERROR StartDiscovery(DiscoveryType type, DiscoveryFilter filter, DiscoveryContext & context) = 0;

    /**
     * Stop discovery (of all node types).
     *
     * Some back ends may not support stopping discovery, so consumers should
     * not assume they will stop getting callbacks after calling this.
     */
    virtual CHIP_ERROR StopDiscovery(DiscoveryContext & context) = 0;

    /**
     * Verify the validity of an address that appears to be out of date (for example
     * because establishing a connection to it has failed).
     */
    virtual CHIP_ERROR ReconfirmRecord(const char * hostname, Inet::IPAddress address, Inet::InterfaceId interfaceId) = 0;

    /**
     * Returns the system-wide implementation of the service resolver.
     *
     * The method returns a reference to the resolver object configured by
     * a user using the \c Resolver::SetInstance() method, or the default
     * resolver returned by the \c GetDefaultResolver() function.
     */
    static Resolver & Instance();

    /**
     * Overrides the default implementation of the service resolver
     */
    static void SetInstance(Resolver & resolver);

private:
    static Resolver * sInstance;
};

/**
 * Returns the default implementation of the service resolver.
 */
extern Resolver & GetDefaultResolver();

} // namespace Dnssd
} // namespace chip

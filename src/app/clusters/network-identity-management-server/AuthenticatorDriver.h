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

#pragma once

#include <app/clusters/network-identity-management-server/NetworkIdentityStorage.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

namespace chip::app::Clusters::NetworkIdentityManagement {

class AuthenticatorDriverCallback;

/**
 * @brief Authenticator integration interface for the Network Identity Management cluster
 *
 * The cluster informs the authenticator driver about changes to the Network Identities
 * and Clients tables. The driver is also provided read-only access to the storage backend
 * of the cluster. (See `ReadOnlyNetworkIdentityStorage` in NetworkIdentityStorage.h for
 * an overview of the cluster's data model.)
 *
 * Depending on the integration strategy, the driver can update the authenticator
 * configuration based on the events from the cluster (push strategy), or the
 * authenticator itself can be set up to retrieve configuration from the cluster storage
 * backend via the driver on demand (query strategy). This could include looking up
 * client identities on the fly based on the Identifier (SKID) derived from certificate
 * presented by a client over EAP-TLS. The push and query strategies can be combined as
 * needed. Note that in both of these strategies, the cluster storage is considered the
 * source of truth; the various events (OnClientAdded(), OnNetworkIdentityAdded(), ...)
 * are void methods and provide no way for the authenticator driver to cause the overall
 * cluster operation to fail. If the driver encounters an error while pushing a change,
 * it should track the failure internally and reconcile from the cluster storage (via the
 * ReadOnlyNetworkIdentityStorage reference provided at startup) at the next opportunity.
 *
 * To help catch obvious environmental failures early, the cluster calls the Prepare*
 * methods before committing addition operations. These allow the driver to perform
 * readiness checks (e.g. authenticator process reachable, config directory writable)
 * and return an error that propagates back to the client as a cluster-level failure.
 * This is a best-effort mechanism -- a successful Prepare does not guarantee the
 * subsequent push will succeed, and drivers must still handle push failures via
 * reconciliation as described above. However, it covers the common case where a
 * gross environmental problem would otherwise result in the cluster operation appearing
 * to succeed while the authenticator remains unconfigured.
 *
 * A third option (shared state) is for the AuthenticatorDriver to also implement a
 * NetworkIdentityStorage backend that directly accesses the configuration store of the
 * authenticator, making it the shared source of truth for both the authenticator itself
 * and the cluster. However this is a broader interface, since it has to support the full
 * storage needs of the cluster implementation. With this strategy, most of the
 * AuthenticatorDriver events can be ignored / no-ops, but signaling OnClientAuthenticated
 * events back to the cluster via the callback interface is still required.
 */
class AuthenticatorDriver
{
public:
    virtual ~AuthenticatorDriver() = default;

    /// Lifecycle events

    /**
     * Called from the cluster's Startup().
     * The provided callback and storage references will be valid until after `OnShutdown()`.
     *
     * @param callback Cluster callback for OnClientAuthenticated notifications.
     * @param storage  Read-only access to the cluster's storage.
     */
    virtual void OnStartup(AuthenticatorDriverCallback & callback, ReadOnlyNetworkIdentityStorage & storage) = 0;

    /**
     * Called from the cluster's Shutdown()
     */
    virtual void OnShutdown() {}

    /// Pre-flight checks

    /**
     * Called before a new Network Identity is imported (ImportAdminSecret command).
     */
    virtual CHIP_ERROR PrepareNetworkIdentityAddition() { return CHIP_NO_ERROR; }

    /**
     * Called before a new Client is added (AddClient command).
     */
    virtual CHIP_ERROR PrepareClientAddition() { return CHIP_NO_ERROR; }

    /// State change events

    /**
     * Called after a Network Identity has been added to the cluster storage.
     * The NetworkIdentityEntry will be fully populated. The authenticator driver
     * configures the provided Network Identity as one of the available server
     * certificates for the EAP-TLS authenticator.
     */
    virtual void OnNetworkIdentityAdded(const ReadOnlyNetworkIdentityStorage::NetworkIdentityEntry & entry) {}

    /**
     * Called after a Network Identity has been removed from the cluster storage.
     * The NetworkIdentityEntry will be fully populated with the pre-deletion state of
     * the Network Identity. The authenticator driver removes the identity from the
     * list EAP-TLS server certificates of the authenticator.
     */
    virtual void OnNetworkIdentityRemoved(const ReadOnlyNetworkIdentityStorage::NetworkIdentityEntry & entry) {}

    /**
     * Called after a Client has been added to the cluster storage.
     * The ClientEntry will be fully populated. The authenticator driver configures the
     * authenticator to allow EAP-TLS authentication from a client presenting the given
     * identity certificate.
     */
    virtual void OnClientAdded(const ReadOnlyNetworkIdentityStorage::ClientEntry & entry) {}

    /**
     * Called after a client has been removed from the cluster storage.
     * The ClientEntry will be fully populated with the pre-deletion state of the client.
     * The authenticator driver configures the authenticator to no longer allow the given
     * client to authenticate, and to disconnect any associated client(s) that were
     * authenticated with this identity.
     */
    virtual void OnClientRemoved(const ReadOnlyNetworkIdentityStorage::ClientEntry & entry) {}
};

/**
 * @brief Callback interface provided to the AuthenticatorDriver by the cluster
 */
class AuthenticatorDriverCallback
{
public:
    virtual ~AuthenticatorDriverCallback() = default;

    /**
     * To be called by the AuthenticatorDriver when a client successfully authenticates. The
     * cluster uses this callback to update meta-data about the client, specifically the reference
     * to the Network Identity most recently used by the client.
     *
     * @param clientIndex          The index of the client that authenticated.
     * @param networkIdentityIndex The index of the Network Identity the client authenticated against.
     */
    virtual void OnClientAuthenticated(uint16_t clientIndex, uint16_t networkIdentityIndex) = 0;
};

} // namespace chip::app::Clusters::NetworkIdentityManagement

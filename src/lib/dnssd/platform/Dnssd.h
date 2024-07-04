/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file defines the platform API to publish and subscribe mDNS
 *      services.
 *
 *      You can find the implementation in src/platform/\<PLATFORM\>/DnssdImpl.cpp.
 */

#pragma once

#include <algorithm>
#include <optional>
#include <stdint.h>

#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <lib/core/CHIPError.h>
#include <lib/dnssd/Constants.h>
#include <lib/dnssd/ServiceNaming.h>
#include <system/TimeSource.h>

#include "DnssdBrowseDelegate.h"

namespace chip {
namespace Dnssd {

// None of these sizes include an null character at the end.
static constexpr size_t kDnssdProtocolTextMaxSize = std::max(sizeof(kOperationalProtocol), sizeof(kCommissionProtocol)) - 1;
static constexpr size_t kDnssdTypeMaxSize =
    std::max({ sizeof(kCommissionableServiceName), sizeof(kOperationalServiceName), sizeof(kCommissionerServiceName) }) - 1;
static constexpr uint8_t kDnssdTypeAndProtocolMaxSize     = kDnssdTypeMaxSize + kDnssdProtocolTextMaxSize + 1; // <type>.<protocol>
static constexpr uint16_t kDnssdTextMaxSize               = 64;
static constexpr uint8_t kDnssdFullTypeAndProtocolMaxSize = Common::kSubTypeMaxLength + /* '.' */ 1 + kDnssdTypeAndProtocolMaxSize;

enum class DnssdServiceProtocol : uint8_t
{
    kDnssdProtocolUdp = 0,
    kDnssdProtocolTcp,
    kDnssdProtocolUnknown = 255,
};

struct TextEntry
{
    const char * mKey;
    const uint8_t * mData;
    size_t mDataSize;
};

struct DnssdService
{
    char mName[Common::kInstanceNameMaxLength + 1];
    char mHostName[kHostNameMaxLength + 1] = "";
    char mType[kDnssdTypeMaxSize + 1];
    DnssdServiceProtocol mProtocol;
    Inet::IPAddressType mAddressType;   // Address record type to query or publish (A or AAAA)
    Inet::IPAddressType mTransportType; // Transport to use for the query.
    uint16_t mPort;
    chip::Inet::InterfaceId mInterface;
    TextEntry * mTextEntries;
    size_t mTextEntrySize;
    const char ** mSubTypes;
    size_t mSubTypeSize;
    std::optional<chip::Inet::IPAddress> mAddress;
    // Time to live in seconds. Per rfc6762 section 10, because we have a hostname, our default TTL is 120 seconds
    uint32_t mTtlSeconds = 120;

    void ToDiscoveredCommissionNodeData(const Span<Inet::IPAddress> & addresses, DiscoveredNodeData & nodeData);
    void ToDiscoveredOperationalNodeBrowseData(DiscoveredNodeData & nodeData);
};

/**
 * The callback function for mDNS resolve.
 *
 * The callback function SHALL NOT take the ownership of the service pointer or
 * any pointer inside this structure.
 *
 * @param[in] context     The context passed to ChipDnssdBrowse or ChipDnssdResolve.
 * @param[in] result      The mDNS resolve result, can be nullptr if error
 *                        happens.  The mAddress of this object will be ignored.
 * @param[in] addresses   IP addresses that we resolved.
 * @param[in] error       The error code.
 *
 */
using DnssdResolveCallback = void (*)(void * context, DnssdService * result, const Span<Inet::IPAddress> & addresses,
                                      CHIP_ERROR error);

/**
 * The callback function for mDNS browse.
 *
 * The callback function SHALL NOT take the ownership of the service pointer or
 * any pointer inside this structure.
 *
 * The callback function SHALL release its internal resources only when the
 * finalBrowse is true or when the error is not CHIP_NO_ERROR. Calling this
 * callback function again in either case is a programming error.
 *
 * @param[in] context       The context passed to ChipDnssdBrowse or ChipDnssdResolve.
 * @param[in] services      The service list, can be nullptr.
 * @param[in] servicesSize  The size of the service list.
 * @param[in] finalBrowse   When true, this is the last callback for this browse.
 * @param[in] error         The error code.
 *
 */
using DnssdBrowseCallback = void (*)(void * context, DnssdService * services, size_t servicesSize, bool finalBrowse,
                                     CHIP_ERROR error);

/**
 * The callback function for mDNS publish.
 *
 * Will be called when publishing succeeds or fails.
 *
 * The callback function SHALL NOT take the ownership of the service pointer or
 * any pointer inside this structure.
 *
 * @param[in] context       The context passed to ChipDnssdPublishService.
 * @param[in] type          The published type if no errors have occured, nullptr otherwise.
 * @param[in] instanceName  The published instance name if no errors have occured, nullptr otherwise.
 * @param[in] error         The error code.
 *
 */
using DnssdPublishCallback = void (*)(void * context, const char * type, const char * instanceName, CHIP_ERROR error);

using DnssdAsyncReturnCallback = void (*)(void * context, CHIP_ERROR error);

/**
 * This function initializes the mDNS module
 *
 * @param[in] initCallback    The callback for notifying the initialization result.
 * @param[in] errorCallback   The callback for notifying internal errors.
 * @param[in] context         The context passed to the callbacks.
 *
 * @retval CHIP_NO_ERROR  The initialization succeeds.
 * @retval Error code     The initialization fails
 *
 */
CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context);

/**
 * This function shuts down the mDNS module
 */
void ChipDnssdShutdown();

/**
 * Removes or marks all services being advertised for removal.
 *
 * Depending on the implementation, the function may either stop advertising existing services
 * immediately, or mark them for removal upon the subsequent `ChipDnssdFinalizeServiceUpdate` call.
 *
 * @retval CHIP_NO_ERROR  The removal succeeds.
 * @retval Error code     The removal fails.
 */
CHIP_ERROR ChipDnssdRemoveServices();

/**
 * This function publishes a service via DNS-SD.
 *
 * Calling the function again with the same service name, type, protocol,
 * interface and port but different text will update the text published.
 * This function will NOT take the ownership of service->mTextEntries memory.
 *
 * @param[in] service   The service entry.
 * @param[in] callback  The callback to call when the service is published.
 * @param[in] context   The context passed to the callback.
 *
 * @retval CHIP_NO_ERROR                The publish succeeds.
 * @retval CHIP_ERROR_INVALID_ARGUMENT  The service is nullptr.
 * @retval Error code                   The publish fails.
 *
 */
CHIP_ERROR ChipDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback = nullptr, void * context = nullptr);

/**
 * Finalizes updating advertised services.
 *
 * This function can be used by some implementations to apply changes made with the
 * `ChipDnssdRemoveServices` and `ChipDnssdPublishService` functions in case they could
 * not be applied immediately (like in case of, but not limited to, implementations
 * using SRP or requiring asynchronous interactions with a DNS-SD implementation).
 *
 * @retval CHIP_NO_ERROR  The service update finalization succeeds.
 * @retval Error code     The service update finalization fails.
 */
CHIP_ERROR ChipDnssdFinalizeServiceUpdate();

/**
 * This function browses the services published by mDNS
 *
 * @param[in] type        The service type.
 * @param[in] protocol    The service protocol.
 * @param[in] addressType The protocol version of the IP address.
 * @param[in] interface   The interface to send queries.
 * @param[in] callback    The callback for found services.
 * @param[in] context     The user context.
 * @param[out] browseIdentifier an identifier for this browse operation. This
 *                              can be used to call ChipDnssdStopBrowse.  Only
 *                              set on success.  This value remains valid until
 *                              the browse callback is called with an error or
 *                              is called with finalBrowse set to true.
 *
 * @retval CHIP_NO_ERROR                The browse succeeds.
 * @retval CHIP_ERROR_INVALID_ARGUMENT  The type or callback is nullptr.
 * @retval Error code                   The browse fails.
 *
 */
CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                           chip::Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context,
                           intptr_t * browseIdentifier);

/**
 * Stop an ongoing browse, if supported by this backend.  If successful, this
 * will trigger a final callback, with either an error status or finalBrowse set
 * to true, to the DnssdBrowseCallback that was passed to the ChipDnssdBrowse
 * call that handed back this browseIdentifier.
 *
 * @param browseIdentifier an identifier for a browse operation that came from
 *                         ChipDnssdBrowse.
 */
CHIP_ERROR ChipDnssdStopBrowse(intptr_t browseIdentifier);

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
/**
 * This function continuously browses the services published by mDNS
 * and reports any addition/removal of services.
 *
 * @param[in] type        The service type.
 * @param[in] protocol    The service protocol.
 * @param[in] addressType The protocol version of the IP address.
 * @param[in] interface   The interface to send queries.
 * @param[in] delegate    The delegate to notify when a service is found/removed.
 *
 * @retval CHIP_NO_ERROR                The browse succeeds.
 * @retval CHIP_ERROR_INVALID_ARGUMENT  The type or the delegate is nullptr.
 * @retval Error code                   The browse fails.
 *
 */
CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                           chip::Inet::InterfaceId interface, DnssdBrowseDelegate * delegate);

/**
 * Stop an ongoing browse, if supported by this backend.  If successful, this
 * will call the OnBrowseStop method of the delegate.
 */
CHIP_ERROR ChipDnssdStopBrowse(DnssdBrowseDelegate * delegate);
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN

/**
 * This function resolves the services published by mDNS
 *
 * @param[in] browseResult  The service entry returned by @ref ChipDnssdBrowse
 * @param[in] interface     The interface to send queries.
 * @param[in] callback      The callback for found services.
 * @param[in] context       The user context.
 *
 * @retval CHIP_NO_ERROR                The resolve succeeds.
 * @retval CHIP_ERROR_INVALID_ARGUMENT  The name, type or callback is nullptr.
 * @retval Error code                   The resolve fails.
 *
 */
CHIP_ERROR ChipDnssdResolve(DnssdService * browseResult, chip::Inet::InterfaceId interface, DnssdResolveCallback callback,
                            void * context);

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
/**
 * This function resolves the services published by mDNS
 *
 * @param[in] browseResult  The service entry returned by @ref ChipDnssdBrowse
 * @param[in] interface     The interface to send queries.
 * @param[in] delegate      The delegate to notify when a service is resolved.
 *
 * @retval CHIP_NO_ERROR                The resolve succeeds.
 * @retval CHIP_ERROR_INVALID_ARGUMENT  The name, type or delegate is nullptr.
 * @retval Error code                   The resolve fails.
 *
 */
CHIP_ERROR ChipDnssdResolve(DnssdService * browseResult, chip::Inet::InterfaceId interface, DiscoverNodeDelegate * delegate);
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN

/**
 * This function notifies the implementation that a resolve result is no longer
 * needed by some consumer, to allow implementations to stop unnecessary resolve
 * work.
 */
void ChipDnssdResolveNoLongerNeeded(const char * instanceName);

/**
 * This function asks the mdns daemon to asynchronously reconfirm an address that appears to be out of date.
 *
 * @param[in] hostname      The hostname the address belongs to.
 * @param[in] address       The address to reconfirm.
 * @param[in] interface     The interfaceId of the address.
 *
 */
CHIP_ERROR ChipDnssdReconfirmRecord(const char * hostname, chip::Inet::IPAddress address, chip::Inet::InterfaceId interface);

} // namespace Dnssd
} // namespace chip

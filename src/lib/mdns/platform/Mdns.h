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
 *      You can find the implementation in src/platform/\<PLATFORM\>/MdnsImpl.cpp.
 */

#pragma once

#include <stdint.h>

#include "core/CHIPError.h"
#include "inet/IPAddress.h"
#include "inet/InetInterface.h"
#include "lib/core/Optional.h"

namespace chip {
namespace Mdns {

static constexpr uint8_t kMdnsNameMaxSize  = 33;
static constexpr uint8_t kMdnsTypeMaxSize  = 32;
static constexpr uint16_t kMdnsTextMaxSize = 64;

enum class MdnsServiceProtocol : uint8_t
{
    kMdnsProtocolUdp = 0,
    kMdnsProtocolTcp,
    kMdnsProtocolUnknown = 255,
};

struct TextEntry
{
    const char * mKey;
    const uint8_t * mData;
    size_t mDataSize;
};

struct MdnsService
{
    char mName[kMdnsNameMaxSize + 1];
    char mType[kMdnsTypeMaxSize + 1];
    MdnsServiceProtocol mProtocol;
    Inet::IPAddressType mAddressType;
    uint16_t mPort;
    chip::Inet::InterfaceId mInterface;
    TextEntry * mTextEntryies;
    size_t mTextEntrySize;
    Optional<chip::Inet::IPAddress> mAddress;
};

/**
 * The callback function for mDNS resolve.
 *
 * The callback function SHALL NOT take the ownership of the service pointer or
 * any pointer inside this structure.
 *
 * @param[in] context     The context passed to ChipMdnsBrowse or ChipMdnsResolve.
 * @param[in] result      The mdns resolve result, can be nullptr if error happens.
 * @param[in] error       The error code.
 *
 */
using MdnsResolveCallback = void (*)(void * context, MdnsService * result, CHIP_ERROR error);

/**
 * The callback function for mDNS browse.
 *
 * The callback function SHALL NOT take the ownership of the service pointer or
 * any pointer inside this structure.
 *
 * @param[in] context       The context passed to ChipMdnsBrowse or ChipMdnsResolve.
 * @param[in] services      The service list, can be nullptr.
 * @param[in] servicesSize  The size of the service list.
 * @param[in] error         The error code.
 *
 */
using MdnsBrowseCallback = void (*)(void * context, MdnsService * services, size_t servicesSize, CHIP_ERROR error);

using MdnsAsnycReturnCallback = void (*)(void * context, CHIP_ERROR error);

/**
 * This function intializes the mdns module
 *
 * @param[in] initCallback    The callback for notifying the initialization result.
 * @param[in] errorCallback   The callback for notifying internal errors.
 * @param[in] context         The context passed to the callbacks.
 *
 * @retval CHIP_NO_ERROR  The initialization succeeds.
 * @retval Error code     The initialization fails
 *
 */
CHIP_ERROR ChipMdnsInit(MdnsAsnycReturnCallback initCallback, MdnsAsnycReturnCallback errorCallback, void * context);

/**
 * This function sets the host name for services.
 *
 * @param[in] hostname   The hostname.
 *
 */
CHIP_ERROR ChipMdnsSetHostname(const char * hostname);

/**
 * This function publishes an service via mDNS.
 *
 * Calling the function again with the same service name, type, protocol,
 * interface and port but different text will update the text published.
 * This function will NOT take the ownership of service->mTextEntries memory.
 *
 * @param[in] service   The service entry.
 *
 * @retval CHIP_NO_ERROR                The publish succeeds.
 * @retval CHIP_ERROR_INVALID_ARGUMENT  The service is nullptr.
 * @retval Error code                   The publish fails.
 *
 */
CHIP_ERROR ChipMdnsPublishService(const MdnsService * service);

/**
 * This function stops publishing service via mDNS.
 *
 * @retval CHIP_NO_ERROR                The publish stops successfully.
 * @retval Error code                   Stopping the publish fails.
 *
 */
CHIP_ERROR ChipMdnsStopPublish();

/**
 * This function browses the services published by mdns
 *
 * @param[in] type        The service type.
 * @param[in] protocol    The service protocol.
 * @param[in] addressType The protocol version of the IP address.
 * @param[in] interface   The interface to send queries.
 * @param[in] callback    The callback for found services.
 * @param[in] context     The user context.
 *
 * @retval CHIP_NO_ERROR                The browse succeeds.
 * @retval CHIP_ERROR_INVALID_ARGUMENT  The type or callback is nullptr.
 * @retval Error code                   The browse fails.
 *
 */
CHIP_ERROR ChipMdnsBrowse(const char * type, MdnsServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                          chip::Inet::InterfaceId interface, MdnsBrowseCallback callback, void * context);

/**
 * This function resolves the services published by mdns
 *
 * @param[in] browseResult  The service entry returned by @ref ChipMdnsBrowse
 * @param[in] interface     The interface to send queries.
 * @param[in] callback      The callback for found services.
 * @param[in] context       The user context.
 *
 * @retval CHIP_NO_ERROR                The resolve succeeds.
 * @retval CHIP_ERROR_INVALID_ARGUMENT  The name, type or callback is nullptr.
 * @retval Error code                   The resolve fails.
 *
 */
CHIP_ERROR ChipMdnsResolve(MdnsService * browseResult, chip::Inet::InterfaceId interface, MdnsResolveCallback callback,
                           void * context);

} // namespace Mdns
} // namespace chip

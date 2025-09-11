/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <functional>
#include <nfc/NFCTag.h>

#if CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING

namespace chip {
namespace Nfc {

/**
 * Defines interface for delegate that receives updates on request to find NFC tags
 */
class DLL_EXPORT NFCReaderTransportDelegate
{
public:
    virtual ~NFCReaderTransportDelegate() {}

    /**
     * Invoked when a tag is discovered by the NFC reader
     *
     * @param tagIdentifier Specifies identifier for the tag that was discovered
     */
    virtual void OnTagDiscovered(const NFCTag::Identifier & tagIdentifier) = 0;

    /**
     * Invoked when request to find tag fails.
     *
     * @param error The error code describing reason for failure to discover the tag
     */
    virtual void OnTagDiscoveryFailed(CHIP_ERROR error) = 0;
};

/**
 * Defines the interface that supports reading NFC tags
 */
class DLL_EXPORT NFCReaderTransport
{
public:
    virtual ~NFCReaderTransport() {}

    /*
     * Set/Reset the delegate to receive tag discovery updates
     *
     * @param delegate If not nullptr, the delegate to receive updates. The lifetime of the delegate
     *                 is assumed to be managed by the caller of this method.
     */
    virtual void SetDelegate(NFCReaderTransportDelegate * delegate) = 0;

    /**
     * Starts discovering NFC tags matching specified identifier
     *
     * @param tagIdentifier Specifies identifier of the tag to discover
     *
     * @return The error code specifying the status of the request.
     */
    virtual CHIP_ERROR StartDiscoveringTagMatchingAddress(const NFCTag::Identifier & tagIdentifier) = 0;

    /**
     * Stop discovering all NFC tags.
     *
     * @return The error code specifying the status of the request.
     */
    virtual CHIP_ERROR StopDiscoveringTags() = 0;

    /**
     * Query if a specified tag has been discovered from an earlier scan
     *
     * @param tagIdentifier Specifies identifier of the tag to discover
     *
     * @return true if the tag was previously detected and is still present, false otherwise.
     */
    virtual bool FindTagMatchingIdentifier(const NFCTag::Identifier & tagIdentifier) = 0;

    /**
     * Sends an NFC message to the tag
     *
     * @param message The NFC message to send to the tag.
     * @param tagIdentifier The identifier of the tag that is the destination of the message
     * @param callback Callback function to be invoked providing status on the send request. The callback must be
     *                 invoked in the context of the Matter Stack
     *
     * @return The error code specifying the status of the request.
     */
    virtual CHIP_ERROR SendMessage(System::PacketBufferHandle && message, const NFCTag::Identifier & tagIdentifier,
                                   std::function<void(System::PacketBufferHandle &&, CHIP_ERROR)> callback) = 0;
};

} // namespace Nfc
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING

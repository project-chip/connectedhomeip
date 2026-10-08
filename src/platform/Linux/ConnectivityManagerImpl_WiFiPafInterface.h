/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <stdint.h>

#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceEvent.h>
#include <platform/ConnectivityManager.h>
#include <system/SystemPacketBuffer.h>
#include <wifipaf/WiFiPAFRole.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 *  An abstract interface for controlling and interacting with a Wi-Fi
 *  Neighbor Awareness Networking (NAN) Unsynchronized Service
 *  Discovery (USD) / Public Action Frame (PAF) Commissioning
 *  Transport on a Linux platform.
 *
 *  Concrete implementations are expected to provide the platform- and
 *  driver-specific integration required to publish and subscribe to
 *  commissioning services and to exchange commissioning data over the
 *  Wi-Fi NAN USD PAF transport.
 *
 */
class WiFiPafInterface
{
public:
    // Destruction

    virtual ~WiFiPafInterface(void) = default;

    // Initialization

    /**
     *  @brief
     *    Perform explicit class initialization.
     *
     *  This initializer is invoked during stack initialization.  It
     *  should perform any actions necessary to support subsequent
     *  introspection, mutation, publish-and-subscribe, or data
     *  transmission on the Wi-Fi Neighbor Awareness Networking (NAN)
     *  Unsynchronized Service Discovery (USD) / Public Action Frame
     *  (PAF) Commissioning Transport.
     *
     *  @param[in]  inConnectivityManagerImpl
     *    A reference to the mutable platform connectivity manager
     *    implementation for which the concrete interface
     *    implementation is being initialized.
     *
     *  @sa WiFiPafShutdown
     *
     */
    virtual CHIP_ERROR Init(ConnectivityManagerImpl & inConnectivityManagerImpl) = 0;

    // Event Handling

    /**
     *  @brief
     *    Handle a platform event.
     *
     *  This provides an opportunity for the concrete interface
     *  implementation to handle a platform event.
     *
     *  @param[in]  inDeviceEvent
     *    A reference to the immutable platform event to handle.
     *
     */
    virtual void OnPlatformEvent(const ChipDeviceEvent & inDeviceEvent) = 0;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    // Wi-Fi Neighbor Awareness Networking (NAN) Unsynchronized
    // Service Discovery (USD) / Public Action Frame (PAF)
    // Commissioning Transport

    // Initialization

    /**
     *  @brief
     *    Perform explicit transport deinitialization.
     *
     *  This deinitializer is invoked at the termination of the Wi-Fi
     *  Neighbor Awareness Networking (NAN) Unsynchronized Service
     *  Discovery (USD) / Public Action Frame (PAF) Commissioning
     *  Transport. This deinitializer should perform any actions
     *  necessary to release resources associated with the Wi-Fi NAN
     *  USD PAF Commissioning Transport.
     *
     *  @param[in]  inId
     *    A reference to the immutable Wi-Fi NAN USD PAF publish or
     *    subscribe identifier.
     *
     *  @param[in]  inWiFiPafRole
     *    A reference to the immutable Wi-Fi NAN USD PAF role,
     *    influencing whether the Wi-Fi NAN USD PAF Commissioning
     *    Transport is deinitialized as a publisher or subscriber and
     *    how @a inId is interpreted.
     *
     *  @sa WiFiPafShutdown
     *
     */
    virtual CHIP_ERROR WiFiPafShutdown(const uint32_t & inId, const WiFiPAF::WiFiPafRole & inWiFiPafRole) = 0;

    // Introspection

    /**
     *  @brief
     *    Return whether the Wi-Fi NAN USD PAF Commissioning
     *    Transport resource is available.
     *
     *  @returns
     *    True if the Wi-Fi NAN USD PAF Commissioning Transport
     *    resource is available; otherwise, false.
     *
     */
    virtual bool IsWiFiPafResourceAvailable(void) const = 0;

    // Mutation

    /**
     *  @brief
     *    Set the Wi-Fi NAN USD PAF Commissioning Transport resource
     *    availability.
     *
     *  @param[in]  inAvailable
     *    A reference to an immutable Boolean indicating the Wi-Fi NAN
     *    USD PAF Commissioning Transport resource availability to
     *    set.
     *
     *  @sa IsWiFiPafResourceAvailable
     *
     */
    virtual CHIP_ERROR WiFiPafSetResourceAvailable(const bool & inAvailable) = 0;

    /**
     *  @brief
     *    Set the primary subscribe scan frequency.
     *
     *  Set the frequency, in megahertz (MHz), to be used by
     *  subsequent #WiFiPafSubscribe operations when scanning for a
     *  matching commissioning service.
     *
     *  If a concrete implementation supports multi-channel scanning,
     *  it may treat this as the preferred or initial frequency. If
     *  the implementation does not support an explicit frequency
     *  selection, it may ignore this value.
     *
     *  @param[in]  inFrequency
     *    A reference to the immutable frequency, in megahertz (MHz),
     *    that should be used for subsequent subscribe operations.
     *
     *  @sa WiFiPafSubscribe
     *  @sa WiFiPafCancelSubscribe
     *  @sa WiFiPafCancelIncompleteSubscribe
     *
     */
    virtual void WiFiPafSetSubscribeFreq(const uint16_t & inFrequency) = 0;

    // Publish-and-subscribe

    /**
     *  @brief
     *    Publish (advertise) a Wi-Fi NAN USD PAF commissioning
     *    service.
     *
     *  Start advertising a commissioning service using the parameters
     *  in @a inOutWiFiPafAdvertiseParams.
     *
     *  On success, the concrete implementation is expected to update
     *  @a inOutWiFiPafAdvertiseParams with any fields produced by the
     *  publish operation (for example, a publish identifier) so that
     *  the caller can subsequently cancel the publish with
     *  #WiFiPafCancelPublish.
     *
     *  @param[in,out]  inOutWiFiPafAdvertiseParams
     *    A reference to the publish (advertise) parameters. This may
     *    be updated by the implementation on success.
     *
     *  @sa WiFiPafCancelPublish
     *
     */
    virtual CHIP_ERROR WiFiPafPublish(ConnectivityManager::WiFiPAFAdvertiseParam & inOutWiFiPafAdvertiseParams) = 0;

    /**
     *  @brief
     *    Cancel a previously-started publish (advertise) operation.
     *
     *  Cancel the publish identified by @a inPublishId. The publish
     *  identifier is expected to be the value returned by
     *  #WiFiPafPublish (for example, via the publish parameters).
     *
     *  @param[in]  inPublishId
     *    A reference to the immutable publish identifier returned by
     *    #WiFiPafPublish on success to cancel.
     *
     *  @sa WiFiPafPublish
     *
     */
    virtual CHIP_ERROR WiFiPafCancelPublish(const uint32_t & inPublishId) = 0;

    /**
     *  @brief
     *    Subscribe to a Wi-Fi NAN USD PAF commissioning service.
     *
     *  Start a subscribe operation to discover a matching
     *  commissioning service for @a inConnectionDiscriminator.
     *
     *  The concrete implementation is expected to notify the caller
     *  of completion by invoking either @a onSuccessFunc or
     *  @a onErrorFunc (with @a inContext passed through) according to
     *  the underlying transport outcome.
     *
     *  Unless otherwise specified by the concrete implementation,
     *  this operation is asynchronous and may complete after this
     *  method returns.
     *
     *  @param[in]  inConnectionDiscriminator
     *    A reference to the immutable commissioning discriminator
     *    used to match the desired commissioning service.
     *
     *  @param[in]  inContext
     *    An opaque caller-provided context pointer that will be
     *    provided to the completion callbacks.
     *
     *  @param[in]  onSuccessFunc
     *    The callback invoked on successful completion of the
     *    subscribe operation.
     *
     *  @param[in]  onErrorFunc
     *    The callback invoked if the subscribe operation fails.
     *
     *  @sa WiFiPafSetSubscribeFreq
     *  @sa WiFiPafCancelSubscribe
     *  @sa WiFiPafCancelIncompleteSubscribe
     *
     */
    virtual CHIP_ERROR WiFiPafSubscribe(const uint16_t & inConnectionDiscriminator, void * inContext,
                                        ConnectivityManager::OnConnectionCompleteFunct onSuccessFunc,
                                        ConnectivityManager::OnConnectionErrorFunct onErrorFunc) = 0;
    /**
     *  @brief
     *    Cancel a previously-started subscribe operation.
     *
     *  Cancel the subscribe identified by @a inSubscribeId. The
     *  identifier is expected to be the value returned by
     *  #WiFiPafSubscribe on success (as defined by the concrete
     *  implementation).
     *
     *  @param[in]  inSubscribeId
     *    A reference to the immutable subscribe identifier to cancel.
     *
     *  @sa WiFiPafSetSubscribeFreq
     *  @sa WiFiPafSubscribe
     *  @sa WiFiPafCancelIncompleteSubscribe
     *
     */
    virtual CHIP_ERROR WiFiPafCancelSubscribe(const uint32_t & inSubscribeId) = 0;

    /**
     *  @brief
     *    Cancel an in-progress subscribe operation that has not yet
     *    produced a subscribe identifier.
     *
     *  Cancel any subscribe operation that is currently in progress
     *  but for which the caller does not yet have a stable subscribe
     *  identifier (for example, because subscription setup has not
     *  completed, or the identifier is produced only at a later
     *  stage).
     *
     *  @sa WiFiPafSetSubscribeFreq
     *  @sa WiFiPafSubscribe
     *  @sa WiFiPafCancelSubscribe
     *
     */
    virtual CHIP_ERROR WiFiPafCancelIncompleteSubscribe(void) = 0;

    // Data Transmission

    /**
     *  @brief
     *    Send commissioning data over an established Wi-Fi NAN USD
     *    PAF session.
     *
     *  Transmit the message contained in @a inOutMessageBuffer over the
     *  Wi-Fi NAN USD PAF session identified by @a inWiFiPafSession.
     *
     *  Ownership of @a inOutMessageBuffer is transferred to the
     *  implementation. On success, the implementation is expected to
     *  consume the buffer. On failure, the implementation may either
     *  consume the buffer or leave it unchanged; callers should treat
     *  the buffer as moved-from after invoking this method.
     *
     *  @param[in]  inWiFiPafSession
     *    A reference to the immutable session describing the Wi-Fi
     *    NAN USD PAF peer and any session-specific addressing
     *    required for transmission.
     *
     *  @param[in,out]  inMessageBuffer
     *    The message payload to transmit.
     *
     */
    virtual CHIP_ERROR WiFiPafSend(const WiFiPAF::WiFiPAFSession & inWiFiPafSession,
                                   chip::System::PacketBufferHandle && inOutMessageBuffer) = 0;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

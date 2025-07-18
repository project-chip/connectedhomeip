/*$Id: //dwh/bluetooth/DWC_ble154combo/firmware/rel/2.00a-lca01/firmware/public_inc/ot_inc/instance.h#1 $*/
/*
 *  Copyright (c) 2016, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 * @brief
 *  This file defines the OpenThread Instance API.
 */

#ifndef OPENTHREAD_INSTANCE_H_
#define OPENTHREAD_INSTANCE_H_

#include <stdlib.h>

#include "error.h"
#include "logging.h"
#include "toolchain.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The OpenThread API monotonic version number.
 *
 * This number MUST increase by one each time the contents of public OpenThread API include headers change.
 *
 * @note This number versions both OpenThread platform and user APIs.
 *
 */
#define OPENTHREAD_API_VERSION (420)

/**
 * @addtogroup api-instance
 *
 * @brief
 *   This module includes functions that control the OpenThread Instance.
 *
 * @{
 *
 */

/**
 * Represents the OpenThread instance structure.
 */
typedef struct otInstance otInstance;

/**
 * Initializes the OpenThread library.
 *
 * Initializes OpenThread and prepares it for subsequent OpenThread API calls. This function must be
 * called before any other calls to OpenThread.
 *
 * Is available and can only be used when support for multiple OpenThread instances is enabled.
 *
 * @param[in]     aInstanceBuffer      The buffer for OpenThread to use for allocating the otInstance structure.
 * @param[in,out] aInstanceBufferSize  On input, the size of aInstanceBuffer. On output, if not enough space for
 *                                     otInstance, the number of bytes required for otInstance.
 *
 * @returns  A pointer to the new OpenThread instance.
 *
 * @sa otInstanceFinalize
 *
 */
otInstance *otInstanceInit(void *aInstanceBuffer, size_t *aInstanceBufferSize);

/**
 * Initializes the static single instance of the OpenThread library.
 *
 * Initializes OpenThread and prepares it for subsequent OpenThread API calls. This function must be
 * called before any other calls to OpenThread.
 *
 * Is available and can only be used when support for multiple OpenThread instances is disabled.
 *
 * @returns A pointer to the single OpenThread instance.
 *
 */
otInstance *otInstanceInitSingle(void);

/**
 * Initializes the OpenThread instance.
 *
 * This function initializes OpenThread and prepares it for subsequent OpenThread API calls. This function must be
 * called before any other calls to OpenThread. This method utilizes static buffer to initialize the OpenThread
 * instance.
 *
 * This function is available and can only be used when support for multiple OpenThread static instances is
 * enabled (`OPENTHREAD_CONFIG_MULTIPLE_STATIC_INSTANCE_ENABLE`)
 *
 * @param[in] aIdx The index of the OpenThread instance to initialize.
 *
 * @returns  A pointer to the new OpenThread instance.
 *
 */
otInstance *otInstanceInitMultiple(uint8_t aIdx);

/**
 * Gets the instance identifier.
 *
 * The instance identifier is set to a random value when the instance is constructed, and then its value will not
 * change after initialization.
 *
 * @returns The instance identifier.
 *
 */
uint32_t otInstanceGetId(otInstance *aInstance);

/**
 * Indicates whether or not the instance is valid/initialized.
 *
 * The instance is considered valid if it is acquired and initialized using either `otInstanceInitSingle()` (in single
 * instance case) or `otInstanceInit()` (in multi instance case). A subsequent call to `otInstanceFinalize()` causes
 * the instance to be considered as uninitialized.
 *
 * @param[in] aInstance A pointer to an OpenThread instance.
 *
 * @returns TRUE if the given instance is valid/initialized, FALSE otherwise.
 *
 */
bool otInstanceIsInitialized(otInstance *aInstance);

/**
 * Disables the OpenThread library.
 *
 * Call this function when OpenThread is no longer in use.
 *
 * @param[in] aInstance A pointer to an OpenThread instance.
 *
 */
void otInstanceFinalize(otInstance *aInstance);

/**
 * Returns the current instance uptime (in msec).
 *
 * Requires `OPENTHREAD_CONFIG_UPTIME_ENABLE` to be enabled.
 *
 * The uptime is given as number of milliseconds since OpenThread instance was initialized.
 *
 * @param[in] aInstance A pointer to an OpenThread instance.
 *
 * @returns The uptime (number of milliseconds).
 *
 */
uint64_t otInstanceGetUptime(otInstance *aInstance);

#define OT_UPTIME_STRING_SIZE 24 ///< Recommended size for string representation of uptime.

/**
 * Returns the current instance uptime as a human-readable string.
 *
 * Requires `OPENTHREAD_CONFIG_UPTIME_ENABLE` to be enabled.
 *
 * The string follows the format "<hh>:<mm>:<ss>.<mmmm>" for hours, minutes, seconds and millisecond (if uptime is
 * shorter than one day) or "<dd>d.<hh>:<mm>:<ss>.<mmmm>" (if longer than a day).
 *
 * If the resulting string does not fit in @p aBuffer (within its @p aSize characters), the string will be truncated
 * but the outputted string is always null-terminated.
 *
 * @param[in]  aInstance A pointer to an OpenThread instance.
 * @param[out] aBuffer   A pointer to a char array to output the string.
 * @param[in]  aSize     The size of @p aBuffer (in bytes). Recommended to use `OT_UPTIME_STRING_SIZE`.
 *
 */
void otInstanceGetUptimeAsString(otInstance *aInstance, char *aBuffer, uint16_t aSize);

#define OT_CHANGED_IP6_ADDRESS_ADDED (1U << 0)             ///< IPv6 address was added
#define OT_CHANGED_IP6_ADDRESS_REMOVED (1U << 1)           ///< IPv6 address was removed
#define OT_CHANGED_THREAD_ROLE (1U << 2)                   ///< Role (disabled, detached, child, router, leader) changed
#define OT_CHANGED_THREAD_LL_ADDR (1U << 3)                ///< The link-local address changed
#define OT_CHANGED_THREAD_ML_ADDR (1U << 4)                ///< The mesh-local address changed
#define OT_CHANGED_THREAD_RLOC_ADDED (1U << 5)             ///< RLOC was added
#define OT_CHANGED_THREAD_RLOC_REMOVED (1U << 6)           ///< RLOC was removed
#define OT_CHANGED_THREAD_PARTITION_ID (1U << 7)           ///< Partition ID changed
#define OT_CHANGED_THREAD_KEY_SEQUENCE_COUNTER (1U << 8)   ///< Thread Key Sequence changed
#define OT_CHANGED_THREAD_NETDATA (1U << 9)                ///< Thread Network Data changed
#define OT_CHANGED_THREAD_CHILD_ADDED (1U << 10)           ///< Child was added
#define OT_CHANGED_THREAD_CHILD_REMOVED (1U << 11)         ///< Child was removed
#define OT_CHANGED_IP6_MULTICAST_SUBSCRIBED (1U << 12)     ///< Subscribed to a IPv6 multicast address
#define OT_CHANGED_IP6_MULTICAST_UNSUBSCRIBED (1U << 13)   ///< Unsubscribed from a IPv6 multicast address
#define OT_CHANGED_THREAD_CHANNEL (1U << 14)               ///< Thread network channel changed
#define OT_CHANGED_THREAD_PANID (1U << 15)                 ///< Thread network PAN Id changed
#define OT_CHANGED_THREAD_NETWORK_NAME (1U << 16)          ///< Thread network name changed
#define OT_CHANGED_THREAD_EXT_PANID (1U << 17)             ///< Thread network extended PAN ID changed
#define OT_CHANGED_NETWORK_KEY (1U << 18)                  ///< Network key changed
#define OT_CHANGED_PSKC (1U << 19)                         ///< PSKc changed
#define OT_CHANGED_SECURITY_POLICY (1U << 20)              ///< Security Policy changed
#define OT_CHANGED_CHANNEL_MANAGER_NEW_CHANNEL (1U << 21)  ///< Channel Manager new pending Thread channel changed
#define OT_CHANGED_SUPPORTED_CHANNEL_MASK (1U << 22)       ///< Supported channel mask changed
#define OT_CHANGED_COMMISSIONER_STATE (1U << 23)           ///< Commissioner state changed
#define OT_CHANGED_THREAD_NETIF_STATE (1U << 24)           ///< Thread network interface state changed
#define OT_CHANGED_THREAD_BACKBONE_ROUTER_STATE (1U << 25) ///< Backbone Router state changed
#define OT_CHANGED_THREAD_BACKBONE_ROUTER_LOCAL (1U << 26) ///< Local Backbone Router configuration changed
#define OT_CHANGED_JOINER_STATE (1U << 27)                 ///< Joiner state changed
#define OT_CHANGED_ACTIVE_DATASET (1U << 28)               ///< Active Operational Dataset changed
#define OT_CHANGED_PENDING_DATASET (1U << 29)              ///< Pending Operational Dataset changed
#define OT_CHANGED_NAT64_TRANSLATOR_STATE (1U << 30)       ///< The state of NAT64 translator changed
#define OT_CHANGED_PARENT_LINK_QUALITY (1U << 31)          ///< Parent link quality changed

/**
 * Represents a bit-field indicating specific state/configuration that has changed. See `OT_CHANGED_*`
 * definitions.
 *
 */
typedef uint32_t otChangedFlags;

/**
 * Pointer is called to notify certain configuration or state changes within OpenThread.
 *
 * @param[in]  aFlags    A bit-field indicating specific state that has changed.  See `OT_CHANGED_*` definitions.
 * @param[in]  aContext  A pointer to application-specific context.
 *
 */
typedef void (*otStateChangedCallback)(otChangedFlags aFlags, void *aContext);

/**
 * Registers a callback to indicate when certain configuration or state changes within OpenThread.
 *
 * @param[in]  aInstance  A pointer to an OpenThread instance.
 * @param[in]  aCallback  A pointer to a function that is called with certain configuration or state changes.
 * @param[in]  aContext   A pointer to application-specific context.
 *
 * @retval OT_ERROR_NONE     Added the callback to the list of callbacks.
 * @retval OT_ERROR_ALREADY  The callback was already registered.
 * @retval OT_ERROR_NO_BUFS  Could not add the callback due to resource constraints.
 *
 */
otError otSetStateChangedCallback(otInstance *aInstance, otStateChangedCallback aCallback, void *aContext);

/**
 * Removes a callback to indicate when certain configuration or state changes within OpenThread.
 *
 * @param[in]  aInstance   A pointer to an OpenThread instance.
 * @param[in]  aCallback   A pointer to a function that is called with certain configuration or state changes.
 * @param[in]  aContext    A pointer to application-specific context.
 *
 */
void otRemoveStateChangeCallback(otInstance *aInstance, otStateChangedCallback aCallback, void *aContext);

/**
 * Triggers a platform reset.
 *
 * The reset process ensures that all the OpenThread state/info (stored in volatile memory) is erased. Note that the
 * `otPlatformReset` does not erase any persistent state/info saved in non-volatile memory.
 *
 * @param[in]  aInstance  A pointer to an OpenThread instance.
 *
 */
void otInstanceReset(otInstance *aInstance);

/**
 * Triggers a platform reset to bootloader mode, if supported.
 *
 * Requires `OPENTHREAD_CONFIG_PLATFORM_BOOTLOADER_MODE_ENABLE`.
 *
 * @param[in]  aInstance  A pointer to an OpenThread instance.
 *
 * @retval OT_ERROR_NONE         Reset to bootloader successfully.
 * @retval OT_ERROR_BUSY         Failed due to another operation is ongoing.
 * @retval OT_ERROR_NOT_CAPABLE  Not capable of resetting to bootloader.
 *
 */
otError otInstanceResetToBootloader(otInstance *aInstance);

/**
 * Deletes all the settings stored on non-volatile memory, and then triggers a platform reset.
 *
 * @param[in]  aInstance  A pointer to an OpenThread instance.
 *
 */
void otInstanceFactoryReset(otInstance *aInstance);

/**
 * Resets the internal states of the OpenThread radio stack.
 *
 * Callbacks and configurations are preserved.
 *
 * This API is only available under radio builds (`OPENTHREAD_RADIO = 1`).
 *
 * @param[in]  aInstance  A pointer to an OpenThread instance.
 *
 */
void otInstanceResetRadioStack(otInstance *aInstance);

/**
 * Erases all the OpenThread persistent info (network settings) stored on non-volatile memory.
 * Erase is successful only if the device is in `disabled` state/role.
 *
 * @param[in]  aInstance A pointer to an OpenThread instance.
 *
 * @retval OT_ERROR_NONE           All persistent info/state was erased successfully.
 * @retval OT_ERROR_INVALID_STATE  Device is not in `disabled` state/role.
 *
 */
otError otInstanceErasePersistentInfo(otInstance *aInstance);

/**
 * Gets the OpenThread version string.
 *
 * @returns A pointer to the OpenThread version.
 *
 */
const char *otGetVersionString(void);

/**
 * Gets the OpenThread radio version string.
 *
 * @param[in]  aInstance A pointer to an OpenThread instance.
 *
 * @returns A pointer to the OpenThread radio version.
 *
 */
const char *otGetRadioVersionString(otInstance *aInstance);

/**
 * @}
 *
 */

#ifdef __cplusplus
} // extern "C"
#endif

#endif // OPENTHREAD_INSTANCE_H_

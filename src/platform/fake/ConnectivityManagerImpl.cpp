/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <platform/ConnectivityManager.h>

#include <platform/internal/GenericConnectivityManagerImpl_UDP.ipp>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <platform/internal/GenericConnectivityManagerImpl_TCP.ipp>
#endif

namespace chip {
namespace DeviceLayer {

/** Singleton instance of the ConnectivityManager implementation object.
 */
ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

} // namespace DeviceLayer
} // namespace chip

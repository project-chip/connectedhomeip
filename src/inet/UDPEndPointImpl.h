/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Google LLC
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * This file includes a specific configured concrete implementation of Inet::UDPEndPoint.
 */

#pragma once

#include <inet/UDPEndPoint.h>

#ifdef INET_UDP_END_POINT_IMPL_CONFIG_FILE
#include INET_UDP_END_POINT_IMPL_CONFIG_FILE
#else // INET_UDP_END_POINT_IMPL_CONFIG_FILE
#error "INET_UDP_END_POINT_IMPL_CONFIG_FILE not defined"
#endif // INET_UDP_END_POINT_IMPL_CONFIG_FILE

namespace chip {
namespace Inet {

using UDPEndPointManagerImpl = EndPointManagerImplPool<UDPEndPointImpl>;

} // namespace Inet
} // namespace chip

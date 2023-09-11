/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Google LLC
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * This file includes a specific configured concrete implementation of Inet::TCPEndPoint.
 */

#pragma once

#include <inet/TCPEndPoint.h>

#ifdef INET_TCP_END_POINT_IMPL_CONFIG_FILE
#include INET_TCP_END_POINT_IMPL_CONFIG_FILE
#else // INET_TCP_END_POINT_IMPL_CONFIG_FILE
#error "INET_TCP_END_POINT_IMPL_CONFIG_FILE not defined"
#endif // INET_TCP_END_POINT_IMPL_CONFIG_FILE

namespace chip {
namespace Inet {

using TCPEndPointManagerImpl = EndPointManagerImplPool<TCPEndPointImpl>;

} // namespace Inet
} // namespace chip

/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

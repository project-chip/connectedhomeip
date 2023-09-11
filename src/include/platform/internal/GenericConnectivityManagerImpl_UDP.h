/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides a generic implementation of ConnectivityManager features
 *          for use on platforms that use UDP.
 */

#pragma once

#include <inet/UDPEndPointImpl.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ImplClass>
class GenericConnectivityManagerImpl_UDP
{
public:
    // ConnectivityManager:
    static chip::Inet::EndPointManager<Inet::UDPEndPoint> & _UDPEndPointManager();
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

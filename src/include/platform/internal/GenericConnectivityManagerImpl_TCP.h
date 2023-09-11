/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides a generic implementation of ConnectivityManager features
 *          for use on platforms that use TCP.
 */

#pragma once

#include <inet/TCPEndPointImpl.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ImplClass>
class GenericConnectivityManagerImpl_TCP
{
public:
    // ConnectivityManager:
    static chip::Inet::EndPointManager<Inet::TCPEndPoint> & _TCPEndPointManager();
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

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

#ifndef GENERIC_CONNECTIVITY_MANAGER_IMPL_TCP_CPP
#define GENERIC_CONNECTIVITY_MANAGER_IMPL_TCP_CPP

#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ImplClass>
chip::Inet::EndPointManager<Inet::TCPEndPoint> & GenericConnectivityManagerImpl_TCP<ImplClass>::_TCPEndPointManager()
{
    static chip::Inet::TCPEndPointManagerImpl sTCPEndPointManagerImpl;
    return sTCPEndPointManagerImpl;
}

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
// NB: This must come after all templated class members are defined.
template class GenericConnectivityManagerImpl_TCP<ConnectivityManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_CONNECTIVITY_MANAGER_IMPL_TCP_CPP

/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

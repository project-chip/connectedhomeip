/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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
 *      This file contains the basis class for all the various transport
 *      endpoint classes in the Inet layer, i.e. TCP and UDP.
 */

#pragma once

#include <inet/InetConfig.h>
#include <lib/support/DLLUtil.h>

namespace chip {
namespace Inet {

class InetLayer;

/**
 * Basis of internet transport endpoint classes.
 */
class DLL_EXPORT EndPointBase
{
public:
    EndPointBase(InetLayer & aInetLayer, void * aAppState = nullptr) : mAppState(aAppState), mInetLayer(aInetLayer) {}
    virtual ~EndPointBase() = default;

    /**
     *  Returns a reference to the Inet layer object that owns this basis object.
     */
    InetLayer & Layer() const { return mInetLayer; }

    void * mAppState;

private:
    InetLayer & mInetLayer; /**< InetLayer object that owns this object. */
};

} // namespace Inet
} // namespace chip

#ifdef CHIP_INET_END_POINT_IMPL_CONFIG_FILE
#include CHIP_INET_END_POINT_IMPL_CONFIG_FILE
#else // CHIP_INET_END_POINT_IMPL_CONFIG_FILE
#include <inet/EndPointBasisImplSockets.h>
#endif // CHIP_INET_END_POINT_IMPL_CONFIG_FILE

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
    /**
     *  Returns a reference to the Inet layer object that owns this basis object.
     */
    InetLayer & Layer() const { return *mInetLayer; }

    /**
     *  Returns \c true if the basis object was obtained by the specified Inet layer instance.
     *
     *  @note
     *      Does not check whether the object is actually obtained by the system layer instance associated with the Inet layer
     *      instance. It merely tests whether \c aInetLayer is the Inet layer instance that was provided to \c InitInetLayerBasis.
     */
    bool IsCreatedByInetLayer(const InetLayer & aInetLayer) const { return mInetLayer == &aInetLayer; }

    void * AppState;

private:
    InetLayer * mInetLayer; /**< Pointer to the InetLayer object that owns this object. */

protected:
    virtual ~EndPointBase() = default;

    void InitEndPointBasis(InetLayer & aInetLayer, void * aAppState = nullptr)
    {
        AppState   = aAppState;
        mInetLayer = &aInetLayer;
        InitEndPointBasisImpl();
    }

    virtual void InitEndPointBasisImpl() = 0;
};

} // namespace Inet
} // namespace chip

#ifdef CHIP_INET_END_POINT_IMPL_CONFIG_FILE
#include CHIP_INET_END_POINT_IMPL_CONFIG_FILE
#else // CHIP_INET_END_POINT_IMPL_CONFIG_FILE
#include <inet/EndPointBasisImplSockets.h>
#endif // CHIP_INET_END_POINT_IMPL_CONFIG_FILE

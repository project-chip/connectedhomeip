/*
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
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
#include <lib/core/ReferenceCounted.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/ReferenceCountedPtr.h>

namespace chip {

namespace System {
class Layer;
} // namespace System

namespace Inet {

template <typename EndPointType>
class EndPointManager;

template <typename EndPointType>
class EndPointDeletor;

template <class EndPointType>
class EndPointHandle;

/**
 * Base class of internet transport endpoint classes. Guarded by EndPointManager::CreateEndPoint to guarantee proper ref-counting.
 */
template <typename EndPointType>
class DLL_EXPORT EndPointBasis : public ReferenceCountedProtected<EndPointType, EndPointDeletor<EndPointType>>
{
public:
    using EndPoint = EndPointType;

    EndPointBasis(EndPointManager<EndPoint> & endPointManager) : mAppState(nullptr), mEndPointManager(endPointManager) {}

    /**
     *  Returns a reference to the endpoint fatory that owns this basis object.
     */
    EndPointManager<EndPoint> & GetEndPointManager() const { return mEndPointManager; }

    /**
     *  Returns a reference to the System::Layer associated with this object.
     */
    chip::System::Layer & GetSystemLayer() const { return mEndPointManager.SystemLayer(); }

    void * mAppState;

protected:
    friend class EndPointHandle<EndPointType>;

    inline void Delete() { GetEndPointManager().DeleteEndPoint(static_cast<EndPointType *>(this)); }

private:
    EndPointManager<EndPoint> & mEndPointManager; /**< Factory that owns this object. */
};

template <typename EndPointType>
class EndPointDeletor
{
public:
    static void Release(EndPointType * obj) { obj->Free(); }
};

} // namespace Inet
} // namespace chip

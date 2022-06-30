/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 * Provides access to UDP (and optionally TCP) EndPointManager.
 */

#pragma once

#include <inet/InetError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ObjectLifeCycle.h>
#include <lib/support/Pool.h>
#include <platform/LockTracker.h>
#include <system/SystemLayer.h>
#include <system/SystemStats.h>

#include <stdint.h>

namespace chip {
namespace Inet {

/**
 * Template providing traits for EndPoint types used by EndPointManager.
 *
 * Instances must define:
 *      static constexpr const char * kName;
 *      static constexpr int kSystemStatsKey;
 */
template <class EndPointType>
struct EndPointProperties;

/**
 * Manage creating, deletion, and iteration of Inet::EndPoint types.
 */
template <class EndPointType>
class EndPointManager
{
public:
    using EndPoint        = EndPointType;
    using EndPointVisitor = Loop (*)(EndPoint *);

    EndPointManager() {}
    virtual ~EndPointManager() { VerifyOrDie(mLayerState.Destroy()); }

    CHIP_ERROR Init(System::Layer & systemLayer)
    {
        RegisterLayerErrorFormatter();
        VerifyOrReturnError(mLayerState.SetInitializing(), CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(systemLayer.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
        mSystemLayer = &systemLayer;
        mLayerState.SetInitialized();
        return CHIP_NO_ERROR;
    }

    void Shutdown()
    {
        // Return to uninitialized state to permit re-initialization.
        mLayerState.ResetFromInitialized();
        mSystemLayer = nullptr;
    }

    System::Layer & SystemLayer() const { return *mSystemLayer; }

    CHIP_ERROR NewEndPoint(EndPoint ** retEndPoint)
    {
        assertChipStackLockedByCurrentThread();
        VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

        *retEndPoint = CreateEndPoint();
        if (*retEndPoint == nullptr)
        {
            ChipLogError(Inet, "%s endpoint pool FULL", EndPointProperties<EndPointType>::kName);
            return CHIP_ERROR_ENDPOINT_POOL_FULL;
        }

        SYSTEM_STATS_INCREMENT(EndPointProperties<EndPointType>::kSystemStatsKey);
        return CHIP_NO_ERROR;
    }

    void DeleteEndPoint(EndPoint * endPoint)
    {
        SYSTEM_STATS_DECREMENT(EndPointProperties<EndPointType>::kSystemStatsKey);
        ReleaseEndPoint(endPoint);
    }

    virtual EndPoint * CreateEndPoint()                         = 0;
    virtual void ReleaseEndPoint(EndPoint * endPoint)           = 0;
    virtual Loop ForEachEndPoint(const EndPointVisitor visitor) = 0;

private:
    ObjectLifeCycle mLayerState;
    System::Layer * mSystemLayer;
};

template <typename EndPointImpl>
class EndPointManagerImplPool : public EndPointManager<typename EndPointImpl::EndPoint>
{
public:
    using Manager  = EndPointManager<typename EndPointImpl::EndPoint>;
    using EndPoint = typename EndPointImpl::EndPoint;

    EndPointManagerImplPool()           = default;
    ~EndPointManagerImplPool() override = default;

    EndPoint * CreateEndPoint() override { return sEndPointPool.CreateObject(*this); }
    void ReleaseEndPoint(EndPoint * endPoint) override { sEndPointPool.ReleaseObject(static_cast<EndPointImpl *>(endPoint)); }
    Loop ForEachEndPoint(const typename Manager::EndPointVisitor visitor) override
    {
        return sEndPointPool.ForEachActiveObject([&](EndPoint * endPoint) -> Loop { return visitor(endPoint); });
    }

private:
    ObjectPool<EndPointImpl, EndPointProperties<EndPoint>::kNumEndPoints> sEndPointPool;
};

class TCPEndPoint;
class UDPEndPoint;

} // namespace Inet
} // namespace chip

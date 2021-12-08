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
 *      static constexpr const char * Name;
 *      static constexpr int SystemStatsKey;
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
    virtual ~EndPointManager() {}

    CHIP_ERROR Init(System::Layer & systemLayer)
    {
        mSystemLayer = &systemLayer;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Shutdown() { return CHIP_NO_ERROR; }

    System::Layer & SystemLayer() const { return *mSystemLayer; }

    CHIP_ERROR NewEndPoint(EndPoint ** retEndPoint)
    {
        assertChipStackLockedByCurrentThread();

        *retEndPoint = CreateEndPoint();
        if (*retEndPoint == nullptr)
        {
            ChipLogError(Inet, "%s endpoint pool FULL", EndPointProperties<EndPointType>::Name);
            return CHIP_ERROR_ENDPOINT_POOL_FULL;
        }

        // TODO: Use the Impl's underlying ObjectPool statistics
        SYSTEM_STATS_INCREMENT(EndPointProperties<EndPointType>::SystemStatsKey);
        return CHIP_NO_ERROR;
    }

    virtual EndPoint * CreateEndPoint()                         = 0;
    virtual void DeleteEndPoint(EndPoint * endPoint)            = 0;
    virtual Loop ForEachEndPoint(const EndPointVisitor visitor) = 0;

private:
    System::Layer * mSystemLayer;
};

template <typename EndPointImpl, unsigned int NUM_ENDPOINTS>
class EndPointManagerImplPool : public EndPointManager<typename EndPointImpl::EndPoint>
{
public:
    using Manager  = EndPointManager<typename EndPointImpl::EndPoint>;
    using EndPoint = typename EndPointImpl::EndPoint;

    EndPointManagerImplPool() = default;
    ~EndPointManagerImplPool() { VerifyOrDie(sEndPointPool.Allocated() == 0); }

    EndPoint * CreateEndPoint() override { return sEndPointPool.CreateObject(*this); }
    void DeleteEndPoint(EndPoint * endPoint) override { sEndPointPool.ReleaseObject(static_cast<EndPointImpl *>(endPoint)); }
    Loop ForEachEndPoint(const typename Manager::EndPointVisitor visitor) override
    {
        return sEndPointPool.ForEachActiveObject([&](EndPoint * endPoint) -> Loop { return visitor(endPoint); });
    }

private:
    ObjectPool<EndPointImpl, NUM_ENDPOINTS> sEndPointPool;
};

class TCPEndPoint;
class UDPEndPoint;

/**
 * Provides access to UDP (and optionally TCP) EndPointManager.
 */
class InetLayer
{
public:
    InetLayer() = default;
    ~InetLayer() { mLayerState.Destroy(); }

    /**
     *  This is the InetLayer explicit initializer. This must be called
     *  and complete successfully before the InetLayer may be used.
     *
     *  The caller may provide an optional context argument which will be
     *  passed back via any platform-specific hook functions. For
     *  LwIP-based adaptations, this will typically be a pointer to the
     *  event queue associated with the InetLayer instance.
     *
     *  @param[in]  aSystemLayer                A required instance of the chip System Layer already successfully initialized.
     *  @param[in]  udpEndPointManager          A required instance of an implementation of EndPointManager<UDPEndPoint>.
     *                                          This function will initialize the EndPointManager.
     *
     *  @retval   #CHIP_ERROR_INCORRECT_STATE   If the InetLayer is in an incorrect state.
     *  @retval   #CHIP_NO_ERROR                On success.
     *
     */
    CHIP_ERROR Init(System::Layer & aSystemLayer, EndPointManager<UDPEndPoint> * udpEndPointManager);

    /**
     *  This is the InetLayer explicit deinitializer and should be called
     *  prior to disposing of an instantiated InetLayer instance.
     *
     *  Must be called before System::Layer::Shutdown(), since this holds a pointer to that.
     *
     *  @return #CHIP_NO_ERROR on success; otherwise, a specific error indicating
     *          the reason for shutdown failure.
     *
     */
    CHIP_ERROR Shutdown();

    EndPointManager<UDPEndPoint> * GetUDPEndPointManager() const { return mUDPEndPointManager; }

    // Initialize the TCP EndPointManager. Must be called after Init() if the appication uses TCP.
    CHIP_ERROR InitTCP(EndPointManager<TCPEndPoint> * tcpEndPointManager);
    // Shut down the TCP EndPointManager. Must be called before Shutdown() if the appication uses TCP.
    CHIP_ERROR ShutdownTCP();
    EndPointManager<TCPEndPoint> * GetTCPEndPointManager() const { return mTCPEndPointManager; }

    chip::System::Layer * SystemLayer() const { return mSystemLayer; }

private:
    ObjectLifeCycle mLayerState;
    System::Layer * mSystemLayer;
    EndPointManager<TCPEndPoint> * mTCPEndPointManager;
    EndPointManager<UDPEndPoint> * mUDPEndPointManager;
};

} // namespace Inet
} // namespace chip

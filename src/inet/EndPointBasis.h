/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2015-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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

namespace chip {

namespace System {
class Layer;
} // namespace System

namespace Inet {

template <typename EndPointType>
class EndPointManager;

template <typename EndPointType>
class EndPointDeletor;

/**
 * Basis of internet transport endpoint classes.
 */
template <typename EndPointType>
class DLL_EXPORT EndPointBasis : public ReferenceCounted<EndPointType, EndPointDeletor<EndPointType>>
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

private:
    EndPointManager<EndPoint> & mEndPointManager; /**< Factory that owns this object. */
};

template <typename EndPointType>
class EndPointDeletor
{
public:
    static void Release(EndPointType * obj) { obj->GetEndPointManager().DeleteEndPoint(obj); }
};

} // namespace Inet
} // namespace chip

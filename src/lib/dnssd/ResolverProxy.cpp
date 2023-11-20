/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "ResolverProxy.h"

#include <lib/support/CHIPMem.h>

namespace chip {
namespace Dnssd {

CHIP_ERROR ResolverProxy::Init(Inet::EndPointManager<Inet::UDPEndPoint> * udpEndPoint)
{
    VerifyOrReturnError(mContext == nullptr, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(mResolver.Init(udpEndPoint));
    mContext = Platform::New<DiscoveryContext>();
    VerifyOrReturnError(mContext != nullptr, CHIP_ERROR_NO_MEMORY);

    return CHIP_NO_ERROR;
}

void ResolverProxy::Shutdown()
{
    VerifyOrReturn(mContext != nullptr);
    mContext->SetCommissioningDelegate(nullptr);
    mContext->Release();
    mContext = nullptr;
}

CHIP_ERROR ResolverProxy::DiscoverCommissionableNodes(DiscoveryFilter filter)
{
    VerifyOrReturnError(mContext != nullptr, CHIP_ERROR_INCORRECT_STATE);

    return mResolver.DiscoverCommissionableNodes(filter, *mContext);
}

CHIP_ERROR ResolverProxy::DiscoverCommissioners(DiscoveryFilter filter)
{
    VerifyOrReturnError(mContext != nullptr, CHIP_ERROR_INCORRECT_STATE);

    return mResolver.DiscoverCommissioners(filter, *mContext);
}

CHIP_ERROR ResolverProxy::StopDiscovery()
{
    VerifyOrReturnError(mContext != nullptr, CHIP_ERROR_INCORRECT_STATE);

    return mResolver.StopDiscovery(*mContext);
}

} // namespace Dnssd
} // namespace chip

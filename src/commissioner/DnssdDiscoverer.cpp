/*
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    All rights reserved.
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

#include <commissioner/Discoverer.h>

namespace chip {
namespace Commissioner {
namespace CommissionableNodeDiscoverer {

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
DnssdDiscoverer::DnssdDiscoverer(Controller::DeviceControllerSystemState & systemState, Delegate ** delegate) :
    Joinable(delegate), mSystemState(systemState)
{
    mSystemState.Retain();
}

DnssdDiscoverer::~DnssdDiscoverer()
{
    mSystemState.Release();
}

void DnssdDiscoverer::Shutdown()
{
    if (InProgress())
    {
        mDNSResolver.Shutdown();
        // TODO: Setting the instance delegate to nullptr is a workaround for
        //       #13227.  If we do not do this, minimal mdns can call back into
        //       our allocated delegate proxy after it is freed.
        chip::Dnssd::Resolver::Instance().SetResolverDelegate(nullptr);
        ReleaseShutdownToken();
    }
}

CHIP_ERROR DnssdDiscoverer::StartDnssdDiscovery(SetupPayload & payload, Platform::SharedPtr<ShutdownToken> token)
{
    VerifyOrReturnError(token.get() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    Dnssd::DiscoveryFilter filter;
    filter.type = payload.isShortDiscriminator ? Dnssd::DiscoveryFilterType::kShortDiscriminator
                                               : Dnssd::DiscoveryFilterType::kLongDiscriminator;
    filter.code = payload.isShortDiscriminator ? (payload.discriminator >> 8) & 0xF : payload.discriminator;
    ReturnErrorOnFailure(this->mDNSResolver.Init(mSystemState.UDPEndPointManager()));
    this->mDNSResolver.SetResolverDelegate(this);
    this->mDeviceDiscoveryDelegate = this;
    ReturnErrorOnFailure(this->mDNSResolver.FindCommissionableNodes(filter));
    RetainShutdownToken(token);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnssdDiscoverer::GetNextDnssdCandidate(Commissionee & commissionee)
{
    ReturnErrorOnFailure(GetValidRecord());
    ReturnErrorOnFailure(GetAddress(commissionee.mCommissionableNodeAddress, commissionee.mMrpConfig));
#if CHIP_PROGRESS_LOGGING
    char addressStr[Transport::PeerAddress::kMaxToStringSize];
    commissionee.mCommissionableNodeAddress.Value().ToString(addressStr);
    ChipLogProgress(Controller, "Commissionable node candidate identified at %s", addressStr);
#endif // CHIP_PROGRESS_LOGGING
    ++mIpIdx;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnssdDiscoverer::GetValidRecord()
{
    while (mNodeIdx < GetDiscoveredNodes().size() && GetDiscoveredNodes().data()[mNodeIdx].IsValid())
    {
        if (mIpIdx < GetDiscoveredNodes().data()[mNodeIdx].numIPs)
        {
            return CHIP_NO_ERROR;
        }
        ++mNodeIdx;
        mIpIdx = 0;
    }
    if (mNodeIdx >= GetDiscoveredNodes().size())
    {
        Shutdown(); // If mNodes is full, no point searching further.
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR DnssdDiscoverer::GetAddress(chip::Optional<Transport::PeerAddress> & address,
                                       chip::Optional<ReliableMessageProtocolConfig> & mrpConfig)
{
    if (mNodeIdx >= GetDiscoveredNodes().size() || mIpIdx >= GetDiscoveredNodes().data()[mNodeIdx].numIPs)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    Dnssd::DiscoveredNodeData & node = GetDiscoveredNodes().data()[mNodeIdx];
    Inet::InterfaceId interfaceId = node.ipAddress[mIpIdx].IsIPv6LinkLocal() ? node.interfaceId[mIpIdx] : Inet::InterfaceId::Null();
#if 0
        // TODO(#13326): connecting via TCP does not work
        if (node.supportsTcp)
        {
            address.SetValue(Transport::PeerAddress::TCP(node.ipAddress[mIpIdx], node.port, interfaceId));
            mrpConfig.SetValue(node.GetMRPConfig());
        }
        else
#endif
    {
        address.SetValue(Transport::PeerAddress::UDP(node.ipAddress[mIpIdx], node.port, interfaceId));
        mrpConfig.SetValue(node.GetMRPConfig());
    }
    return CHIP_NO_ERROR;
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_DNSSD

} // namespace CommissionableNodeDiscoverer
} // namespace Commissioner
} // namespace chip

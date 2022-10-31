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
#include <lib/dnssd/minimal_mdns/AddressPolicy.h>

#include <lib/support/logging/CHIPLogging.h>

namespace mdns {
namespace Minimal {
namespace {

class SimpleInterfacesIterator : public mdns::Minimal::ListenIterator
{
private:
    enum class ReturnState
    {
        kStart,
#if INET_CONFIG_ENABLE_IPV4
        kIPv4,
#endif
        kEnd,
    };
public:
    SimpleInterfacesIterator() : mState(ReturnState::kStart) {}

    bool Next(chip::Inet::InterfaceId * id, chip::Inet::IPAddressType * type) override
    {
        *id = chip::Inet::InterfaceId::Null();

        switch (mState)
        {
#if INET_CONFIG_ENABLE_IPV4
        case ReturnState::kStart:
            *type  = chip::Inet::IPAddressType::kIPv6;
            mState = ReturnState::kIPv4;
            return true;
        case ReturnState::kIPv4:
            *type  = chip::Inet::IPAddressType::kIPv4;
            mState = ReturnState::kEnd;
            return true;
#else
        case ReturnState::kStart:
            *type  = chip::Inet::IPAddressType::kIPv6;
            mState = ReturnState::kEnd;
            return true;
#endif
        case ReturnState::kEnd:
        default:
            return false;
        }
    }

private:
    ReturnState mState;
};

class AllAddressesIterator : public mdns::Minimal::IpAddressIterator
{
public:
    AllAddressesIterator(chip::Inet::InterfaceId interfaceId, chip::Inet::IPAddressType addrType) :
        mInterfaceIdFilter(interfaceId), mAddrType(addrType)
    {}

    bool Next(chip::Inet::IPAddress & dest)
    {
        while (true)
        {
            if (!mIterator.HasCurrent())
            {
                return false;
            }

            if (mIterator.GetInterfaceId() != mInterfaceIdFilter)
            {
                mIterator.Next();
                continue;
            }

            CHIP_ERROR err = mIterator.GetAddress(dest);
            mIterator.Next();

            if (mAddrType != chip::Inet::IPAddressType::kAny)
            {
                if (dest.Type() != mAddrType)
                {
                    continue;
                }
            }

            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Discovery, "Failed to fetch interface IP address: %" CHIP_ERROR_FORMAT, err.Format());
                continue;
            }

            return true;
        }
    }

private:
    const chip::Inet::InterfaceId mInterfaceIdFilter;
    const chip::Inet::IPAddressType mAddrType;
    chip::Inet::InterfaceAddressIterator mIterator;
};

class DefaultAddressPolicy : public AddressPolicy
{
public:
    chip::Platform::UniquePtr<ListenIterator> GetListenEndpoints() override
    {
        return chip::Platform::UniquePtr<ListenIterator>(chip::Platform::New<SimpleInterfacesIterator>());
    }

    chip::Platform::UniquePtr<IpAddressIterator> GetIpAddressesForEndpoint(chip::Inet::InterfaceId interfaceId,
                                                                           chip::Inet::IPAddressType type) override
    {
        return chip::Platform::UniquePtr<IpAddressIterator>(chip::Platform::New<AllAddressesIterator>(interfaceId, type));
    }
};

DefaultAddressPolicy gDefaultAddressPolicy;

} // namespace

void SetDefaultAddressPolicy()
{
    SetAddressPolicy(&gDefaultAddressPolicy);
}

} // namespace Minimal
} // namespace mdns

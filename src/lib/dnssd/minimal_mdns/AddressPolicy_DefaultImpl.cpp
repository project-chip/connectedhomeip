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

/// Checks if the current interface is powered on
/// and not local loopback.
template <typename T>
bool IsCurrentInterfaceUsable(T & iterator)
{
    if (!iterator.IsUp())
    {
        return false; // not a usable interface
    }
    char name[chip::Inet::InterfaceId::kMaxIfNameLength];
    if (iterator.GetInterfaceName(name, sizeof(name)) != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to get interface name.");
        return false;
    }

    // TODO: need a better way to ignore local loopback interfaces/addresses
    // We do not want to listen on local loopback even though they are up and
    // support multicast
    //
    // Some way to detect 'is local looback' that is smarter (e.g. at least
    // strict string compare on linux instead of substring) would be better.
    //
    // This would reject likely valid interfaces like 'lollipop' or 'lostinspace'
    if (strncmp(name, "lo", 2) == 0)
    {
        /// local loopback interface is not usable by MDNS
        return false;
    }
    return true;
}

class AllInterfaces : public mdns::Minimal::ListenIterator
{
public:
    AllInterfaces() { SkipToFirstValidInterface(); }

    bool Next(chip::Inet::InterfaceId * id, chip::Inet::IPAddressType * type) override
    {
        if (!mIterator.HasCurrent())
        {
            return false;
        }

#if INET_CONFIG_ENABLE_IPV4
        if (mState == State::kIpV4)
        {
            mState = State::kIpV6;

            if (CurrentInterfaceHasAddressOfType(chip::Inet::IPAddressType::kIPv4))
            {
                *id   = mIterator.GetInterfaceId();
                *type = chip::Inet::IPAddressType::kIPv4;
                return true;
            }
        }
#endif

#if INET_CONFIG_ENABLE_IPV4
        mState = State::kIpV4;
#endif

        bool haveResult = CurrentInterfaceHasAddressOfType(chip::Inet::IPAddressType::kIPv6);
        if (haveResult)
        {
            *id   = mIterator.GetInterfaceId();
            *type = chip::Inet::IPAddressType::kIPv6;
        }

        for (mIterator.Next(); SkipCurrentInterface(); mIterator.Next())
        {
        }

        if (haveResult)
        {
            return true;
        }

        return Next(id, type);
    }

private:
#if INET_CONFIG_ENABLE_IPV4
    enum class State
    {
        kIpV4,
        kIpV6,
    };
    State mState = State::kIpV4;
#endif
    chip::Inet::InterfaceIterator mIterator;

    void SkipToFirstValidInterface()
    {
        do
        {
            if (!SkipCurrentInterface())
            {
                break;
            }
        } while (mIterator.Next());
    }

    bool SkipCurrentInterface()
    {
        if (!mIterator.HasCurrent())
        {
            return false; // nothing to try.
        }

        return !IsCurrentInterfaceUsable(mIterator);
    }

    bool CurrentInterfaceHasAddressOfType(chip::Inet::IPAddressType type);
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

bool AllInterfaces::CurrentInterfaceHasAddressOfType(chip::Inet::IPAddressType type)
{
    // mIterator.HasCurrent() must be true here.
    AllAddressesIterator addressIter(mIterator.GetInterfaceId(), type);
    chip::Inet::IPAddress addr;
    if (addressIter.Next(addr))
    {
        return true;
    }

    return false;
}

class DefaultAddressPolicy : public AddressPolicy
{
public:
    chip::Platform::UniquePtr<ListenIterator> GetListenEndpoints() override
    {
        return chip::Platform::UniquePtr<ListenIterator>(chip::Platform::New<AllInterfaces>());
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

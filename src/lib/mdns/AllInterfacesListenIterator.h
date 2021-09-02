/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <stdio.h>

#include <lib/mdns/minimal/Server.h>

namespace chip {
namespace Mdns {

/// Checks if the current interface is powered on
/// and not local loopback.
template <typename T>
bool IsCurrentInterfaceUsable(T & iterator)
{
    if (!iterator.IsUp() || !iterator.SupportsMulticast())
    {
        return false; // not a usable interface
    }
    char name[chip::Inet::InterfaceIterator::kMaxIfNameLength];
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
private:
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
            *id    = mIterator.GetInterfaceId();
            *type  = chip::Inet::kIPAddressType_IPv4;
            mState = State::kIpV6;
            return true;
        }
#endif

        *id   = mIterator.GetInterfaceId();
        *type = chip::Inet::kIPAddressType_IPv6;
#if INET_CONFIG_ENABLE_IPV4
        mState = State::kIpV4;
#endif

        for (mIterator.Next(); SkipCurrentInterface(); mIterator.Next())
        {
        }
        return true;
    }

private:
    enum class State
    {
        kIpV4,
        kIpV6,
    };
#if INET_CONFIG_ENABLE_IPV4
    State mState = State::kIpV4;
#else
    State mState = State::kIpV6;
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
};

} // namespace Mdns
} // namespace chip

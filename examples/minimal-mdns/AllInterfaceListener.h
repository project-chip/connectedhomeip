/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <lib/mdns/minimal/Server.h>

namespace MdnsExample {

/// Returns
///   - NULL interface for IPv4
///   - all interfaces for IPv6
class AllInterfaces : public mdns::Minimal::ListenIterator
{
public:
    AllInterfaces(bool enableIpV4) : mState(enableIpV4 ? State::kIpV4 : State::kIpV6)
    {
        if (!enableIpV4)
        {
            SkipToFirstValidInterface();
        }
    }

    bool Next(chip::Inet::InterfaceId * id, chip::Inet::IPAddressType * type) override
    {
        if (mState == State::kIpV4)
        {
            *id    = INET_NULL_INTERFACEID;
            *type  = chip::Inet::kIPAddressType_IPv4;
            mState = State::kIpV6;

            SkipToFirstValidInterface();

            return true;
        }

        if (!mIterator.HasCurrent())
        {
            return false;
        }

        *id   = mIterator.GetInterfaceId();
        *type = chip::Inet::kIPAddressType_IPv6;

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
    State mState;
    chip::Inet::InterfaceIterator mIterator;

    void SkipToFirstValidInterface()
    {
        if (SkipCurrentInterface())
        {
            while (mIterator.Next())
            {
                if (!SkipCurrentInterface())
                {
                    break;
                }
            }
        }
    }

    bool SkipCurrentInterface()
    {
        if (!mIterator.HasCurrent())
        {
            return false; // nothing to try.
        }

        if (!mIterator.IsUp() || !mIterator.SupportsMulticast())
        {
            return true; // not a usable interface
        }

        char name[chip::Inet::InterfaceIterator::kMaxIfNameLength];
        if (mIterator.GetInterfaceName(name, sizeof(name)) != CHIP_NO_ERROR)
        {
            printf("!!!! FAILED TO GET INTERFACE NAME\n");
            return true;
        }

        if (strncmp(name, "lo", 2) == 0)
        {
            printf("Skipping interface '%s' (assume local loopback)\n", name);
            return true;
        }

        printf("Usable interface: %s (%d)\n", name, static_cast<int>(mIterator.GetInterfaceId()));

        return false;
    }
};

} // namespace MdnsExample

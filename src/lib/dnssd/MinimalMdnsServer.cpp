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
#include "MinimalMdnsServer.h"

namespace chip {
namespace Dnssd {
namespace {

using namespace mdns::Minimal;

class AllInterfaces : public ListenIterator
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
            *type  = chip::Inet::IPAddressType::kIPv4;
            mState = State::kIpV6;
            return true;
        }
#endif

        *id   = mIterator.GetInterfaceId();
        *type = chip::Inet::IPAddressType::kIPv6;
#if INET_CONFIG_ENABLE_IPV4
        mState = State::kIpV4;
#endif

        for (mIterator.Next(); SkipCurrentInterface(); mIterator.Next())
        {
        }
        return true;
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

        return !Internal::IsCurrentInterfaceUsable(mIterator);
    }
};

} // namespace

GlobalMinimalMdnsServer & GlobalMinimalMdnsServer::Instance()
{
    static GlobalMinimalMdnsServer _instance;
    return _instance;
}

CHIP_ERROR GlobalMinimalMdnsServer::StartServer(chip::Inet::EndPointManager<chip::Inet::UDPEndPoint> * udpEndPointManager,
                                                uint16_t port)
{
    GlobalMinimalMdnsServer::Server().Shutdown();
    AllInterfaces allInterfaces;
    return GlobalMinimalMdnsServer::Server().Listen(udpEndPointManager, &allInterfaces, port);
}

void GlobalMinimalMdnsServer::ShutdownServer()
{
    GlobalMinimalMdnsServer::Server().Shutdown();
}

} // namespace Dnssd
} // namespace chip

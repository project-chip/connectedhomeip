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

#pragma once

#include <platform/NetworkCommissioning.h>

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

class DarwinNetworkIterator : public NetworkIterator
{
public:
    DarwinNetworkIterator() {}

    ~DarwinNetworkIterator() override = default;

    size_t Count() override { return mInterfaceCount; }

    bool Next(Network & item) override
    {
        VerifyOrReturnError(mInterfaceCount, false);
        mInterfaceCount--;

        size_t interfaceNameLen = strlen(mInterfaceName);
        memcpy(item.networkID, mInterfaceName, interfaceNameLen);
        item.networkIDLen = static_cast<uint8_t>(interfaceNameLen);
        item.connected    = mInterfaceStatus;

        return true;
    }

    void Release() override { delete this; }

protected:
    size_t mInterfaceCount                                   = 0;
    char mInterfaceName[Inet::InterfaceId::kMaxIfNameLength] = {};
    bool mInterfaceStatus                                    = false;
};

class DarwinEthernetNetworkIterator final : public DarwinNetworkIterator
{
public:
    DarwinEthernetNetworkIterator()
    {
        auto & connectivityMgr = ConnectivityMgrImpl();
        ReturnOnFailure(connectivityMgr.GetEthernetInterfaceName(mInterfaceName, sizeof(mInterfaceName)));
        ReturnOnFailure(connectivityMgr.GetInterfaceStatus(mInterfaceName, &mInterfaceStatus));
        mInterfaceCount++;
    }
};

class DarwinEthernetDriver final : public EthernetDriver
{
public:
    // BaseDriver
    NetworkIterator * GetNetworks() override { return new DarwinEthernetNetworkIterator(); }

    // EthernetDriver
    uint8_t GetMaxNetworks() override { return 1; };
};

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip

/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
    size_t mInterfaceCount        = 0;
    char mInterfaceName[IFNAMSIZ] = {};
    bool mInterfaceStatus         = false;
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

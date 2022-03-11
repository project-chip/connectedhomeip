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

#include <jni.h>
#include <platform/NetworkCommissioning.h>
#include "DiagnosticDataProviderImpl.h"

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

class AndroidEthernetDriver final : public EthernetDriver
{
public:
    class EthernetNetworkIterator final : public NetworkIterator
    {
    public:
        EthernetNetworkIterator(AndroidEthernetDriver * aDriver) : mNetifs(aDriver->mNetifs) {}
        size_t Count() override;
        bool Next(Network & item) override;
        void Release() override { delete this; }
        ~EthernetNetworkIterator() = default;

    private:
        DeviceLayer::NetworkInterface * mNetifs = nullptr;
    };

    CHIP_ERROR Init() override { return DiagnosticDataProviderImpl().GetDefaultInstance().GetNetworkInterfaces(&mNetifs);};
    CHIP_ERROR Shutdown() override { DiagnosticDataProviderImpl().GetDefaultInstance().ReleaseNetworkInterfaces(mNetifs); return CHIP_NO_ERROR; }
    uint8_t GetMaxNetworks() override { return 4; }
    NetworkIterator * GetNetworks() override { return new EthernetNetworkIterator(this); }

private:
    DeviceLayer::NetworkInterface * mNetifs = nullptr;
    
};

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip

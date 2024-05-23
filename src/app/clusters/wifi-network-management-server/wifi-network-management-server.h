/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/Global.h>
#include <lib/support/Span.h>

void emberAfWiFiNetworkManagementClusterServerInitCallback(chip::EndpointId);

namespace chip {
namespace app {
namespace Clusters {
namespace WiFiNetworkManagement {

class Server : private AttributeAccessInterface, private CommandHandlerInterface
{
public:
    static Server & Instance();

    CHIP_ERROR ClearNetworkCredentials();
    CHIP_ERROR SetNetworkCredentials(ByteSpan ssid, ByteSpan passphrase);

private:
    friend Global<Server>;
    friend void ::emberAfWiFiNetworkManagementClusterServerInitCallback(chip::EndpointId);

    Server();
    ~Server();
    CHIP_ERROR Init(EndpointId endpoint);

    Server(Server const &)             = delete;
    Server & operator=(Server const &) = delete;

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    void InvokeCommand(HandlerContext & handlerContext) override;

    void HandleNetworkPassphraseRequest(HandlerContext & ctx, const Commands::NetworkPassphraseRequest::DecodableType & req);

    EndpointId mEndpointId = kInvalidEndpointId;

    uint8_t mSsid[32];
    uint8_t mSsidLen = 0;
    static_assert(std::numeric_limits<decltype(mSsidLen)>::max() >= sizeof(mSsid));
    ByteSpan SsidSpan() const { return ByteSpan(mSsid, mSsidLen); }

    Crypto::SensitiveDataBuffer<64> mPassphrase;
    ByteSpan PassphraseSpan() const { return mPassphrase.Span(); }

    bool HaveNetworkCredentials() { return mSsidLen > 0; }
};

} // namespace WiFiNetworkManagement
} // namespace Clusters
} // namespace app
} // namespace chip

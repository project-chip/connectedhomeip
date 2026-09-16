/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <AppMain.h>
#include <app/clusters/network-identity-management-server/AuthenticatorDriver.h>
#include <app/clusters/network-identity-management-server/DefaultNetworkIdentityStorage.h>
#include <app/clusters/network-identity-management-server/NetworkIdentityManagementCluster.h>
#include <app/clusters/network-identity-management-server/NetworkIdentityManagementTestEventTriggerHandler.h>
#include <app/clusters/network-identity-management-server/RawKeyNetworkIdentityKeystore.h>
#include <app/clusters/thread-border-router-management-server/thread-border-router-management-server.h>
#include <app/clusters/thread-network-directory-server/thread-network-directory-server.h>
#include <app/clusters/wifi-network-management-server/wifi-network-management-server.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>

#if MATTER_ENABLE_UBUS
#include "ThreadBROpenThreadUbus.h"
#include "UbusManager.h"
#else
#include "ThreadBRFake.h"
#endif

#include <optional>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

#if MATTER_ENABLE_UBUS
ubus::UbusManager gUbusManager{};
#endif

std::optional<DefaultThreadNetworkDirectoryServer> gThreadNetworkDirectoryServer;
void emberAfThreadNetworkDirectoryClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(!gThreadNetworkDirectoryServer);
    TEMPORARY_RETURN_IGNORED gThreadNetworkDirectoryServer.emplace(endpoint).Init();
}

std::optional<WiFiNetworkManagementServer> gWiFiNetworkManagementServer;
void emberAfWiFiNetworkManagementClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(!gWiFiNetworkManagementServer);
    TEMPORARY_RETURN_IGNORED gWiFiNetworkManagementServer.emplace(endpoint).Init();
}

std::optional<ThreadBorderRouterManagement::ServerInstance> gThreadBorderRouterManagementServer;
void emberAfThreadBorderRouterManagementClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(!gThreadBorderRouterManagementServer);
#if MATTER_ENABLE_UBUS
    static OpenThreadUbusBorderRouterDelegate delegate{ gUbusManager };
#else
    static FakeBorderRouterDelegate delegate{};
#endif
    TEMPORARY_RETURN_IGNORED gThreadBorderRouterManagementServer
        .emplace(endpoint, &delegate, Server::GetInstance().GetFailSafeContext())
        .Init();
}

// AuthenticatorDriver for standalone testing. There is no real PDC authenticator, so the driver
// is passive during normal operation. It additionally exposes AuthenticateClient(), which a test
// event trigger uses to simulate a client authenticating against the current Network Identity
// (see NetworkIdentityManagementTestEventTriggerHandler.h and TC-NETIM-1.4).
class PdcTestAuthenticatorDriver : public NetworkIdentityManagement::AuthenticatorDriver
{
public:
    void OnStartup(NetworkIdentityManagement::AuthenticatorDriverCallback & callback,
                   ReadOnlyNetworkIdentityStorage & storage) override
    {
        mCallback = &callback;
        mStorage  = &storage;
    }

    void OnShutdown() override
    {
        mCallback = nullptr;
        mStorage  = nullptr;
    }

    // Simulate a successful authentication of the given client against the current ECDSA Network
    // Identity, so the client references that identity and prevents it from being retired.
    CHIP_ERROR AuthenticateClient(uint16_t clientIndex)
    {
        VerifyOrReturnError(mCallback != nullptr && mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReadOnlyNetworkIdentityStorage::NetworkIdentityEntry entry;
        ReturnErrorOnFailure(mStorage->FindCurrentNetworkIdentity(NetworkIdentityManagement::IdentityTypeEnum::kEcdsa, entry,
                                                                  BitFlags<ReadOnlyNetworkIdentityStorage::NetworkIdentityFlags>(),
                                                                  MutableByteSpan()));
        mCallback->OnClientAuthenticated(clientIndex, entry.index);
        return CHIP_NO_ERROR;
    }

private:
    NetworkIdentityManagement::AuthenticatorDriverCallback * mCallback = nullptr;
    ReadOnlyNetworkIdentityStorage * mStorage                          = nullptr;
};

std::optional<DefaultNetworkIdentityStorage> gNetworkIdentityStorage;
Crypto::RawKeyNetworkIdentityKeystore gNetworkIdentityKeystore;
PdcTestAuthenticatorDriver gPdcTestAuthenticatorDriver;
LazyRegisteredServerCluster<NetworkIdentityManagementCluster> gNetworkIdentityManagementCluster;

void emberAfNetworkIdentityManagementClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(!gNetworkIdentityManagementCluster.IsConstructed());
    gNetworkIdentityStorage.emplace(Server::GetInstance().GetPersistentStorage());
    gNetworkIdentityManagementCluster.Create(endpoint, *gNetworkIdentityStorage, gNetworkIdentityKeystore,
                                             gPdcTestAuthenticatorDriver);
    SuccessOrDie(CodegenDataModelProvider::Instance().Registry().Register(gNetworkIdentityManagementCluster.Registration()));
}

// Implements the test event trigger declared in NetworkIdentityManagementTestEventTriggerHandler.h.
// The low 16 bits of the trigger value carry the ClientIndex to authenticate against the current NI.
bool HandleNetworkIdentityManagementTestEventTrigger(uint64_t eventTrigger)
{
    constexpr uint64_t kClientIndexMask = 0xFFFF;
    if ((eventTrigger & ~kClientIndexMask) !=
        static_cast<uint64_t>(NetworkIdentityManagementTrigger::kAuthenticateClientAgainstCurrentIdentity))
    {
        return false; // Not a Network Identity Management trigger.
    }

    auto clientIndex = static_cast<uint16_t>(eventTrigger & kClientIndexMask);
    CHIP_ERROR err   = gPdcTestAuthenticatorDriver.AuthenticateClient(clientIndex);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "NETIM test trigger: failed to authenticate client %u: %" CHIP_ERROR_FORMAT, clientIndex, err.Format());
    }
    return err == CHIP_NO_ERROR;
}

static void ApplicationEarlyInit()
{
#if MATTER_ENABLE_UBUS
    SuccessOrDie(gUbusManager.Init());
#endif
}

void ApplicationInit()
{
    TEMPORARY_RETURN_IGNORED gWiFiNetworkManagementServer->SetNetworkCredentials(ByteSpan::fromCharSpan("MatterAP"_span),
                                                                                 ByteSpan::fromCharSpan("Setec Astronomy"_span));
}

void ApplicationShutdown()
{
#if MATTER_ENABLE_UBUS
    gUbusManager.Shutdown();
#endif
}

int main(int argc, char * argv[])
{
    VerifyOrReturnValue(ChipLinuxAppInit(argc, argv) == 0, -1);
    ApplicationEarlyInit();
    ChipLinuxAppMainLoop();
    return 0;
}

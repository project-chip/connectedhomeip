/**
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>
#include <platform/ConnectivityManager.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::GeneralDiagnostics::Attributes;
using chip::DeviceLayer::ConnectivityMgr;
using chip::DeviceLayer::PlatformManager;

namespace {

class GeneralDiagosticsAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the GeneralDiagnostics cluster on all endpoints.
    GeneralDiagosticsAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), GeneralDiagnostics::Id) {}

    CHIP_ERROR Read(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    template <typename T>
    CHIP_ERROR ReadIfSupported(CHIP_ERROR (PlatformManager::*getter)(T &), AttributeValueEncoder & aEncoder);

#if CHIP_CLUSTER_CONFIG_ENABLE_COMPLEX_ATTRIBUTE_READ
    CHIP_ERROR ReadNetworkInterfaces(AttributeValueEncoder & aEncoder);
#endif
};

template <typename T>
CHIP_ERROR GeneralDiagosticsAttrAccess::ReadIfSupported(CHIP_ERROR (PlatformManager::*getter)(T &),
                                                        AttributeValueEncoder & aEncoder)
{
    T data;
    CHIP_ERROR err = (DeviceLayer::PlatformMgr().*getter)(data);
    if (err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        data = 0;
    }
    else if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    return aEncoder.Encode(data);
}

#if CHIP_CLUSTER_CONFIG_ENABLE_COMPLEX_ATTRIBUTE_READ
CHIP_ERROR GeneralDiagosticsAttrAccess::ReadNetworkInterfaces(AttributeValueEncoder & aEncoder)
{
    DeviceLayer::NetworkInterface * netifs;

    ReturnErrorOnFailure(ConnectivityMgr().GetNetworkInterfaces(&netifs));

    CHIP_ERROR err = aEncoder.EncodeList([&netifs](const TagBoundEncoder & encoder) -> CHIP_ERROR {
        for (DeviceLayer::NetworkInterface * ifp = netifs; ifp != nullptr; ifp = ifp->Next)
        {
            GeneralDiagnostics::Structs::NetworkInterfaceType::Type networkInterface;

            networkInterface.name                            = ByteSpan(Uint8::from_char(ifp->Name), strlen(ifp->Name));
            networkInterface.fabricConnected                 = ifp->FabricConnected;
            networkInterface.offPremiseServicesReachableIPv4 = ifp->OffPremiseServicesReachableIPv4;
            networkInterface.offPremiseServicesReachableIPv6 = ifp->OffPremiseServicesReachableIPv6;
            networkInterface.type                            = static_cast<GeneralDiagnostics::InterfaceType>(ifp->Type);

            if (ifp->Is64MacAddress)
            {
                // The Hardware Address is a 64-bit IEEE MAC Address (e.g. EUI-64).
                networkInterface.hardwareAddress = ByteSpan(Uint8::from_char(ifp->HardwareAddress), 8);
            }
            else
            {
                // The Hardware Address is a 48-bit IEEE MAC Address.
                networkInterface.hardwareAddress = ByteSpan(Uint8::from_char(ifp->HardwareAddress), 6);
            }

            ReturnErrorOnFailure(encoder.Encode(networkInterface));
        }

        return CHIP_NO_ERROR;
    });

    ConnectivityMgr().ReleaseNetworkInterfaces(&netifs);

    return err;
}
#endif

GeneralDiagosticsAttrAccess gAttrAccess;

CHIP_ERROR GeneralDiagosticsAttrAccess::Read(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    if (aPath.mClusterId != GeneralDiagnostics::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    switch (aPath.mAttributeId)
    {
#if CHIP_CLUSTER_CONFIG_ENABLE_COMPLEX_ATTRIBUTE_READ
    case NetworkInterfaces::Id: {
        return ReadNetworkInterfaces(aEncoder);
    }
#endif
    case RebootCount::Id: {
        return ReadIfSupported(&PlatformManager::GetRebootCount, aEncoder);
    }
    case UpTime::Id: {
        return ReadIfSupported(&PlatformManager::GetUpTime, aEncoder);
    }
    case TotalOperationalHours::Id: {
        return ReadIfSupported(&PlatformManager::GetTotalOperationalHours, aEncoder);
    }
    case BootReasons::Id: {
        return ReadIfSupported(&PlatformManager::GetBootReasons, aEncoder);
    }
    default: {
        break;
    }
    }
    return CHIP_NO_ERROR;
}
} // anonymous namespace

void emberAfGeneralDiagnosticsClusterServerInitCallback(EndpointId endpoint)
{
    static bool attrAccessRegistered = false;
    if (!attrAccessRegistered)
    {
        registerAttributeAccessOverride(&gAttrAccess);
        attrAccessRegistered = true;
    }
}

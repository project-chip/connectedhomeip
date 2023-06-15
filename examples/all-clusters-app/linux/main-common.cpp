/*
*
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

#include "main-common.h"
#include "AllClustersCommandDelegate.h"
#include "WindowCoveringManager.h"
#include "include/tv-callbacks.h"
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/clusters/mode-select-server/mode-select-server.h>
#include "../all-clusters-common/include/mode-select-delegates.h"
#include <app/server/Server.h>
#include <new>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/PlatformManager.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SessionManager.h>
#include <transport/raw/PeerAddress.h>

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
#include <platform/Darwin/NetworkCommissioningDriver.h>
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/Darwin/WiFi/NetworkCommissioningWiFiDriver.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN

#if CHIP_DEVICE_LAYER_TARGET_LINUX
#include <platform/Linux/NetworkCommissioningDriver.h>
#endif // CHIP_DEVICE_LAYER_TARGET_LINUX

#include <Options.h>

using namespace chip;
using namespace chip::app;
using namespace chip::DeviceLayer;

namespace {

constexpr const char kChipEventFifoPathPrefix[] = "/tmp/chip_all_clusters_fifo_";
LowPowerManager sLowPowerManager;
NamedPipeCommands sChipNamedPipeCommands;
AllClustersCommandDelegate sAllClustersCommandDelegate;
chip::app::Clusters::WindowCovering::WindowCoveringManager sWindowCoveringManager;

} // namespace

void OnIdentifyStart(::Identify *)
{
    ChipLogProgress(Zcl, "OnIdentifyStart");
}

void OnIdentifyStop(::Identify *)
{
    ChipLogProgress(Zcl, "OnIdentifyStop");
}

void OnTriggerEffect(::Identify * identify)
{
    switch (identify->mCurrentEffectIdentifier)
    {
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE");
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
        return;
    }
}

static Identify gIdentify0 = {
    chip::EndpointId{ 0 }, OnIdentifyStart, OnIdentifyStop, EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED, OnTriggerEffect,
};

static Identify gIdentify1 = {
    chip::EndpointId{ 1 }, OnIdentifyStart, OnIdentifyStop, EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED, OnTriggerEffect,
};

// Network commissioning
namespace {
// This file is being used by platforms other than Linux, so we need this check to disable related features since we only
// implemented them on linux.
constexpr EndpointId kNetworkCommissioningEndpointMain      = 0;
constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;

#if CHIP_DEVICE_LAYER_TARGET_LINUX
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
NetworkCommissioning::LinuxThreadDriver sThreadDriver;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
NetworkCommissioning::LinuxWiFiDriver sWiFiDriver;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

NetworkCommissioning::LinuxEthernetDriver sEthernetDriver;
#endif // CHIP_DEVICE_LAYER_TARGET_LINUX

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
NetworkCommissioning::DarwinWiFiDriver sWiFiDriver;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

NetworkCommissioning::DarwinEthernetDriver sEthernetDriver;
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
Clusters::NetworkCommissioning::Instance sThreadNetworkCommissioningInstance(kNetworkCommissioningEndpointMain, &sThreadDriver);
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
Clusters::NetworkCommissioning::Instance sWiFiNetworkCommissioningInstance(kNetworkCommissioningEndpointSecondary, &sWiFiDriver);
#endif

Clusters::NetworkCommissioning::Instance sEthernetNetworkCommissioningInstance(kNetworkCommissioningEndpointMain, &sEthernetDriver);

class ExampleDeviceInstanceInfoProvider : public DeviceInstanceInfoProvider
{
public:
    void Init(DeviceInstanceInfoProvider * defaultProvider) { mDefaultProvider = defaultProvider; }

    CHIP_ERROR GetVendorName(char * buf, size_t bufSize) override { return mDefaultProvider->GetVendorName(buf, bufSize); }
    CHIP_ERROR GetVendorId(uint16_t & vendorId) override { return mDefaultProvider->GetVendorId(vendorId); }
    CHIP_ERROR GetProductName(char * buf, size_t bufSize) override { return mDefaultProvider->GetProductName(buf, bufSize); }
    CHIP_ERROR GetProductId(uint16_t & productId) override { return mDefaultProvider->GetProductId(productId); }
    CHIP_ERROR GetPartNumber(char * buf, size_t bufSize) override { return mDefaultProvider->GetPartNumber(buf, bufSize); }
    CHIP_ERROR GetProductURL(char * buf, size_t bufSize) override { return mDefaultProvider->GetPartNumber(buf, bufSize); }
    CHIP_ERROR GetProductLabel(char * buf, size_t bufSize) override { return mDefaultProvider->GetProductLabel(buf, bufSize); }
    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize) override { return mDefaultProvider->GetSerialNumber(buf, bufSize); }
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day) override
    {
        return mDefaultProvider->GetManufacturingDate(year, month, day);
    }
    CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion) override
    {
        return mDefaultProvider->GetHardwareVersion(hardwareVersion);
    }
    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize) override
    {
        return mDefaultProvider->GetHardwareVersionString(buf, bufSize);
    }
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) override
    {
        return mDefaultProvider->GetRotatingDeviceIdUniqueId(uniqueIdSpan);
    }
    CHIP_ERROR GetProductFinish(Clusters::BasicInformation::ProductFinishEnum * finish) override;
    CHIP_ERROR GetProductPrimaryColor(Clusters::BasicInformation::ColorEnum * primaryColor) override;

private:
    DeviceInstanceInfoProvider * mDefaultProvider;
};

CHIP_ERROR ExampleDeviceInstanceInfoProvider::GetProductFinish(Clusters::BasicInformation::ProductFinishEnum * finish)
{
    // Our example device claims to have a Satin finish for now.  We can make
    // this configurable as needed.
    *finish = Clusters::BasicInformation::ProductFinishEnum::kSatin;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleDeviceInstanceInfoProvider::GetProductPrimaryColor(Clusters::BasicInformation::ColorEnum * primaryColor)
{
    // Our example device claims to have a nice purple color for now.  We can
    // make this configurable as needed.
    *primaryColor = Clusters::BasicInformation::ColorEnum::kPurple;
    return CHIP_NO_ERROR;
}

ExampleDeviceInstanceInfoProvider gExampleDeviceInstanceInfoProvider;

} // namespace

// Mode select clusters
namespace {

    Clusters::ModeSelect::ModeSelectDelegate modeSelectDelegate;
    Clusters::ModeSelect::Instance modeSelectInstance(0x1, Clusters::ModeSelect::Id, &modeSelectDelegate);

//    // RVC Run
//    constexpr SemanticTagStructType semanticTagsIdle[]     = { { .value = static_cast<uint16_t>(Clusters::RvcRun::SemanticTags::kIdle) } };
//    constexpr SemanticTagStructType semanticTagsCleaning[] = { { .value = static_cast<uint16_t>(Clusters::RvcRun::SemanticTags::kCleaning) } };
//
//    std::vector<ModeOptionStructType> rvcRunOptions = {
//        chip::app::Clusters::ModeSelect::Delegate::BuildModeOptionStruct("Idle", Clusters::RvcRun::ModeIdle, List<const SemanticTagStructType>(semanticTagsIdle)),
//        chip::app::Clusters::ModeSelect::Delegate::BuildModeOptionStruct("Cleaning", Clusters::RvcRun::ModeCleaning, List<const SemanticTagStructType>(semanticTagsCleaning)),
//        chip::app::Clusters::ModeSelect::Delegate::BuildModeOptionStruct("Mapping", Clusters::RvcRun::ModeMapping, noSemanticTags),
//    };
//    Clusters::ModeSelect::RvcRunDelegate RvcRunDelegate(rvcRunOptions);
//    Clusters::ModeSelect::Instance rvcRunInstance(0x1, Clusters::RvcRun::Id, &RvcRunDelegate);
//
//    // RVC Clean
//    constexpr SemanticTagStructType semanticTagsVac[]     = { { .value = static_cast<uint16_t>(Clusters::RvcClean::SemanticTags::kVacuum) } };
//    constexpr SemanticTagStructType semanticTagsMop[] = { { .value = static_cast<uint16_t>(Clusters::RvcClean::SemanticTags::kMop) } };
//    constexpr SemanticTagStructType semanticTagsBoost[] = { { .value = static_cast<uint16_t>(Clusters::ModeSelect::SemanticTags::kMax) },
//                                                              { .value = static_cast<uint16_t>(Clusters::RvcClean::SemanticTags::kDeepClean) }};
//
//    std::vector<ModeOptionStructType> rvcCleanOptions = {
//        chip::app::Clusters::ModeSelect::Delegate::BuildModeOptionStruct("Vacuum", Clusters::RvcClean::ModeVacuum, List<const SemanticTagStructType>(semanticTagsVac)),
//        chip::app::Clusters::ModeSelect::Delegate::BuildModeOptionStruct("Wash", Clusters::RvcClean::ModeWash, List<const SemanticTagStructType>(semanticTagsMop)),
//        chip::app::Clusters::ModeSelect::Delegate::BuildModeOptionStruct("Deep clean", Clusters::RvcClean::ModeDeepClean, List<const SemanticTagStructType>(semanticTagsBoost)),
//    };
//    Clusters::ModeSelect::RvcCleanDelegate RvcCleanDelegate(rvcCleanOptions);
//    Clusters::ModeSelect::Instance rvcCleanInstance(0x1, Clusters::RvcClean::Id, &RvcCleanDelegate);
//
//    // Dishwasher Mode Select
//    constexpr SemanticTagStructType semanticTagsNormal[] = { { .value = static_cast<uint16_t>(Clusters::DishwasherModeSelect::SemanticTags::kNormal) } };
//    constexpr SemanticTagStructType semanticTagsHeavy[] = {{ .value = static_cast<uint16_t>(Clusters::ModeSelect::SemanticTags::kMax) },
//                                                             { .value = static_cast<uint16_t>(Clusters::DishwasherModeSelect::SemanticTags::kHeavy) }};
//    constexpr SemanticTagStructType semanticTagsLight[] = { { .value = static_cast<uint16_t>(Clusters::DishwasherModeSelect::SemanticTags::kLight) },
//                                                            { .value = static_cast<uint16_t>(Clusters::ModeSelect::SemanticTags::kNight) },
//                                                            { .value = static_cast<uint16_t>(Clusters::ModeSelect::SemanticTags::kQuiet) }};
//
//    std::vector<ModeOptionStructType> dishwasherModeSelectOptions = {
//        chip::app::Clusters::ModeSelect::Delegate::BuildModeOptionStruct("Normal", Clusters::DishwasherModeSelect::ModeNormal, List<const SemanticTagStructType>(semanticTagsNormal)),
//        chip::app::Clusters::ModeSelect::Delegate::BuildModeOptionStruct("Heavy", Clusters::DishwasherModeSelect::ModeHeavy, List<const SemanticTagStructType>(semanticTagsHeavy)),
//        chip::app::Clusters::ModeSelect::Delegate::BuildModeOptionStruct("Light", Clusters::DishwasherModeSelect::ModeLight, List<const SemanticTagStructType>(semanticTagsLight)),
//    };
//    Clusters::ModeSelect::DishwasherControlDelegate DishwasherModeSelectDelegate(dishwasherModeSelectOptions);
//    Clusters::ModeSelect::Instance dishwasherModeSelectInstance(0x1, Clusters::DishwasherModeSelect::Id, &DishwasherModeSelectDelegate);
//
//    // Laundry Washer
//    constexpr SemanticTagStructType semanticTagsLaundryNormal[] = { { .value = static_cast<uint16_t>(Clusters::LaundryWasher::SemanticTags::kNormal) } };
//    constexpr SemanticTagStructType semanticTagsLaundryDelicate[] = { { .value = static_cast<uint16_t>(Clusters::LaundryWasher::SemanticTags::kDelicate) },
//                                                             { .value = static_cast<uint16_t>(Clusters::ModeSelect::SemanticTags::kNight) },
//                                                             { .value = static_cast<uint16_t>(Clusters::ModeSelect::SemanticTags::kQuiet) }};
//    constexpr SemanticTagStructType semanticTagsLaundryHeavy[] = {{ .value = static_cast<uint16_t>(Clusters::ModeSelect::SemanticTags::kMax) },
//                                                             { .value = static_cast<uint16_t>(Clusters::LaundryWasher::SemanticTags::kHeavy) }};
//    constexpr SemanticTagStructType semanticTagsLaundryWhites[] = {{ .value = static_cast<uint16_t>(Clusters::LaundryWasher::SemanticTags::kWhites) }};
//
//    std::vector<ModeOptionStructType> laundryWasherOptions = {
//        chip::app::Clusters::ModeSelect::Delegate::BuildModeOptionStruct("Normal", Clusters::LaundryWasher::ModeNormal, List<const SemanticTagStructType>(semanticTagsLaundryNormal)),
//        chip::app::Clusters::ModeSelect::Delegate::BuildModeOptionStruct("Delicate", Clusters::LaundryWasher::ModeDelicate, List<const SemanticTagStructType>(semanticTagsLaundryDelicate)),
//        chip::app::Clusters::ModeSelect::Delegate::BuildModeOptionStruct("Heavy", Clusters::LaundryWasher::ModeHeavy, List<const SemanticTagStructType>(semanticTagsLaundryHeavy)),
//        chip::app::Clusters::ModeSelect::Delegate::BuildModeOptionStruct("Whites", Clusters::LaundryWasher::ModeWhites, List<const SemanticTagStructType>(semanticTagsLaundryWhites)),
//    };
//    Clusters::ModeSelect::LaundryWasherDelegate LaundryWasherDelegate(laundryWasherOptions);
//    Clusters::ModeSelect::Instance laundryWasherInstance(0x1, Clusters::LaundryWasher::Id, &LaundryWasherDelegate);
//
//    // Refrigerator And Temperature Controlled Cabinet
//    constexpr SemanticTagStructType semanticTagsTccNormal[] = { { .value = static_cast<uint16_t>(Clusters::ModeSelect::SemanticTags::kAuto) } };
//    constexpr SemanticTagStructType semanticTagsTccRapidCool[] = { { .value = static_cast<uint16_t>(Clusters::RefrigeratorAndTemperatureControlledCabinet::SemanticTags::kRapidCool) }};
//    constexpr SemanticTagStructType semanticTagsTccRapidFreeze[] = { { .value = static_cast<uint16_t>(Clusters::RefrigeratorAndTemperatureControlledCabinet::SemanticTags::kRapidFreeze) }};
//
//    std::vector<ModeOptionStructType> tccOptions = {
//        chip::app::Clusters::ModeSelect::Delegate::BuildModeOptionStruct("Normal", Clusters::RefrigeratorAndTemperatureControlledCabinet::ModeNormal, List<const SemanticTagStructType>(semanticTagsTccNormal)),
//        chip::app::Clusters::ModeSelect::Delegate::BuildModeOptionStruct("Rapid Cool", Clusters::RefrigeratorAndTemperatureControlledCabinet::ModeRapidCool, List<const SemanticTagStructType>(semanticTagsTccRapidCool)),
//        chip::app::Clusters::ModeSelect::Delegate::BuildModeOptionStruct("Rapid Freeze", Clusters::RefrigeratorAndTemperatureControlledCabinet::ModeRapidFreeze, List<const SemanticTagStructType>(semanticTagsTccRapidFreeze)),
//    };
//    Clusters::ModeSelect::TccDelegate TccDelegate(tccOptions);
//    Clusters::ModeSelect::Instance tccInstance(0x1, Clusters::RefrigeratorAndTemperatureControlledCabinet::Id, &TccDelegate);
}

void ApplicationInit()
{
    (void) kNetworkCommissioningEndpointMain;
    // Enable secondary endpoint only when we need it, this should be applied to all platforms.
    emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, false);

    const bool kThreadEnabled = {
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        LinuxDeviceOptions::GetInstance().mThread
#else
        false
#endif
    };

    const bool kWiFiEnabled = {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
        LinuxDeviceOptions::GetInstance().mWiFi
#else
        false
#endif
    };

    if (kThreadEnabled && kWiFiEnabled)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        sThreadNetworkCommissioningInstance.Init();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
        sWiFiNetworkCommissioningInstance.Init();
#endif
        // Only enable secondary endpoint for network commissioning cluster when both WiFi and Thread are enabled.
        emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, true);
    }
    else if (kThreadEnabled)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        sThreadNetworkCommissioningInstance.Init();
#endif
    }
    else if (kWiFiEnabled)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
        // If we only enable WiFi on this device, "move" WiFi instance to main NetworkCommissioning cluster endpoint.
        sWiFiNetworkCommissioningInstance.~Instance();
        new (&sWiFiNetworkCommissioningInstance)
            Clusters::NetworkCommissioning::Instance(kNetworkCommissioningEndpointMain, &sWiFiDriver);
        sWiFiNetworkCommissioningInstance.Init();
#endif
    }
    else
    {
        sEthernetNetworkCommissioningInstance.Init();
    }

    modeSelectInstance.Init();
//    rvcRunInstance.Init();
//    rvcCleanInstance.Init();
//    dishwasherModeSelectInstance.Init();
//    laundryWasherInstance.Init();
//    tccInstance.Init();

    std::string path = kChipEventFifoPathPrefix + std::to_string(getpid());

    if (sChipNamedPipeCommands.Start(path, &sAllClustersCommandDelegate) != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to start CHIP NamedPipeCommands");
        sChipNamedPipeCommands.Stop();
    }

    auto * defaultProvider = GetDeviceInstanceInfoProvider();
    if (defaultProvider != &gExampleDeviceInstanceInfoProvider)
    {
        gExampleDeviceInstanceInfoProvider.Init(defaultProvider);
        SetDeviceInstanceInfoProvider(&gExampleDeviceInstanceInfoProvider);
    }
}

void ApplicationExit()
{
    if (sChipNamedPipeCommands.Stop() != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to stop CHIP NamedPipeCommands");
    }
}

void emberAfLowPowerClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(NotSpecified, "Setting LowPower default delegate to global manager");
    chip::app::Clusters::LowPower::SetDefaultDelegate(endpoint, &sLowPowerManager);
}

void emberAfWindowCoveringClusterInitCallback(chip::EndpointId endpoint)
{
    sWindowCoveringManager.Init(endpoint);
    chip::app::Clusters::WindowCovering::SetDefaultDelegate(endpoint, &sWindowCoveringManager);
    chip::app::Clusters::WindowCovering::ConfigStatusUpdateFeatures(endpoint);
}

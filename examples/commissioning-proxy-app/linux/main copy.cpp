/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
 *    Copyright 2022-2024 Morse Micro
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
#include "ota-requestor.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app/util/endpoint-config-api.h>

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
#include <imgui_ui/ui.h>
#include <imgui_ui/windows/light.h>
#include <imgui_ui/windows/occupancy_sensing.h>
#include <imgui_ui/windows/qrcode.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY
#include "CommissioningProxyCommandDelegate.h"
#include "CommissioningProxyManager.h"
//#include "platform/morsemicro/mm_linux/ConnectivityManagerImpl.cpp"
//#include "app/EventManagement.h"
//#include "protocols/secure_channel/CASESession.h"
#endif

using namespace chip;
using namespace chip::app;
using DataModel::List;

namespace {

// The FIFO and CommandDelegate is used to provide local user control to the
// device.  See examples/lighting-app/linux/README.md for further information
constexpr const char kChipEventFifoPathPrefix[] = "/tmp/matter_app_fifo_";
NamedPipeCommands sChipNamedPipeCommands;
CommissioningProxyAppCommandDelegate sCommissioningProxyAppCommandDelegate;

// Name of the environment variable naming the script to execute to gather
// sensor data via the FIFO and CommandDelegate
#define SENSOR_ENV_NAME "MORSE_MICRO_MATTER_SENSOR_COMMAND"

// Name of the environment variable naming the script to execute to gather
// switch events via the FIFO and CommandDelegate
#define SWITCH_ENV_NAME "MORSE_MICRO_MATTER_SWITCH_COMMAND"

// identifyEffectStart is the time when an Identify function started.  If no
// such function is running, it is zero
time_t identifyEffectStart = 0;

// identifyEffectStop is the time when an Identify function is expected to
// finish.  If no finish is scheduled, it is zero.  The units of seconds are
// crude and so identifyEffectAnimation* is checked first to ensure that jitter
// does not result in more flashes than expected
// It may also be shortened by kFinishEffect to terminate the effect at the
// next animation.
time_t identifyEffectStop = 0;

// identifyEffectAnimationPeriodMs is currently fixed to 500ms which is
// adequate but a bit basic
uint16_t identifyEffectAnimationPeriodMs = 500;

// identifyEffectAnimations is the number of light toggles remaining for an
// active Identify function.  If no such function is running, it is expected to
// be zero
uint32_t identifyEffectAnimations = 0;
} // namespace


// OnIdentifyStop() terminates any Identify function in progress.  It may be
// invoked by the cluster for the basic Identify function, locally if a
// trigger-effect function has run its course, or by a trigger-effect function
// with the kStopEvent type.
//
// It does not cancel any outstanding animation timer so this may expire later
// on and find there is nothing to do.
void OnIdentifyStop(Identify * identify)
{
    if (identifyEffectStart > 0)
    {
        ChipLogProgress(Zcl, "%s: completing identify function", __func__);
        CommissioningProxyMgr().SetIdentifyLight(false);
        CommissioningProxyMgr().RestoreMainLight();
        identifyEffectStart = 0;
        identifyEffectStop = 0;
        identifyEffectAnimations = 0;
    }
}

// OnIdentifyAnimate() is used to "animate" trigger-effect functions.  At
// present this simply takes the form of toggling the visual indicator on or
// off.  It is called to start the animation and registers itself as a timer
// callback to schedule the next animation.
//
// The systemLayer argument is provided for the timer callback but it is not
// used.
void OnIdentifyAnimate(chip::System::Layer * systemLayer, void * appState)
{
    if (identifyEffectStart > 0)
    {
        if (identifyEffectStop > 0)
        {
            // Check the animation has not exceeded its expected duration
            time_t now = time(NULL);
            if (now > identifyEffectStop)
            {
                ChipLogProgress(Zcl, "%s: finished a timed effect", __func__);
                OnIdentifyStop((Identify *) appState);
                return;
            }

            // Check the animation has not exceeded its expected number of
            // animations
            if (identifyEffectAnimations == 0)
            {
                ChipLogProgress(Zcl, "%s: finished the expected number of animations", __func__);
                OnIdentifyStop((Identify *) appState);
                return;
            }

            identifyEffectAnimations--;
        }

        // Toggle the light to provide animation
        if (appState != nullptr)
        {
            // Use different color depending on the Endpoint from which the
            // request originated
            Identify *identify = (Identify *) appState;
            uint32_t color;

            switch(identify->mEndpoint)
            {
                case CommissioningProxyEndpoint:
                    color = 0x00FF9933; // Orange
                    break;
            }

            CommissioningProxyMgr().ToggleIdentifyLight(color);
        }
        else
        {
            CommissioningProxyMgr().ToggleIdentifyLight();
        }

        // Set timer to trigger the next animation
        chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds16(identifyEffectAnimationPeriodMs),
             OnIdentifyAnimate, appState);
    }
    else
    {
        // This can happen if the effect was terminated with Clusters::Identify::EffectIdentifierEnum::kStopEffect
        ChipLogDetail(Zcl, "%s: called with no identify in progress: ignore", __func__);
    }
}

// OnIdentifyStart() is invoked to start the basic unbounded Identify function
// which will be terminated later by OnIdentifyStop().
void OnIdentifyStart(Identify * identify)
{
    if (identifyEffectStart > 0)
    {
        ChipLogProgress(Zcl, "%s: identify function already in progress - ignoring", __func__);
        return;
    }

    ChipLogProgress(Zcl, "%s: identify function starting", __func__);
    identifyEffectStart = time(NULL);

    // Do not stop until commanded to do so
    identifyEffectStop = 0;

    // Start the identify pattern
    OnIdentifyAnimate(nullptr, identify);
}

// OnIdentifyTriggerEffect() is invoked to start a trigger-effect Identify
// function which has a fixed duration which is stored here.
//
// kFinishEffect and kStopEffect stop other effects rather than starting their
// own.
void OnIdentifyTriggerEffect(Identify * identify)
{
    // Called for the trigger-effect command
    // Starts an effect or stops one already in progress
    uint16_t blinks = 0;

    switch (identify->mCurrentEffectIdentifier)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBlink: Light is turned on/off once");
        blinks = 1;
        break;

    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBreathe: Light is turned on/off over 1 second and repeated 15 times");
        blinks = 15;
        break;

    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kOkay: Light flashes twice");
        blinks = 2;
        break;

    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kChannelChange: Light flashes for 8 seconds");
        blinks = 8;
        break;

    case Clusters::Identify::EffectIdentifierEnum::kFinishEffect:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kFinishEffect: complete the current stage of the effect neatly then stop");
        // Set the stop time to now so that the next animation will end the effect
        identifyEffectStop = time(NULL);
        return;

    case Clusters::Identify::EffectIdentifierEnum::kStopEffect:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kStopEffect: stop immediately");
        OnIdentifyStop(identify);
        return;

    default:
        ChipLogProgress(Zcl, "Identifier effect not recognised: Light is turned on/off once");
        blinks = 1;
        break;
    }

    if (blinks > 0)
    {
        if (identifyEffectStart > 0)
        {
            ChipLogProgress(Zcl, "%s: identify function already in progress - ignoring", __func__);
            return;
        }

        // Store the time the effect started
        identifyEffectStart = time(NULL);

        // Calculate the approximate time that the effect should cease as a
        // guard against animations taking longer than expected
        identifyEffectStop = identifyEffectStart + (((identifyEffectAnimationPeriodMs * 2) * blinks) / 1000);

        // Set the number of "animations" that are expected to take place -
        // currently representing a toggle of the state of the light.  This is
        // a guard against more animations being actioned than expected due to
        // time jitter
        identifyEffectAnimations = blinks * 2;

        // Start the identify pattern
        OnIdentifyAnimate(nullptr, identify);
    }
}

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    switch (attributePath.mClusterId)
    {
        case Clusters::OnOff::Id:
        {
            switch (attributePath.mAttributeId)
            {
                case Clusters::OnOff::Attributes::OnOff::Id:
                {
                    if (type != ZCL_BOOLEAN_ATTRIBUTE_TYPE)
                    {
                        ChipLogError(NotSpecified, "%s: OnOff attribute %#x (%u) has unexpected type %#x",
                            __func__, attributePath.mAttributeId, attributePath.mAttributeId, type);
                    }

                    CommissioningProxyMgr().InitiateAction(*value ? CommissioningProxyManager::ON_ACTION : CommissioningProxyManager::OFF_ACTION);
                    break;
                }

                default:
                {
                    ChipLogDetail(NotSpecified, "%s: OnOff attribute %#x (%u) updated",
                        __func__, attributePath.mAttributeId, attributePath.mAttributeId);
                    break;
                }
            }
            break;
        }
#if 0
        case Clusters::LevelControl::Id:
        {
            switch (attributePath.mAttributeId)
            {
                case Clusters::LevelControl::Attributes::CurrentLevel::Id:
                {
                    ChipLogDetail(NotSpecified, "%s: Set light level to %#x (%u)",
                        __func__, *value, *value);
                    CommissioningProxyMgr().SetMainLightLevel(*value);
                    break;
                }

                default:
                {
                    ChipLogDetail(NotSpecified, "%s: Level attribute %#x (%u) (type %#x size %u) updated",
                        __func__,
                        attributePath.mAttributeId, attributePath.mAttributeId,
                        type, size);
                    break;
                }
            }
            break;
        }
#endif
        case Clusters::TemperatureMeasurement::Id:
        {
            if ((type != ZCL_TEMPERATURE_ATTRIBUTE_TYPE) && (type != ZCL_INT16S_ATTRIBUTE_TYPE))
            {
                ChipLogError(NotSpecified, "%s: Temperature attribute %#x (%u) has unexpected type %#x",
                    __func__, attributePath.mAttributeId, attributePath.mAttributeId, type);
                break;
            }

            int16_t temperature;
            memcpy(&temperature, value, sizeof(int16_t));

            switch (attributePath.mAttributeId)
            {
                case Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Id:
                {
                    ChipLogDetail(NotSpecified, "%s: temperature %d.%d degrees Celsius",
                        __func__, temperature / 100, abs(temperature) % 100);
                    break;
                }

                case Clusters::TemperatureMeasurement::Attributes::MinMeasuredValue::Id:
                {
                    ChipLogDetail(NotSpecified, "%s: minimum temperature %d.%d degrees Celsius",
                        __func__, temperature / 100, abs(temperature) % 100);
                    break;
                }

                case Clusters::TemperatureMeasurement::Attributes::MaxMeasuredValue::Id:
                {
                    ChipLogDetail(NotSpecified, "%s: maximum temperature %d.%d degrees Celsius",
                        __func__, temperature / 100, abs(temperature) % 100);
                    break;
                }

                default:
                {
                    ChipLogDetail(NotSpecified, "%s: Temperature attribute %#x (%u) (type %#x size %u) updated",
                        __func__,
                        attributePath.mAttributeId, attributePath.mAttributeId,
                        type, size);
                    break;
                }
            }
            break;
        }

        case Clusters::RelativeHumidityMeasurement::Id:
        {
            if ((type != ZCL_PERCENT100THS_ATTRIBUTE_TYPE) && (type != ZCL_INT16U_ATTRIBUTE_TYPE))
            {
                ChipLogError(NotSpecified, "%s: Relative humidity attribute %#x (%u) has unexpected type %#x",
                    __func__, attributePath.mAttributeId, attributePath.mAttributeId, type);
                break;
            }

            uint16_t humidity;
            memcpy(&humidity, value, sizeof(uint16_t));

            switch (attributePath.mAttributeId)
            {
                case Clusters::RelativeHumidityMeasurement::Attributes::MeasuredValue::Id:
                {
                    ChipLogDetail(NotSpecified, "%s: relative humidity %d.%d %%",
                        __func__, humidity / 100, humidity % 100);
                    break;
                }

                case Clusters::RelativeHumidityMeasurement::Attributes::MinMeasuredValue::Id:
                {
                    ChipLogDetail(NotSpecified, "%s: minimum relative humidity %d.%d %%",
                        __func__, humidity / 100, humidity % 100);
                    break;
                }

                case Clusters::RelativeHumidityMeasurement::Attributes::MaxMeasuredValue::Id:
                {
                    ChipLogDetail(NotSpecified, "%s: maximum relative humidity %d.%d %%",
                        __func__, humidity / 100, humidity % 100);
                    break;
                }

                default:
                {
                    ChipLogDetail(NotSpecified, "%s: Relative humidity attribute %#x (%u) (type %#x size %u) updated",
                        __func__,
                        attributePath.mAttributeId, attributePath.mAttributeId,
                        type, size);
                    break;
                }
            }
            break;
        }

        default:
        {
            ChipLogDetail(NotSpecified, "%s: cluster %#x (%u) attribute %#x (%u) (type %#x size %u) updated",
                __func__, attributePath.mClusterId, attributePath.mClusterId,
                attributePath.mAttributeId, attributePath.mAttributeId, type, size);
            break;
        }
    }
}

/** @brief OnOff Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 * TODO Issue #3841
 * emberAfOnOffClusterInitCallback happens before the stack initialize the cluster
 * attributes to the default value.
 * The logic here expects something similar to the deprecated Plugins callback
 * emberAfPluginOnOffClusterServerPostInitCallback.
 *
 */
void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    // TODO: implement any additional Cluster Server init actions
}

void MatterCommissioningProxyPluginServerInitCallback()
{
    ChipLogError(NotSpecified, "=== %s() Entered", __func__);
    // Needed for linking
}

bool emberAfCommissioningProxyClusterProxyConnectRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::CommissioningProxy::Commands::ProxyConnectRequest::DecodableType & commandData)
{
    ChipLogError(NotSpecified, "=== %s() Received ProxyConnectRequest", __func__);
    return true;
}

bool emberAfCommissioningProxyClusterProxyDisconnectRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::CommissioningProxy::Commands::ProxyDisconnectRequest::DecodableType & commandData)
{
    ChipLogError(NotSpecified, "=== %s() Received ProxyDisconnectRequest", __func__);
    return true;
}

bool emberAfCommissioningProxyClusterProxyScanRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::CommissioningProxy::Commands::ProxyScanRequest::DecodableType & commandData)
{
    using ScanResultT = chip::app::Clusters::CommissioningProxy::Structs::ScanResultStruct::DecodableType;
    std::vector<ScanResultT> results;
    // ScanResultT r{};

    // Start PAF
    WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().Init(&DeviceLayer::SystemLayer());

     // TODO: parse these from commandData
    //chip::Optional<uint16_t> srvProtoType;  // set if you want to filter
    //uint32_t windowMs = 1500;

    std::vector<chip::DeviceLayer::NanPeerInfo> peers;
    //CHIP_ERROR err = chip::DeviceLayer::ConnectivityMgrImpl().ScanNanPublishers(
    //    serviceName, srvProtoType, windowMs, peers);

    ChipLogProgress(NotSpecified, "=== %s() Transport:0x%x WiFiBands:0x%x", __func__,
        static_cast<uint16_t>(commandData.transport.Raw()),
        static_cast<uint16_t>((commandData.wiFiBands.HasValue() ? commandData.wiFiBands.Value().Raw() : 0)));

    // Create a Handle and move it into ConnectivityManagerImpl
    // This keeps the ProxyScanRequest open, so the scan can complete before the ProxyScanResponse is sent
    CommandHandler::Handle handle(commandObj);
    CHIP_ERROR err = chip::DeviceLayer::ConnectivityMgrImpl()._WiFiPAFScan( std::move(handle), commandPath);
    if (err != CHIP_NO_ERROR) {
        commandObj->AddStatus(commandPath, chip::Protocols::InteractionModel::Status::Failure);
        return true;
    }

    ChipLogProgress(Controller, "===SHM %s() Before", __func__);
    // Ensure Response Timeout is greater than the ScanMaxTimeSeconds
    if (auto * ec = commandObj->GetExchangeContext())
    {
        ec->SetResponseTimeout(chip::System::Clock::Seconds16(140));
        ChipLogProgress(Controller, "===SHM %s() In", __func__);
    }

#if 0
    // Convert each NanPeerInfo -> ScanResultStruct::Type
    for (const auto & p : peers)
    {
        ScanResultT r{};
        r.address.SetNonNull(chip::ByteSpan(p.mac, 6));  // 6-byte MAC

        // If you want to include SSI as extended data:
        if (!p.ssi.empty()) {
            r.extendedData.SetNonNull(chip::ByteSpan(p.ssi.data(), p.ssi.size()));
        } else {
            r.extendedData.SetNull();
        }

        // Fill anything else you can/need. Examples:
        r.transport = chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap::kWiFiPAF;
        r.discriminator = 0; // unknown if you don't have it yet
        r.vendorId = static_cast<chip::VendorId>(0);
        r.productId = 0;

        results.push_back(r);
    }

    ChipLogProgress(NotSpecified, "=== %s() Received ProxyScanRequest", __func__);
    Clusters::CommissioningProxy::Commands::ProxyScanResponse::Type response;
    List<const ScanResultT> list{ Span<const ScanResultT>(results.data(), results.size()) };
    response.proxyScanResult = list;

    // response.numberOfResults = results.size();
    response.numberOfResults = static_cast<uint8_t>(response.proxyScanResult.size());

    commandObj->AddResponse(commandPath, response);
    commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);

//    commandObj->AddClusterSpecificFailure(commandPath,
  //                                        to_underlying(Clusters::CommissioningProxy::ProxyErrorEnum::kProxyBusy));
#endif

    return true;
}

bool emberAfCommissioningProxyClusterProxyMessageRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::CommissioningProxy::Commands::ProxyMessageRequest::DecodableType & commandData)
{
    ChipLogError(NotSpecified, "=== %s() Received ProxyMessageRequest", __func__);
    return true;
}

bool emberAfCommissioningProxyClusterProxyBackGroundScanStartRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::CommissioningProxy::Commands::ProxyBackGroundScanStartRequest::DecodableType & commandData)
{
    ChipLogError(NotSpecified, "=== %s() Received ProxyBackGroundScanStartRequest", __func__);

    // Use the NodeId and fabric Index as unique identifiers for the background scan
    FabricIndex fabricIndex = kUndefinedFabricIndex;
    NodeId localNodeId = kUndefinedNodeId;

    fabricIndex = commandObj->GetAccessingFabricIndex();
    if (IsValidFabricIndex(fabricIndex))
    {
        const auto * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
        if (fabricInfo != nullptr)
        {
            localNodeId = fabricInfo->GetNodeId();
        }
    }

    ChipLogProgress(AppServer,
                    "===SHM %s(): fabricIndex=%u localNodeId=0x" ChipLogFormatX64, __func__,
                    static_cast<unsigned>(fabricIndex), ChipLogValueX64(localNodeId));

    return true;
}

void ApplicationInit()
{
    std::string path = kChipEventFifoPathPrefix + std::to_string(getpid());

    if (sChipNamedPipeCommands.Start(path, &sCommissioningProxyAppCommandDelegate) != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to start CHIP NamedPipeCommands");
        sChipNamedPipeCommands.Stop();
    }

    // Initialize all OTA download components
    InitOTARequestor();

    ChipLogProgress(AppServer, "%s: Main function is Proxy Commissioner on endpoint %u",
            __func__, CommissioningProxyEndpoint);

    // Proxy Commissioner endpoint, and instantiate an Identify object for it
    // new Identify(chip::EndpointId{ CommissioningProxyEndpoint }, OnIdentifyStart, OnIdentifyStop,
    //     Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator, OnIdentifyTriggerEffect);
}

void ApplicationShutdown()
{
    if (sChipNamedPipeCommands.Stop() != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to stop CHIP NamedPipeCommands");
    }
}

#ifdef __NuttX__
// NuttX requires the main function to be defined with C-linkage. However, marking
// the main as extern "C" is not strictly conformant with the C++ standard. Since
// clang >= 20 such code triggers -Wmain warning.
extern "C" {
#endif

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv, GetOTARequestorOptions()) != 0)
    {
        return -1;
    }

    CHIP_ERROR err = CommissioningProxyMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to initialize Commissioning Proxy manager: %" CHIP_ERROR_FORMAT, err.Format());
        chip::DeviceLayer::PlatformMgr().Shutdown();
        return -1;
    }

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
    example::Ui::ImguiUi ui;

    ui.AddWindow(std::make_unique<example::Ui::Windows::QRCode>());
    ui.AddWindow(std::make_unique<example::Ui::Windows::OccupancySensing>(chip::EndpointId(1), "Occupancy"));
    ui.AddWindow(std::make_unique<example::Ui::Windows::Light>(chip::EndpointId(1)));

    ChipLinuxAppMainLoop(&ui);
#else
    ChipLinuxAppMainLoop();
#endif

    // If the event loop had been stopped due to an update being applied, boot into the new image
    (void) CheckOTARequestorState(argc, argv);
    return 0;
}

#ifdef __NuttX__
}
#endif

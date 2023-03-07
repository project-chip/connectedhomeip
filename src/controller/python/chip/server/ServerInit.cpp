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
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <platform/TestOnlyCommissionableDataProvider.h>

#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/af.h>

#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>

#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <credentials/examples/DeviceAttestationCredsExample.h>

// #include <support/CHIPMem.h>
// #include <support/ErrorStr.h>

// #include <examples/platform/linux/AppMain.h>

#include "Options.h"

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;

namespace {

void EventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    (void) arg;
    if (event->Type == chip::DeviceLayer::DeviceEventType::kCHIPoBLEConnectionEstablished)
    {
        ChipLogProgress(DeviceLayer, "Receive kCHIPoBLEConnectionEstablished");
    }
}

#if CHIP_DEVICE_LAYER_TARGET_LINUX && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
// uint32_t sBluetoothAdapterId = 0;
#endif

} // namespace

extern "C" {

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
/*
 * The device shall check every kWiFiStartCheckTimeUsec whether Wi-Fi management
 * has been fully initialized. If after kWiFiStartCheckAttempts Wi-Fi management
 * still hasn't been initialized, the device configuration is reset, and device
 * needs to be paired again.
 */
static constexpr useconds_t kWiFiStartCheckTimeUsec = 100 * 1000; // 100 ms
static constexpr uint8_t kWiFiStartCheckAttempts    = 5;
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
static bool EnsureWiFiIsStarted()
{
    for (int cnt = 0; cnt < kWiFiStartCheckAttempts; cnt++)
    {
        if (chip::DeviceLayer::ConnectivityMgrImpl().IsWiFiManagementStarted())
        {
            return true;
        }

        usleep(kWiFiStartCheckTimeUsec);
    }

    return chip::DeviceLayer::ConnectivityMgrImpl().IsWiFiManagementStarted();
}
#endif

void CleanShutdown()
{
    chip::DeviceLayer::PlatformMgr().StopEventLoopTask();

    chip::Server::GetInstance().Shutdown();
    chip::DeviceLayer::PlatformMgr().Shutdown();
}

using PostAttributeChangeCallback = void (*)(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t type,
                                             uint16_t size, uint8_t * value);

using PreAttributeChangeCallback = chip::Protocols::InteractionModel::Status (*)(EndpointId endpoint, ClusterId clusterId,
                                                                                 AttributeId attributeId, uint8_t type,
                                                                                 uint16_t size, uint8_t * value);

using ExternalAttributeReadCallback = EmberAfStatus (*)(EndpointId endpoint, ClusterId clusterId,
                                                        const EmberAfAttributeMetadata * attributeMetadata, void * buffer,
                                                        uint16_t maxReadLength);

using ExternalAttributeWriteCallback = EmberAfStatus (*)(EndpointId endpoint, ClusterId clusterId,
                                                         const EmberAfAttributeMetadata * attributeMetadata, void * buffer);

class PythonServerDelegate // : public ServerDelegate
{
public:
    void SetPreAttributeChangeCallback(PreAttributeChangeCallback cb) { mPreAttributeChangeCallback = cb; };
    void SetPostAttributeChangeCallback(PostAttributeChangeCallback cb) { mPostAttributeChangeCallback = cb; };
    void SetExternalAttributeReadCallback(ExternalAttributeReadCallback cb) { mExternalAttributeReadCallback = cb; };
    void SetExternalAttributeWriteCallback(ExternalAttributeWriteCallback cb) { mExternalAttributeWriteCallback = cb; };
    PreAttributeChangeCallback mPreAttributeChangeCallback         = nullptr;
    PostAttributeChangeCallback mPostAttributeChangeCallback       = nullptr;
    ExternalAttributeReadCallback mExternalAttributeReadCallback   = nullptr;
    ExternalAttributeWriteCallback mExternalAttributeWriteCallback = nullptr;
};

PythonServerDelegate gPythonServerDelegate;

void pychip_server_set_callbacks(PreAttributeChangeCallback pre, PostAttributeChangeCallback post,
                                 ExternalAttributeReadCallback read, ExternalAttributeWriteCallback write)
{
    // ChipLogProgress(NotSpecified, "setting cb");
    gPythonServerDelegate.SetPreAttributeChangeCallback(pre);
    gPythonServerDelegate.SetPostAttributeChangeCallback(post);
    gPythonServerDelegate.SetExternalAttributeReadCallback(read);
    gPythonServerDelegate.SetExternalAttributeWriteCallback(write);
}

void pychip_server_native_init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to initialize CHIP stack: memory init failed: %s", chip::ErrorStr(err));
    }

    err = chip::DeviceLayer::PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to initialize CHIP stack: platform init failed: %s", chip::ErrorStr(err));
    }

    static chip::DeviceLayer::TestOnlyCommissionableDataProvider TestOnlyCommissionableDataProvider;
    chip::DeviceLayer::SetCommissionableDataProvider(&TestOnlyCommissionableDataProvider);

    ConfigurationMgr().LogDeviceConfig();

    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));

#if defined(PW_RPC_ENABLED)
    chip::rpc::Init();
    ChipLogProgress(NotSpecified, "PW_RPC initialized.");
#endif // defined(PW_RPC_ENABLED)

    chip::DeviceLayer::PlatformMgrImpl().AddEventHandler(EventHandler, 0);

#if CONFIG_NETWORK_LAYER_BLE
    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName("RpiMatterDali"); // Use default device name (CHIP-XXXX)
    chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(LinuxDeviceOptions::GetInstance().mBleDevice, false);
    chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(true);
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    if (LinuxDeviceOptions::GetInstance().mWiFi)
    {
        chip::DeviceLayer::ConnectivityMgrImpl().StartWiFiManagement();
        if (!EnsureWiFiIsStarted())
        {
            ChipLogError(NotSpecified, "Wi-Fi Management taking too long to start - device configuration will be reset.");
        }
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

    // parts from ChipLinuxAppMainLoop

    // Init ZCL Data Model and CHIP App Server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.operationalServicePort        = CHIP_PORT;
    initParams.userDirectedCommissioningPort = CHIP_UDC_PORT;

    chip::Server::GetInstance().Init(initParams);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(chip::Credentials::Examples::GetExampleDACProvider());

    err = chip::DeviceLayer::PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to initialize CHIP stack: platform init failed: %s", chip::ErrorStr(err));
    }

    atexit(CleanShutdown);

    return /*err*/;
}
}

chip::Protocols::InteractionModel::Status MatterPreAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath,
                                                                           uint8_t type, uint16_t size, uint8_t * value)
{
    // ChipLogProgress(NotSpecified, "MatterPreAttributeChangeCallback()");
    if (gPythonServerDelegate.mPreAttributeChangeCallback != nullptr)
    {
        // ChipLogProgress(NotSpecified, "callback %p", gPythonServerDelegate.mPreAttributeChangeCallback);
        return gPythonServerDelegate.mPreAttributeChangeCallback(attributePath.mEndpointId, attributePath.mClusterId,
                                                                 attributePath.mAttributeId, type, size, value);
    }
    else
    {
        // ChipLogProgress(NotSpecified, "callback nullptr");
    }

    return chip::Protocols::InteractionModel::Status::Success;
};

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    // ChipLogProgress(NotSpecified, "MatterPostAttributeChangeCallback()");
    if (gPythonServerDelegate.mPostAttributeChangeCallback != nullptr)
    {
        // ChipLogProgress(NotSpecified, "callback %p", gPythonServerDelegate.mPostAttributeChangeCallback);
        gPythonServerDelegate.mPostAttributeChangeCallback(attributePath.mEndpointId, attributePath.mClusterId,
                                                           attributePath.mAttributeId, type, size, value);
    }
    else
    {
        // ChipLogProgress(NotSpecified, "callback nullptr");
    }
};

EmberAfStatus emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
                                                   const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                                   uint16_t maxReadLength)
{
    ChipLogProgress(NotSpecified, "emberAfExternalAttributeReadCallback()");
    if (gPythonServerDelegate.mExternalAttributeReadCallback != nullptr)
    {
        // ChipLogProgress(NotSpecified, "callback %p", gPythonServerDelegate.mExternalAttributeReadCallback);
        return gPythonServerDelegate.mExternalAttributeReadCallback(endpoint, clusterId, attributeMetadata, buffer, maxReadLength);
    }
    else
    {
        // ChipLogProgress(NotSpecified, "callback nullptr");
    }

    return EmberAfStatus::EMBER_ZCL_STATUS_FAILURE;
}

EmberAfStatus emberAfExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId,
                                                    const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    ChipLogProgress(NotSpecified, "emberAfExternalAttributeWriteCallback()");
    if (gPythonServerDelegate.mExternalAttributeWriteCallback != nullptr)
    {
        // ChipLogProgress(NotSpecified, "callback %p", gPythonServerDelegate.mExternalAttributeWriteCallback);
        return gPythonServerDelegate.mExternalAttributeWriteCallback(endpoint, clusterId, attributeMetadata, buffer);
    }
    else
    {
        // ChipLogProgress(NotSpecified, "callback nullptr");
    }

    return EmberAfStatus::EMBER_ZCL_STATUS_FAILURE;
}

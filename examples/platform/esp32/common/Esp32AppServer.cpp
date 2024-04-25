/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "Esp32AppServer.h"
#include "CHIPDeviceManager.h"
#include <app/InteractionModelEngine.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/clusters/ota-requestor/OTATestEventTriggerHandler.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <platform/ESP32/NetworkCommissioningDriver.h>
#if CONFIG_ENABLE_ICD_SERVER
#include <ICDSubscriptionCallback.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#if CONFIG_BT_ENABLED
#include "esp_bt.h"
#if CONFIG_BT_NIMBLE_ENABLED
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
#include "esp_nimble_hci.h"
#endif // ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
#include "nimble/nimble_port.h"
#endif // CONFIG_BT_NIMBLE_ENABLED
#endif // CONFIG_BT_ENABLED
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <string.h>

using namespace chip;
using namespace chip::Credentials;
using namespace chip::DeviceLayer;

static constexpr char TAG[] = "ESP32Appserver";

namespace {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
constexpr chip::EndpointId kNetworkCommissioningEndpointWiFi = 0xFFFE;
#else
constexpr chip::EndpointId kNetworkCommissioningEndpointWiFi = 0;
#endif
app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(kNetworkCommissioningEndpointWiFi, &(NetworkCommissioning::ESPWiFiDriver::GetInstance()));
#elif CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
static app::Clusters::NetworkCommissioning::Instance
    sEthernetNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::ESPEthernetDriver::GetInstance()));
#endif

#if CONFIG_TEST_EVENT_TRIGGER_ENABLED
static uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
                                                                                          0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
                                                                                          0xcc, 0xdd, 0xee, 0xff };
#endif
#if CONFIG_ENABLE_ICD_SERVER
static ICDSubscriptionCallback sICDSubscriptionHandler;
#endif
} // namespace

#if CONFIG_TEST_EVENT_TRIGGER_ENABLED
static int hex_digit_to_int(char hex)
{
    if ('A' <= hex && hex <= 'F')
    {
        return 10 + hex - 'A';
    }
    if ('a' <= hex && hex <= 'f')
    {
        return 10 + hex - 'a';
    }
    if ('0' <= hex && hex <= '9')
    {
        return hex - '0';
    }
    return -1;
}

static size_t hex_string_to_binary(const char * hex_string, uint8_t * buf, size_t buf_size)
{
    int num_char = strlen(hex_string);
    if (num_char != buf_size * 2)
    {
        return 0;
    }
    for (size_t i = 0; i < num_char; i += 2)
    {
        int digit0 = hex_digit_to_int(hex_string[i]);
        int digit1 = hex_digit_to_int(hex_string[i + 1]);

        if (digit0 < 0 || digit1 < 0)
        {
            return 0;
        }
        buf[i / 2] = (digit0 << 4) + digit1;
    }

    return buf_size;
}
#endif // CONFIG_TEST_EVENT_TRIGGER_ENABLED

void Esp32AppServer::DeInitBLEIfCommissioned(void)
{
#ifdef CONFIG_USE_BLE_ONLY_FOR_COMMISSIONING
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() > 0)
    {
        chip::DeviceLayer::Internal::BLEMgr().Shutdown();
    }
#endif /* CONFIG_USE_BLE_ONLY_FOR_COMMISSIONING */
}

void Esp32AppServer::Init(AppDelegate * sAppDelegate)
{
    // Init ZCL Data Model and CHIP App Server
    static chip::CommonCaseDeviceServerInitParams initParams;
#if CONFIG_TEST_EVENT_TRIGGER_ENABLED && CONFIG_ENABLE_OTA_REQUESTOR
    if (hex_string_to_binary(CONFIG_TEST_EVENT_TRIGGER_ENABLE_KEY, sTestEventTriggerEnableKey,
                             sizeof(sTestEventTriggerEnableKey)) == 0)
    {
        ChipLogError(DeviceLayer, "Failed to convert the EnableKey string to octstr type value");
        memset(sTestEventTriggerEnableKey, 0, sizeof(sTestEventTriggerEnableKey));
    }
    static SimpleTestEventTriggerDelegate sTestEventTriggerDelegate{};
    static OTATestEventTriggerHandler sOtaTestEventTriggerHandler{};
    VerifyOrDie(sTestEventTriggerDelegate.Init(ByteSpan(sTestEventTriggerEnableKey)) == CHIP_NO_ERROR);
    VerifyOrDie(sTestEventTriggerDelegate.AddHandler(&sOtaTestEventTriggerHandler) == CHIP_NO_ERROR);
    initParams.testEventTriggerDelegate = &sTestEventTriggerDelegate;
#endif // CONFIG_TEST_EVENT_TRIGGER_ENABLED
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    if (sAppDelegate != nullptr)
    {
        initParams.appDelegate = sAppDelegate;
    }
    chip::Server::GetInstance().Init(initParams);
#if CONFIG_ENABLE_ICD_SERVER
    // Register ICD subscription callback to match subscription max intervals to its idle time interval
    chip::app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&sICDSubscriptionHandler);
#endif // CONFIG_ENABLE_ICD_SERVER

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    sWiFiNetworkCommissioningInstance.Init();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (chip::DeviceLayer::ConnectivityMgr().IsThreadProvisioned() &&
        (chip::Server::GetInstance().GetFabricTable().FabricCount() != 0))
    {
        ChipLogProgress(DeviceLayer, "Thread has been provisioned, publish the dns service now");
        chip::app::DnssdServer::Instance().StartServer();
    }
#endif
    DeInitBLEIfCommissioned();
}

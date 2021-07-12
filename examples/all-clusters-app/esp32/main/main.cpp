/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "BluetoothWidget.h"
#include "Button.h"
#include "CHIPDeviceManager.h"
#include "DeviceCallbacks.h"
#include "Display.h"
#include "Globals.h"
#include "LEDWidget.h"
#include "ListScreen.h"
#include "QRCodeScreen.h"
#include "ScreenManager.h"
#include "WiFiWidget.h"
#include "esp_heap_caps_init.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "shell_extension/launch.h"

#include <cmath>
#include <cstdio>
#include <ctype.h>
#include <string>
#include <vector>

#include <app/common/gen/att-storage.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/server/AppDelegate.h>
#include <app/server/Mdns.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/af-types.h>
#include <app/util/af.h>
#include <lib/shell/Engine.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <support/CHIPMem.h>
#include <support/ErrorStr.h>

#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <app/clusters/temperature-measurement-server/temperature-measurement-server.h>

using namespace ::chip;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;

#define QRCODE_BASE_URL "https://dhrishi.github.io/connectedhomeip/qrcode.html"

#if CONFIG_DEVICE_TYPE_M5STACK

#define BUTTON_1_GPIO_NUM GPIO_NUM_39    // Left button on M5Stack
#define BUTTON_2_GPIO_NUM GPIO_NUM_38    // Middle button on M5Stack
#define BUTTON_3_GPIO_NUM GPIO_NUM_37    // Right button on M5Stack
#define STATUS_LED_GPIO_NUM GPIO_NUM_MAX // No status LED on M5Stack

#elif CONFIG_DEVICE_TYPE_ESP32_WROVER_KIT

#define STATUS_LED_GPIO_NUM GPIO_NUM_26

#elif CONFIG_DEVICE_TYPE_ESP32_DEVKITC

#define STATUS_LED_GPIO_NUM GPIO_NUM_2 // Use LED1 (blue LED) as status LED on DevKitC

#elif CONFIG_DEVICE_TYPE_ESP32_C3_DEVKITM

#define STATUS_LED_GPIO_NUM GPIO_NUM_8

#else // !CONFIG_DEVICE_TYPE_ESP32_DEVKITC

#error "Unsupported device type selected"

#endif // !CONFIG_DEVICE_TYPE_ESP32_DEVKITC

// Used to indicate that an IP address has been added to the QRCode
#define EXAMPLE_VENDOR_TAG_IP 1

const char * TAG = "all-clusters-app";

static DeviceCallbacks EchoCallbacks;

namespace {

#if CONFIG_DEVICE_TYPE_M5STACK

std::vector<Button> buttons          = { Button(), Button(), Button() };
std::vector<gpio_num_t> button_gpios = { BUTTON_1_GPIO_NUM, BUTTON_2_GPIO_NUM, BUTTON_3_GPIO_NUM };

#endif

// Pretend these are devices with endpoints with clusters with attributes
typedef std::tuple<std::string, std::string> Attribute;
typedef std::vector<Attribute> Attributes;
typedef std::tuple<std::string, Attributes> Cluster;
typedef std::vector<Cluster> Clusters;
typedef std::tuple<std::string, Clusters> Endpoint;
typedef std::vector<Endpoint> Endpoints;
typedef std::tuple<std::string, Endpoints> Device;
typedef std::vector<Device> Devices;
Devices devices;

void AddAttribute(std::string name, std::string value)
{
    Attribute attribute = std::make_tuple(std::move(name), std::move(value));
    std::get<1>(std::get<1>(std::get<1>(devices.back()).back()).back()).emplace_back(std::move(attribute));
}

void AddCluster(std::string name)
{
    Cluster cluster = std::make_tuple(std::move(name), std::move(Attributes()));
    std::get<1>(std::get<1>(devices.back()).back()).emplace_back(std::move(cluster));
}

void AddEndpoint(std::string name)
{
    Endpoint endpoint = std::make_tuple(std::move(name), std::move(Clusters()));
    std::get<1>(devices.back()).emplace_back(std::move(endpoint));
}

void AddDevice(std::string name)
{
    Device device = std::make_tuple(std::move(name), std::move(Endpoints()));
    devices.emplace_back(std::move(device));
}

#if CONFIG_DEVICE_TYPE_M5STACK

class EditAttributeListModel : public ListScreen::Model
{
    int deviceIndex;
    int endpointIndex;
    int clusterIndex;
    int attributeIndex;

public:
    EditAttributeListModel(int deviceIndex, int endpointIndex, int clusterIndex, int attributeIndex) :
        deviceIndex(deviceIndex), endpointIndex(endpointIndex), clusterIndex(clusterIndex), attributeIndex(attributeIndex)
    {}
    Attribute & attribute()
    {
        return std::get<1>(std::get<1>(std::get<1>(devices[deviceIndex])[endpointIndex])[clusterIndex])[attributeIndex];
    }
    bool IsBooleanAttribute()
    {
        auto & attribute = this->attribute();
        auto & value     = std::get<1>(attribute);
        return value == "On" || value == "Off";
    }
    virtual std::string GetTitle()
    {
        auto & attribute = this->attribute();
        auto & name      = std::get<0>(attribute);
        auto & value     = std::get<1>(attribute);
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%s : %s", name.c_str(), value.c_str());
        return buffer;
    }
    virtual int GetItemCount() { return IsBooleanAttribute() ? 1 : 2; }
    virtual std::string GetItemText(int i)
    {
        if (IsBooleanAttribute())
        {
            return "Toggle";
        }
        return i == 0 ? "+" : "-";
    }
    virtual void ItemAction(int i)
    {
        auto & attribute = this->attribute();
        auto & value     = std::get<1>(attribute);
        int n;
        if (sscanf(value.c_str(), "%d", &n) == 1)
        {
            auto & name = std::get<0>(attribute);

            ESP_LOGI(TAG, "editing attribute as integer: %d (%s)", n, i == 0 ? "+" : "-");
            n += (i == 0) ? 1 : -1;
            char buffer[32];
            sprintf(buffer, "%d", n);
            if (name == "Temperature")
            {
                // update the temp attribute here for hardcoded endpoint 1
                emberAfTemperatureMeasurementClusterSetMeasuredValueCallback(1, static_cast<int16_t>(n * 100));
            }
            value = buffer;
        }
        else if (IsBooleanAttribute())
        {
            auto & name    = std::get<0>(attribute);
            auto & cluster = std::get<0>(std::get<1>(std::get<1>(devices[deviceIndex])[endpointIndex])[i]);
            value          = (value == "On") ? "Off" : "On";

            if (name == "OnOff" && cluster == "OnOff")
            {
                uint8_t attributeValue = (value == "On") ? 1 : 0;
                emberAfWriteServerAttribute(endpointIndex + 1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID,
                                            (uint8_t *) &attributeValue, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
            }
        }
        else
        {
            auto & name    = std::get<0>(attribute);
            auto & cluster = std::get<0>(std::get<1>(std::get<1>(devices[deviceIndex])[endpointIndex])[i]);

            ESP_LOGI(TAG, "editing attribute as string: '%s' (%s)", value.c_str(), i == 0 ? "+" : "-");
            value = (value == "Closed") ? "Open" : "Closed";
            ESP_LOGI(TAG, "name and cluster: '%s' (%s)", name.c_str(), cluster.c_str());
            if (name == "State" && cluster == "Lock")
            {
                // update the doorlock attribute here
                uint8_t attributeValue = value == "Closed" ? EMBER_ZCL_DOOR_LOCK_STATE_LOCKED : EMBER_ZCL_DOOR_LOCK_STATE_UNLOCKED;
                emberAfWriteServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, ZCL_LOCK_STATE_ATTRIBUTE_ID,
                                            (uint8_t *) &attributeValue, ZCL_INT8U_ATTRIBUTE_TYPE);
            }
        }
    }
};

class AttributeListModel : public ListScreen::Model
{
    int deviceIndex;
    int endpointIndex;
    int clusterIndex;

public:
    AttributeListModel(int deviceIndex, int endpointIndex, int clusterIndex) :
        deviceIndex(deviceIndex), endpointIndex(endpointIndex), clusterIndex(clusterIndex)
    {}
    virtual std::string GetTitle() { return "Attributes"; }
    virtual int GetItemCount()
    {
        return std::get<1>(std::get<1>(std::get<1>(devices[deviceIndex])[endpointIndex])[clusterIndex]).size();
    }
    virtual std::string GetItemText(int i)
    {
        auto & attribute = std::get<1>(std::get<1>(std::get<1>(devices[deviceIndex])[endpointIndex])[clusterIndex])[i];
        auto & name      = std::get<0>(attribute);
        auto & value     = std::get<1>(attribute);
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%s : %s", name.c_str(), value.c_str());
        return buffer;
    }
    virtual void ItemAction(int i)
    {
        ESP_LOGI(TAG, "Opening attribute %d", i);
        ScreenManager::PushScreen(chip::Platform::New<ListScreen>(
            chip::Platform::New<EditAttributeListModel>(deviceIndex, endpointIndex, clusterIndex, i)));
    }
};

class ClusterListModel : public ListScreen::Model
{
    int deviceIndex;
    int endpointIndex;

public:
    ClusterListModel(int deviceIndex, int endpointIndex) : deviceIndex(deviceIndex), endpointIndex(endpointIndex) {}
    virtual std::string GetTitle() { return "Clusters"; }
    virtual int GetItemCount() { return std::get<1>(std::get<1>(devices[deviceIndex])[endpointIndex]).size(); }
    virtual std::string GetItemText(int i) { return std::get<0>(std::get<1>(std::get<1>(devices[deviceIndex])[endpointIndex])[i]); }
    virtual void ItemAction(int i)
    {
        ESP_LOGI(TAG, "Opening cluster %d", i);
        ScreenManager::PushScreen(
            chip::Platform::New<ListScreen>(chip::Platform::New<AttributeListModel>(deviceIndex, endpointIndex, i)));
    }
};

class EndpointListModel : public ListScreen::Model
{
    int deviceIndex;

public:
    EndpointListModel(int deviceIndex) : deviceIndex(deviceIndex) {}
    virtual std::string GetTitle() { return "Endpoints"; }
    virtual int GetItemCount() { return std::get<1>(devices[deviceIndex]).size(); }
    virtual std::string GetItemText(int i) { return std::get<0>(std::get<1>(devices[deviceIndex])[i]); }
    virtual void ItemAction(int i)
    {
        ESP_LOGI(TAG, "Opening endpoint %d", i);
        ScreenManager::PushScreen(chip::Platform::New<ListScreen>(chip::Platform::New<ClusterListModel>(deviceIndex, i)));
    }
};

class DeviceListModel : public ListScreen::Model
{
public:
    virtual std::string GetTitle() { return "Devices"; }
    virtual int GetItemCount() { return devices.size(); }
    virtual std::string GetItemText(int i) { return std::get<0>(devices[i]); }
    virtual void ItemAction(int i)
    {
        ESP_LOGI(TAG, "Opening device %d", i);
        ScreenManager::PushScreen(chip::Platform::New<ListScreen>(chip::Platform::New<EndpointListModel>(i)));
    }
};

class SetupListModel : public ListScreen::Model
{
public:
    SetupListModel()
    {
        std::string resetWiFi              = "Reset WiFi";
        std::string resetToFactory         = "Reset to factory";
        std::string forceWifiCommissioning = "Force WiFi commissioning";
        options.emplace_back(resetWiFi);
        options.emplace_back(resetToFactory);
        options.emplace_back(forceWifiCommissioning);
    }
    virtual std::string GetTitle() { return "Setup"; }
    virtual int GetItemCount() { return options.size(); }
    virtual std::string GetItemText(int i) { return options.at(i); }
    virtual void ItemAction(int i)
    {
        ESP_LOGI(TAG, "Opening options %d: %s", i, GetItemText(i).c_str());
        if (i == 0)
        {
            ConnectivityMgr().ClearWiFiStationProvision();
            OpenDefaultPairingWindow(ResetAdmins::kYes);
        }
        else if (i == 1)
        {
            ConfigurationMgr().InitiateFactoryReset();
        }
        else if (i == 2)
        {
            app::Mdns::AdvertiseCommissionableNode();
            OpenDefaultPairingWindow(ResetAdmins::kYes, PairingWindowAdvertisement::kMdns);
        }
    }

private:
    std::vector<std::string> options;
};

class CustomScreen : public Screen
{
public:
    virtual void Display()
    {
        TFT_drawCircle(0.3 * DisplayWidth, 0.3 * DisplayHeight, 8, TFT_BLUE);
        TFT_drawCircle(0.7 * DisplayWidth, 0.3 * DisplayHeight, 8, TFT_BLUE);
        TFT_drawLine(0.2 * DisplayWidth, 0.6 * DisplayHeight, 0.3 * DisplayWidth, 0.7 * DisplayHeight, TFT_BLUE);
        TFT_drawLine(0.3 * DisplayWidth, 0.7 * DisplayHeight, 0.7 * DisplayWidth, 0.7 * DisplayHeight, TFT_BLUE);
        TFT_drawLine(0.7 * DisplayWidth, 0.7 * DisplayHeight, 0.8 * DisplayWidth, 0.6 * DisplayHeight, TFT_BLUE);
    }
};

#endif // CONFIG_DEVICE_TYPE_M5STACK

void SetupInitialLevelControlValues(chip::EndpointId endpointId)
{
    uint8_t level = UINT8_MAX;

    emberAfWriteAttribute(endpointId, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, CLUSTER_MASK_SERVER, &level,
                          ZCL_INT8U_ATTRIBUTE_TYPE);
}

void SetupPretendDevices()
{
    AddDevice("Watch");
    AddEndpoint("Default");
    AddCluster("Battery");
    AddAttribute("Level", "89");
    AddAttribute("Voltage", "490");
    AddAttribute("Amperage", "501");
    AddCluster("Heart Monitor");
    AddAttribute("BPM", "72");
    AddCluster("Step Counter");
    AddAttribute("Steps", "9876");

    AddDevice("Light Bulb");
    AddEndpoint("1");
    AddCluster("OnOff");
    AddAttribute("OnOff", "Off");
    AddCluster("Level Control");
    AddAttribute("Current Level", "255");
    AddEndpoint("2");
    AddCluster("OnOff");
    AddAttribute("OnOff", "Off");
    AddCluster("Level Control");
    AddAttribute("Current Level", "255");

    AddDevice("Thermometer");
    AddEndpoint("External");
    AddCluster("Thermometer");
    AddAttribute("Temperature", "21");
    // write the temp attribute
    emberAfTemperatureMeasurementClusterSetMeasuredValueCallback(1, static_cast<int16_t>(21 * 100));

    AddDevice("Door Lock");
    AddEndpoint("Default");
    AddCluster("Lock");
    AddAttribute("State", "Open");
    // write the door lock state
    uint8_t attributeValue = EMBER_ZCL_DOOR_LOCK_STATE_UNLOCKED;
    emberAfWriteServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, ZCL_LOCK_STATE_ATTRIBUTE_ID, &attributeValue,
                                ZCL_INT8U_ATTRIBUTE_TYPE);
    AddDevice("Garage 1");
    AddEndpoint("Door 1");
    AddCluster("Door");
    AddAttribute("State", "Closed");
    AddEndpoint("Door 2");
    AddCluster("Door");
    AddAttribute("State", "Closed");
    AddEndpoint("Door 3");
    AddCluster("Door");
    AddAttribute("State", "Open");

    AddDevice("Garage 2");
    AddEndpoint("Door 1");
    AddCluster("Door");
    AddAttribute("State", "Open");
    AddEndpoint("Door 2");
    AddCluster("Door");
    AddAttribute("State", "Closed");
}

void GetGatewayIP(char * ip_buf, size_t ip_len)
{
    esp_netif_ip_info_t ipInfo;
    esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ipInfo);
    esp_ip4addr_ntoa(&ipInfo.ip, ip_buf, ip_len);
    ESP_LOGI(TAG, "Got gateway ip %s", ip_buf);
}

bool isRendezvousBLE()
{
    RendezvousInformationFlags flags = RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE);
    return flags.Has(RendezvousInformationFlag::kBLE);
}

std::string createSetupPayload()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    std::string result;

    uint16_t discriminator;
    err = ConfigurationMgr().GetSetupDiscriminator(discriminator);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Couldn't get discriminator: %s", ErrorStr(err));
        return result;
    }
    ESP_LOGI(TAG, "Setup discriminator: %u (0x%x)", discriminator, discriminator);

    uint32_t setupPINCode;
    err = ConfigurationMgr().GetSetupPinCode(setupPINCode);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Couldn't get setupPINCode: %s", ErrorStr(err));
        return result;
    }
    ESP_LOGI(TAG, "Setup PIN code: %u (0x%x)", setupPINCode, setupPINCode);

    uint16_t vendorId;
    err = ConfigurationMgr().GetVendorId(vendorId);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Couldn't get vendorId: %s", ErrorStr(err));
        return result;
    }

    uint16_t productId;
    err = ConfigurationMgr().GetProductId(productId);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Couldn't get productId: %s", ErrorStr(err));
        return result;
    }

    SetupPayload payload;
    payload.version               = 0;
    payload.discriminator         = discriminator;
    payload.setUpPINCode          = setupPINCode;
    payload.rendezvousInformation = RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE);
    payload.vendorID              = vendorId;
    payload.productID             = productId;

    if (!isRendezvousBLE())
    {
        char gw_ip[INET6_ADDRSTRLEN];
        GetGatewayIP(gw_ip, sizeof(gw_ip));
        payload.addOptionalVendorData(EXAMPLE_VENDOR_TAG_IP, gw_ip);

        QRCodeSetupPayloadGenerator generator(payload);

        size_t tlvDataLen = sizeof(gw_ip);
        uint8_t tlvDataStart[tlvDataLen];
        err = generator.payloadBase38Representation(result, tlvDataStart, tlvDataLen);
    }
    else
    {
        QRCodeSetupPayloadGenerator generator(payload);
        err = generator.payloadBase38Representation(result);
    }

    {
        ManualSetupPayloadGenerator generator(payload);
        std::string outCode;

        if (generator.payloadDecimalStringRepresentation(outCode) == CHIP_NO_ERROR)
        {
            ESP_LOGI(TAG, "Short Manual(decimal) setup code: %s", outCode.c_str());
        }
        else
        {
            ESP_LOGE(TAG, "Failed to get decimal setup code");
        }

        payload.commissioningFlow = CommissioningFlow::kCustom;
        generator                 = ManualSetupPayloadGenerator(payload);

        if (generator.payloadDecimalStringRepresentation(outCode) == CHIP_NO_ERROR)
        {
            // intentional extra space here to align the log with the short code
            ESP_LOGI(TAG, "Long Manual(decimal) setup code:  %s", outCode.c_str());
        }
        else
        {
            ESP_LOGE(TAG, "Failed to get decimal setup code");
        }
    }

    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Couldn't get payload string %d", err);
    }
    return result;
};

WiFiWidget pairingWindowLED;

class AppCallbacks : public AppDelegate
{
public:
    void OnReceiveError() override { statusLED1.BlinkOnError(); }
    void OnRendezvousStarted() override { bluetoothLED.Set(true); }
    void OnRendezvousStopped() override
    {
        bluetoothLED.Set(false);
        pairingWindowLED.Set(false);
    }
    void OnPairingWindowOpened() override { pairingWindowLED.Set(true); }
    void OnPairingWindowClosed() override { pairingWindowLED.Set(false); }
};

} // namespace

extern "C" void app_main()
{
    ESP_LOGI(TAG, "All Clusters Demo!");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    ESP_LOGI(TAG, "This is ESP32 chip with %d CPU cores, WiFi%s%s, ", chip_info.cores,
             (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "", (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    ESP_LOGI(TAG, "silicon revision %d, ", chip_info.revision);

    ESP_LOGI(TAG, "%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
             (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    CHIP_ERROR err; // A quick note about errors: CHIP adopts the error type and numbering
                    // convention of the environment into which it is ported.  Thus esp_err_t
                    // and CHIP_ERROR are in fact the same type, and both ESP-IDF errors
                    // and CHIO-specific errors can be stored in the same value without
                    // ambiguity.  For convenience, ESP_OK and CHIP_NO_ERROR are mapped
                    // to the same value.

    // Initialize the ESP NVS layer.
    err = nvs_flash_init();
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "nvs_flash_init() failed: %s", ErrorStr(err));
        return;
    }

#if CONFIG_ENABLE_CHIP_SHELL
    chip::LaunchShell();
#endif // CONFIG_ENABLE_CHIP_SHELL

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    err = deviceMgr.Init(&EchoCallbacks);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "device.Init() failed: %s", ErrorStr(err));
        return;
    }

    statusLED1.Init(STATUS_LED_GPIO_NUM);
    // Our second LED doesn't map to any physical LEDs so far, just to virtual
    // "LED"s on devices with screens.
    statusLED2.Init(GPIO_NUM_MAX);
    bluetoothLED.Init();
    wifiLED.Init();
    pairingWindowLED.Init();

    // Init ZCL Data Model and CHIP App Server
    AppCallbacks callbacks;
    InitServer(&callbacks);

    SetupPretendDevices();
    SetupInitialLevelControlValues(/* endpointId = */ 1);
    SetupInitialLevelControlValues(/* endpointId = */ 2);

    std::string qrCodeText = createSetupPayload();
    ESP_LOGI(TAG, "QR CODE Text: '%s'", qrCodeText.c_str());

    {
        std::vector<char> qrCode(3 * qrCodeText.size() + 1);
        err = EncodeQRCodeToUrl(qrCodeText.c_str(), qrCodeText.size(), qrCode.data(), qrCode.max_size());
        if (err == CHIP_NO_ERROR)
        {
            ESP_LOGI(TAG, "Copy/paste the below URL in a browser to see the QR CODE:\n\t%s?data=%s", QRCODE_BASE_URL,
                     qrCode.data());
        }
    }

#if CONFIG_HAVE_DISPLAY
    // Initialize the display device.
    err = InitDisplay();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "InitDisplay() failed: %s", ErrorStr(err));
        return;
    }

    // Initialize the screen manager
    ScreenManager::Init();

#if CONFIG_DEVICE_TYPE_M5STACK
    // Initialize the buttons.
    for (int i = 0; i < buttons.size(); ++i)
    {
        err = buttons[i].Init(button_gpios[i], 50);
        if (err != CHIP_NO_ERROR)
        {
            ESP_LOGE(TAG, "Button.Init() failed: %s", ErrorStr(err));
            return;
        }
    }

    // Push a rudimentary user interface.
    ScreenManager::PushScreen(chip::Platform::New<ListScreen>(
        (chip::Platform::New<SimpleListModel>())
            ->Title("CHIP")
            ->Action([](int i) { ESP_LOGI(TAG, "action on item %d", i); })
            ->Item("Devices",
                   []() {
                       ESP_LOGI(TAG, "Opening device list");
                       ScreenManager::PushScreen(chip::Platform::New<ListScreen>(chip::Platform::New<DeviceListModel>()));
                   })
            ->Item("Custom",
                   []() {
                       ESP_LOGI(TAG, "Opening custom screen");
                       ScreenManager::PushScreen(chip::Platform::New<CustomScreen>());
                   })
            ->Item("QR Code",
                   [=]() {
                       ESP_LOGI(TAG, "Opening QR code screen");
                       ESP_LOGI(TAG, "QR CODE Text: '%s'", qrCodeText.c_str());
                       uint16_t discriminator;
                       if (ConfigurationMgr().GetSetupDiscriminator(discriminator) == CHIP_NO_ERROR)
                       {
                           ESP_LOGI(TAG, "Setup discriminator: %u (0x%x)", discriminator, discriminator);
                       }
                       uint32_t setupPINCode;
                       if (ConfigurationMgr().GetSetupPinCode(setupPINCode) == CHIP_NO_ERROR)
                       {
                           ESP_LOGI(TAG, "Setup PIN code: %u (0x%x)", setupPINCode, setupPINCode);
                       }
                       ScreenManager::PushScreen(chip::Platform::New<QRCodeScreen>(qrCodeText));
                   })
            ->Item("Setup",
                   [=]() {
                       ESP_LOGI(TAG, "Opening Setup list");
                       ScreenManager::PushScreen(chip::Platform::New<ListScreen>(chip::Platform::New<SetupListModel>()));
                   })
            ->Item("More")
            ->Item("Items")
            ->Item("For")
            ->Item("Demo")));

#elif CONFIG_DEVICE_TYPE_ESP32_WROVER_KIT

    // Display the QR Code
    QRCodeScreen qrCodeScreen(qrCodeText);
    qrCodeScreen.Display();

#endif

    // Connect the status LED to VLEDs.
    {
        int vled1 = ScreenManager::AddVLED(TFT_GREEN);
        int vled2 = ScreenManager::AddVLED(TFT_RED);
        statusLED1.SetVLED(vled1, vled2);

        int vled3 = ScreenManager::AddVLED(TFT_CYAN);
        int vled4 = ScreenManager::AddVLED(TFT_ORANGE);
        statusLED2.SetVLED(vled3, vled4);

        bluetoothLED.SetVLED(ScreenManager::AddVLED(TFT_BLUE));
        wifiLED.SetVLED(ScreenManager::AddVLED(TFT_YELLOW));
        pairingWindowLED.SetVLED(ScreenManager::AddVLED(TFT_ORANGE));
    }

#endif // CONFIG_HAVE_DISPLAY

    // Run the UI Loop
    while (true)
    {
#if CONFIG_DEVICE_TYPE_M5STACK
        // TODO consider refactoring this example to use FreeRTOS tasks

        bool woken = false;

        // Poll buttons, possibly wake screen.
        for (int i = 0; i < buttons.size(); ++i)
        {
            if (buttons[i].Poll())
            {
                if (!woken)
                {
                    woken = WakeDisplay();
                }
                if (woken)
                {
                    continue;
                }
                if (buttons[i].IsPressed())
                {
                    ScreenManager::ButtonPressed(1 + i);
                }
            }
        }

#endif // CONFIG_DEVICE_TYPE_M5STACK

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

bool lowPowerClusterSleep()
{
    return true;
}

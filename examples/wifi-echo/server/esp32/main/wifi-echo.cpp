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
#include "DataModelHandler.h"
#include "Display.h"
#include "EchoDeviceCallbacks.h"
#include "LEDWidget.h"
#include "ListScreen.h"
#include "QRCodeScreen.h"
#include "RendezvousSession.h"
#include "ScreenManager.h"
#include "WiFiWidget.h"
#include "esp_event_loop.h"
#include "esp_heap_caps_init.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "tcpip_adapter.h"

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include <crypto/CHIPCryptoPAL.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <support/ErrorStr.h>
#include <transport/SecureSessionMgr.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;

extern void startServer();

#if CONFIG_USE_ECHO_CLIENT
extern void startClient(void);
#endif // CONFIG_USE_ECHO_CLIENT

#if CONFIG_DEVICE_TYPE_M5STACK

#define BUTTON_1_GPIO_NUM GPIO_NUM_39               // Left button on M5Stack
#define BUTTON_2_GPIO_NUM GPIO_NUM_38               // Middle button on M5Stack
#define BUTTON_3_GPIO_NUM GPIO_NUM_37               // Right button on M5Stack
#define STATUS_LED_GPIO_NUM GPIO_NUM_MAX            // No status LED on M5Stack
#define LIGHT_CONTROLLER_OUTPUT_GPIO_NUM GPIO_NUM_2 // Use GPIO2 as the light controller output on M5Stack

#elif CONFIG_DEVICE_TYPE_ESP32_DEVKITC

#define BUTTON_1_GPIO_NUM GPIO_NUM_34                // Button 1 on DevKitC
#define BUTTON_2_GPIO_NUM GPIO_NUM_35                // Button 2 on DevKitC
#define BUTTON_3_GPIO_NUM GPIO_NUM_0                 // Button 3 on DevKitC
#define STATUS_LED_GPIO_NUM GPIO_NUM_2               // Use LED1 (blue LED) as status LED on DevKitC
#define LIGHT_CONTROLLER_OUTPUT_GPIO_NUM GPIO_NUM_33 // Use GPIO33 as the light controller output on DevKitC

#else // !CONFIG_DEVICE_TYPE_ESP32_DEVKITC

#error "Unsupported device type selected"

#endif // !CONFIG_DEVICE_TYPE_ESP32_DEVKITC

// A temporary value assigned for this example's QRCode
// Spells CHIP on a dialer
#define EXAMPLE_VENDOR_ID 2447
// Spells ESP32 on a dialer
#define EXAMPLE_PRODUCT_ID 37732
// Used to indicate that an IP address has been added to the QRCode
#define EXAMPLE_VENDOR_TAG_IP 1

#if CONFIG_HAVE_DISPLAY

// Where to draw the connection status message
#define CONNECTION_MESSAGE 75
// Where to draw the IPv6 information
#define IPV6_INFO 85

#endif // CONFIG_HAVE_DISPLAY

LEDWidget statusLED;
BluetoothWidget bluetoothLED;
WiFiWidget wifiLED;

const char * TAG = "wifi-echo-demo";

static EchoDeviceCallbacks EchoCallbacks;
RendezvousSession * rendezvousSession = nullptr;

namespace {

std::vector<Button> buttons          = { Button(), Button(), Button() };
std::vector<gpio_num_t> button_gpios = { BUTTON_1_GPIO_NUM, BUTTON_2_GPIO_NUM, BUTTON_3_GPIO_NUM };

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

#if CONFIG_HAVE_DISPLAY

class EditAttributeListModel : public ListScreen::Model
{
    int d;
    int e;
    int c;
    int a;

public:
    EditAttributeListModel(int d, int e, int c, int a) : d(d), e(e), c(c), a(a) {}
    virtual std::string GetTitle()
    {
        auto & attribute = std::get<1>(std::get<1>(std::get<1>(devices[d])[e])[c])[a];
        auto & name      = std::get<0>(attribute);
        auto & value     = std::get<1>(attribute);
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%s : %s", name.c_str(), value.c_str());
        return buffer;
    }
    virtual int GetItemCount() { return 2; }
    virtual std::string GetItemText(int i) { return i == 0 ? "+" : "-"; }
    virtual void ItemAction(int i)
    {
        auto & attribute = std::get<1>(std::get<1>(std::get<1>(devices[d])[e])[c])[a];
        auto & value     = std::get<1>(attribute);
        int n;
        if (sscanf(value.c_str(), "%d", &n) == 1)
        {
            ESP_LOGI(TAG, "editing attribute as integer: %d (%s)", n, i == 0 ? "+" : "-");
            n += (i == 0) ? 1 : -1;
            char buffer[32];
            sprintf(buffer, "%d", n);
            value = buffer;
        }
        else
        {
            ESP_LOGI(TAG, "editing attribute as string: '%s' (%s)", value.c_str(), i == 0 ? "+" : "-");
            value = (value == "Closed") ? "Open" : "Closed";
        }
    }
};

class AttributeListModel : public ListScreen::Model
{
    int d;
    int e;
    int c;

public:
    AttributeListModel(int d, int e, int c) : d(d), e(e), c(c) {}
    virtual std::string GetTitle() { return "Attributes"; }
    virtual int GetItemCount() { return std::get<1>(std::get<1>(std::get<1>(devices[d])[e])[c]).size(); }
    virtual std::string GetItemText(int i)
    {
        auto & attribute = std::get<1>(std::get<1>(std::get<1>(devices[d])[e])[c])[i];
        auto & name      = std::get<0>(attribute);
        auto & value     = std::get<1>(attribute);
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%s : %s", name.c_str(), value.c_str());
        return buffer;
    }
    virtual void ItemAction(int i)
    {
        ESP_LOGI(TAG, "Opening attribute %d", i);
        ScreenManager::PushScreen(new ListScreen(new EditAttributeListModel(d, e, c, i)));
    }
};

class ClusterListModel : public ListScreen::Model
{
    int d;
    int e;

public:
    ClusterListModel(int d, int e) : d(d), e(e) {}
    virtual std::string GetTitle() { return "Clusters"; }
    virtual int GetItemCount() { return std::get<1>(std::get<1>(devices[d])[e]).size(); }
    virtual std::string GetItemText(int i) { return std::get<0>(std::get<1>(std::get<1>(devices[d])[e])[i]); }
    virtual void ItemAction(int i)
    {
        ESP_LOGI(TAG, "Opening cluster %d", i);
        ScreenManager::PushScreen(new ListScreen(new AttributeListModel(d, e, i)));
    }
};

class EndpointListModel : public ListScreen::Model
{
    int d;

public:
    EndpointListModel(int d) : d(d) {}
    virtual std::string GetTitle() { return "Endpoints"; }
    virtual int GetItemCount() { return std::get<1>(devices[d]).size(); }
    virtual std::string GetItemText(int i) { return std::get<0>(std::get<1>(devices[d])[i]); }
    virtual void ItemAction(int i)
    {
        ESP_LOGI(TAG, "Opening endpoint %d", i);
        ScreenManager::PushScreen(new ListScreen(new ClusterListModel(d, i)));
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
        ScreenManager::PushScreen(new ListScreen(new EndpointListModel(i)));
    }
};

class SetupListModel : public ListScreen::Model
{
public:
    SetupListModel()
    {
        std::string resetWiFi = "Reset WiFi";
        options.emplace_back(resetWiFi);
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

#endif // CONFIG_HAVE_DISPLAY

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

    AddDevice("Thermometer");
    AddEndpoint("External");
    AddCluster("Thermometer");
    AddAttribute("Temperature", "21");
    AddEndpoint("Internal");
    AddCluster("Thermometer");
    AddAttribute("Temperature", "42");

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
    tcpip_adapter_ip_info_t ip;
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ip);
    IPAddress::FromIPv4(ip.ip).ToString(ip_buf, ip_len);
    ESP_LOGE(TAG, "Got gateway ip %s", ip_buf);
}

bool isRendezvousBLE()
{
    return static_cast<RendezvousInformationFlags>(CONFIG_RENDEZVOUS_MODE) == RendezvousInformationFlags::kBLE;
}

std::string createSetupPayload()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    string result;

    uint32_t discriminator;
    err = ConfigurationMgr().GetSetupDiscriminator(discriminator);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Couldn't get discriminator: %d", err);
        return result;
    }

    uint32_t setupPINCode;
    err = ConfigurationMgr().GetSetupPinCode(setupPINCode);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Couldn't get setupPINCode: %d", err);
        return result;
    }

    SetupPayload payload;
    payload.version               = 1;
    payload.discriminator         = discriminator;
    payload.setUpPINCode          = setupPINCode;
    payload.rendezvousInformation = static_cast<RendezvousInformationFlags>(CONFIG_RENDEZVOUS_MODE);
    payload.vendorID              = EXAMPLE_VENDOR_ID;
    payload.productID             = EXAMPLE_PRODUCT_ID;

    if (!isRendezvousBLE())
    {
        char gw_ip[INET6_ADDRSTRLEN];
        GetGatewayIP(gw_ip, sizeof(gw_ip));
        payload.addOptionalVendorData(EXAMPLE_VENDOR_TAG_IP, gw_ip);

        QRCodeSetupPayloadGenerator generator(payload);

        size_t tlvDataLen = sizeof(gw_ip);
        uint8_t tlvDataStart[tlvDataLen];
        err = generator.payloadBase41Representation(result, tlvDataStart, tlvDataLen);
    }
    else
    {
        QRCodeSetupPayloadGenerator generator(payload);
        err = generator.payloadBase41Representation(result);
    }

    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Couldn't get payload string %d", err);
    }
    return result;
};

} // namespace

extern "C" void app_main()
{
    ESP_LOGI(TAG, "WiFi Echo Demo!");

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

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    err = deviceMgr.Init(&EchoCallbacks);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "device.Init() failed: %s", ErrorStr(err));
        return;
    }

    SetupPretendDevices();

    statusLED.Init(STATUS_LED_GPIO_NUM);
    bluetoothLED.Init();
    wifiLED.Init();

    // Start the Echo Server
    InitDataModelHandler();
    startServer();

    if (isRendezvousBLE())
    {
        rendezvousSession = new RendezvousSession(&bluetoothLED);
    }

#if CONFIG_USE_ECHO_CLIENT
    startClient();
#endif

    std::string qrCodeText = createSetupPayload();
    ESP_LOGI(TAG, "QR CODE: '%s'", qrCodeText.c_str());

#if CONFIG_HAVE_DISPLAY
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

    // Initialize the display device.
    err = InitDisplay();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "InitDisplay() failed: %s", ErrorStr(err));
        return;
    }

    // Initialize the screen manager and push a rudimentary user interface.
    ScreenManager::Init();
    ScreenManager::PushScreen(new ListScreen((new SimpleListModel())
                                                 ->Title("CHIP")
                                                 ->Action([](int i) { ESP_LOGI(TAG, "action on item %d", i); })
                                                 ->Item("Devices",
                                                        []() {
                                                            ESP_LOGI(TAG, "Opening device list");
                                                            ScreenManager::PushScreen(new ListScreen(new DeviceListModel()));
                                                        })
                                                 ->Item("Custom",
                                                        []() {
                                                            ESP_LOGI(TAG, "Opening custom screen");
                                                            ScreenManager::PushScreen(new CustomScreen());
                                                        })
                                                 ->Item("QR Code",
                                                        [=]() {
                                                            ESP_LOGI(TAG, "Opening QR code screen");
                                                            ScreenManager::PushScreen(new QRCodeScreen(qrCodeText));
                                                        })
                                                 ->Item("Setup",
                                                        [=]() {
                                                            ESP_LOGI(TAG, "Opening Setup list");
                                                            ScreenManager::PushScreen(new ListScreen(new SetupListModel()));
                                                        })
                                                 ->Item("More")
                                                 ->Item("Items")
                                                 ->Item("For")
                                                 ->Item("Demo")));

    // Connect the status LED to VLEDs.
    {
        int vled1 = ScreenManager::AddVLED(TFT_GREEN);
        int vled2 = ScreenManager::AddVLED(TFT_RED);
        statusLED.SetVLED(vled1, vled2);

        bluetoothLED.SetVLED(ScreenManager::AddVLED(TFT_BLUE));
        wifiLED.SetVLED(ScreenManager::AddVLED(TFT_YELLOW));
    }

#endif // CONFIG_HAVE_DISPLAY

    // Run the UI Loop
    while (true)
    {
#if CONFIG_HAVE_DISPLAY
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

#endif // CONFIG_HAVE_DISPLAY

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

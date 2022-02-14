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
#include "OpenThreadLaunch.h"
#include "QRCodeScreen.h"
#include "ScreenManager.h"
#include "ShellCommands.h"
#include "StatusScreen.h"
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
#include "platform/PlatformManager.h"
#include "shell_extension/launch.h"

#include <cmath>
#include <cstdio>
#include <ctype.h>
#include <string>
#include <vector>

#include <app-common/zap-generated/att-storage.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/OTARequestor.h>
#include <app/server/AppDelegate.h>
#include <app/server/Dnssd.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/af-types.h>
#include <app/util/af.h>
#include <binding-handler.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/shell/Engine.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/ErrorStr.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ESP32/NetworkCommissioningDriver.h>
#include <platform/ESP32/OTAImageProcessorImpl.h>
#include <platform/GenericOTARequestorDriver.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

#if CONFIG_ENABLE_PW_RPC
#include "Rpc.h"
#endif

#if CONFIG_OPENTHREAD_ENABLED
#include <platform/ThreadStackManager.h>
#endif

using namespace ::chip;
using namespace ::chip::Shell;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;

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

#if CONFIG_ENABLE_OTA_REQUESTOR
OTARequestor gRequestorCore;
GenericOTARequestorDriver gRequestorUser;
BDXDownloader gDownloader;
OTAImageProcessorImpl gImageProcessor;
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

namespace {
app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::ESPWiFiDriver::GetInstance()));
} // namespace

void NetWorkCommissioningInstInit()
{
    sWiFiNetworkCommissioningInstance.Init();
}

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

class TouchesMatterStackModel : public ListScreen::Model
{
    // We could override Action() and then hope focusIndex has not changed by
    // the time our queued task runs, but it's cleaner to just capture its value
    // now.
    struct QueuedAction
    {
        QueuedAction(TouchesMatterStackModel * selfArg, int iArg) : self(selfArg), i(iArg) {}
        TouchesMatterStackModel * self;
        int i;
    };

    void ItemAction(int i) final
    {
        auto * action = chip::Platform::New<QueuedAction>(this, i);
        chip::DeviceLayer::PlatformMgr().ScheduleWork(QueuedActionHandler, reinterpret_cast<intptr_t>(action));
    }

    static void QueuedActionHandler(intptr_t closure)
    {
        auto * queuedAction = reinterpret_cast<QueuedAction *>(closure);
        queuedAction->self->DoAction(queuedAction->i);
        chip::Platform::Delete(queuedAction);
    }

    virtual void DoAction(int i) = 0;
};

class EditAttributeListModel : public TouchesMatterStackModel
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

    void DoAction(int i) override
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
                chip::app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(1, static_cast<int16_t>(n * 100));
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
                using namespace chip::app::Clusters;
                // update the doorlock attribute here
                auto attributeValue = value == "Closed" ? DoorLock::DlLockState::kLocked : DoorLock::DlLockState::kUnlocked;
                DoorLock::Attributes::LockState::Set(DOOR_LOCK_SERVER_ENDPOINT, attributeValue);
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

class ActionListModel : public ListScreen::Model
{
    int GetItemCount() override { return static_cast<int>(mActions.size()); }
    std::string GetItemText(int i) override { return mActions[i].title.c_str(); }
    void ItemAction(int i) override
    {
        ESP_LOGI(TAG, "generic action %d", i);
        mActions[i].action();
    }

protected:
    void AddAction(const char * name, std::function<void(void)> action) { mActions.push_back(Action(name, action)); }

private:
    struct Action
    {
        std::string title;
        std::function<void(void)> action;

        Action(const char * t, std::function<void(void)> a) : title(t), action(a) {}
    };

    std::vector<Action> mActions;
};

class MdnsDebugListModel : public ActionListModel
{
public:
    std::string GetTitle() override { return "mDNS Debug"; }

    MdnsDebugListModel() { AddAction("(Re-)Init", std::bind(&MdnsDebugListModel::DoReinit, this)); }

private:
    void DoReinit()
    {
        CHIP_ERROR err = Dnssd::ServiceAdvertiser::Instance().Init(DeviceLayer::UDPEndPointManager());
        if (err != CHIP_NO_ERROR)
        {
            ESP_LOGE(TAG, "Error initializing: %s", err.AsString());
        }
    }
};

class SetupListModel : public TouchesMatterStackModel
{
public:
    SetupListModel()
    {
        std::string resetWiFi                   = "Reset WiFi";
        std::string resetToFactory              = "Reset to factory";
        std::string forceWiFiCommissioningBasic = "Force WiFi commissioning (basic)";
        options.emplace_back(resetWiFi);
        options.emplace_back(resetToFactory);
        options.emplace_back(forceWiFiCommissioningBasic);
    }
    virtual std::string GetTitle() { return "Setup"; }
    virtual int GetItemCount() { return options.size(); }
    virtual std::string GetItemText(int i) { return options.at(i); }
    void DoAction(int i) override
    {
        ESP_LOGI(TAG, "Opening options %d: %s", i, GetItemText(i).c_str());
        if (i == 0)
        {
            ConnectivityMgr().ClearWiFiStationProvision();
            chip::Server::GetInstance().GetFabricTable().DeleteAllFabrics();
            chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
        }
        else if (i == 1)
        {
            ConfigurationMgr().InitiateFactoryReset();
        }
        else if (i == 2)
        {
            app::DnssdServer::Instance().StartServer(Dnssd::CommissioningMode::kEnabledBasic);
            chip::Server::GetInstance().GetFabricTable().DeleteAllFabrics();
            chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow(
                kNoCommissioningTimeout, CommissioningWindowAdvertisement::kDnssdOnly);
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
    chip::app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(1, static_cast<int16_t>(21 * 100));

    AddDevice("Door Lock");
    AddEndpoint("Default");
    AddCluster("Lock");
    AddAttribute("State", "Open");
    // write the door lock state
    chip::app::Clusters::DoorLock::Attributes::LockState::Set(DOOR_LOCK_SERVER_ENDPOINT,
                                                              chip::app::Clusters::DoorLock::DlLockState::kUnlocked);
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

    AddDevice("Occupancy Sensor");
    AddEndpoint("External");
    AddCluster("Occupancy Sensor");
    AddAttribute("Occupancy", "1");
    app::Clusters::OccupancySensing::Attributes::Occupancy::Set(1, 1);

    AddDevice("Contact Sensor");
    AddEndpoint("External");
    AddCluster("Contact Sensor");
    AddAttribute("BooleanState", "true");
    app::Clusters::BooleanState::Attributes::StateValue::Set(1, true);

    AddDevice("Thermostat");
    AddEndpoint("1");
    AddCluster("Thermostat");
    app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(1, static_cast<int16_t>(21 * 100));
    app::Clusters::Thermostat::Attributes::LocalTemperature::Set(1, static_cast<int16_t>(21 * 100));
    AddAttribute("SystemMode", "4");
    app::Clusters::Thermostat::Attributes::SystemMode::Set(1, 4);
    AddAttribute("OccupiedCoolingSetpoint", "19");
    app::Clusters::Thermostat::Attributes::OccupiedCoolingSetpoint::Set(1, static_cast<int16_t>(19 * 100));
    AddAttribute("OccupiedHeatingSetpoint", "25");
    app::Clusters::Thermostat::Attributes::OccupiedHeatingSetpoint::Set(1, static_cast<int16_t>(25 * 100));

    AddDevice("Humidity Sensor");
    AddEndpoint("External");
    AddCluster("Humidity Sensor");
    AddAttribute("MeasuredValue", "30");
    app::Clusters::RelativeHumidityMeasurement::Attributes::MeasuredValue::Set(1, static_cast<int16_t>(30 * 100));

    AddDevice("Light Sensor");
    AddEndpoint("External");
    AddCluster("Illuminance Measurement");
    AddAttribute("MeasuredValue", "1000");
    app::Clusters::IlluminanceMeasurement::Attributes::MeasuredValue::Set(1, static_cast<int16_t>(1000));
}

WiFiWidget pairingWindowLED;

class AppCallbacks : public AppDelegate
{
public:
    void OnRendezvousStarted() override { bluetoothLED.Set(true); }
    void OnRendezvousStopped() override
    {
        bluetoothLED.Set(false);
        pairingWindowLED.Set(false);
    }
    void OnPairingWindowOpened() override { pairingWindowLED.Set(true); }
    void OnPairingWindowClosed() override { pairingWindowLED.Set(false); }
};

AppCallbacks sCallbacks;

} // namespace

static void InitServer(intptr_t context)
{
    // Init ZCL Data Model and CHIP App Server
    chip::Server::GetInstance().Init(&sCallbacks);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
    NetWorkCommissioningInstInit();
    SetupPretendDevices();
    InitBindingHandlers();
}

static void InitOTARequestor(void)
{
#if CONFIG_ENABLE_OTA_REQUESTOR
    SetRequestorInstance(&gRequestorCore);
    gRequestorCore.Init(&Server::GetInstance(), &gRequestorUser, &gDownloader);
    gImageProcessor.SetOTADownloader(&gDownloader);
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);
    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);
#endif
}

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

    // Initialize the ESP NVS layer.
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_flash_init() failed: %s", esp_err_to_name(err));
        return;
    }
#if CONFIG_ENABLE_PW_RPC
    chip::rpc::Init();
#endif

#if CONFIG_ENABLE_CHIP_SHELL
    chip::LaunchShell();
    OnOffCommands::GetInstance().Register();
    CASECommands::GetInstance().Register();
#endif // CONFIG_ENABLE_CHIP_SHELL

#if CONFIG_OPENTHREAD_ENABLED
    LaunchOpenThread();
    ThreadStackMgr().InitThreadStack();
#endif

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    CHIP_ERROR error = deviceMgr.Init(&EchoCallbacks);
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "device.Init() failed: %s", ErrorStr(error));
        return;
    }

    statusLED1.Init(STATUS_LED_GPIO_NUM);
    // Our second LED doesn't map to any physical LEDs so far, just to virtual
    // "LED"s on devices with screens.
    statusLED2.Init(GPIO_NUM_MAX);
    bluetoothLED.Init();
    wifiLED.Init();
    pairingWindowLED.Init();

    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitServer, reinterpret_cast<intptr_t>(nullptr));

    // Print QR Code URL
    PrintOnboardingCodes(chip::RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE));

    InitOTARequestor();

#if CONFIG_HAVE_DISPLAY
    std::string qrCodeText;

    GetQRCode(qrCodeText, chip::RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE));

    // Initialize the display device.
    err = InitDisplay();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "InitDisplay() failed: %s", esp_err_to_name(err));
        return;
    }

    // Initialize the screen manager
    ScreenManager::Init();

#if CONFIG_DEVICE_TYPE_M5STACK
    // Initialize the buttons.
    for (int i = 0; i < buttons.size(); ++i)
    {
        err = buttons[i].Init(button_gpios[i], 50);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Button.Init() failed: %s", esp_err_to_name(err));
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
            ->Item("mDNS Debug",
                   []() {
                       ESP_LOGI(TAG, "Opening MDNS debug");
                       ScreenManager::PushScreen(chip::Platform::New<ListScreen>(chip::Platform::New<MdnsDebugListModel>()));
                   })
            ->Item("QR Code",
                   [=]() {
                       ESP_LOGI(TAG, "Opening QR code screen");
                       PrintOnboardingCodes(chip::RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE));
                       ScreenManager::PushScreen(chip::Platform::New<QRCodeScreen>(qrCodeText));
                   })
            ->Item("Setup",
                   [=]() {
                       ESP_LOGI(TAG, "Opening Setup list");
                       ScreenManager::PushScreen(chip::Platform::New<ListScreen>(chip::Platform::New<SetupListModel>()));
                   })
            ->Item("Status",
                   [=]() {
                       ESP_LOGI(TAG, "Opening Status screen");
                       ScreenManager::PushScreen(chip::Platform::New<StatusScreen>());
                   })
            ->Item("Custom",
                   []() {
                       ESP_LOGI(TAG, "Opening custom screen");
                       ScreenManager::PushScreen(chip::Platform::New<CustomScreen>());
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

#if CONFIG_DEVICE_TYPE_M5STACK
    // Run the UI Loop
    while (true)
    {
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

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
#endif // CONFIG_DEVICE_TYPE_M5STACK
}

bool lowPowerClusterSleep()
{
    return true;
}

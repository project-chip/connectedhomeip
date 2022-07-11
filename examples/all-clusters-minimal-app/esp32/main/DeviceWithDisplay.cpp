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

#include "DeviceWithDisplay.h"
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

#if CONFIG_HAVE_DISPLAY
using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;

static const char * TAG = "DeviceWithDisplay";

#if CONFIG_DEVICE_TYPE_M5STACK

Button gButtons[BUTTON_NUMBER] = { Button(BUTTON_1_GPIO_NUM), Button(BUTTON_2_GPIO_NUM), Button(BUTTON_3_GPIO_NUM) };

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
        return value == "On" || value == "Off" || value == "Yes" || value == "No";
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
            else if (name == "Color Current Level")
            {
                // update the current level here for hardcoded endpoint 1
                ESP_LOGI(TAG, "Brightness changed to : %d", (n * 100 / 255));
                app::Clusters::LevelControl::Attributes::CurrentLevel::Set(1, n);
            }
            else if (name == "Current Hue")
            {
                // update the current hue here for hardcoded endpoint 1
                ESP_LOGI(TAG, "Hue changed to : %d", n * 360 / 254);
                app::Clusters::ColorControl::Attributes::CurrentHue::Set(1, n);
            }
            else if (name == "Current Saturation")
            {
                // update the current saturation here for hardcoded endpoint 1
                ESP_LOGI(TAG, "Saturation changed to : %d", n * 100 / 254);
                app::Clusters::ColorControl::Attributes::CurrentSaturation::Set(1, n);
            }
            else if (name == "Illuminance")
            {
                // update the current illuminance here for hardcoded endpoint 1
                ESP_LOGI(TAG, "Illuminance changed to : %d", n);
                app::Clusters::IlluminanceMeasurement::Attributes::MeasuredValue::Set(1, static_cast<int16_t>(n));
            }
            else if (name == "Humidity")
            {
                // update the current humidity here for hardcoded endpoint 1
                ESP_LOGI(TAG, "Humidity changed to : %d", n);
                app::Clusters::RelativeHumidityMeasurement::Attributes::MeasuredValue::Set(1, static_cast<int16_t>(n * 100));
            }
            else if (name == "OccupiedCoolingSetpoint")
            {
                ESP_LOGI(TAG, "OccupiedCoolingSetpoint changed to : %d", n);
                app::Clusters::Thermostat::Attributes::OccupiedCoolingSetpoint::Set(1, static_cast<int16_t>(n * 100));
            }
            else if (name == "OccupiedHeatingSetpoint")
            {
                ESP_LOGI(TAG, "OccupiedHeatingSetpoint changed to : %d", n);
                app::Clusters::Thermostat::Attributes::OccupiedHeatingSetpoint::Set(1, static_cast<int16_t>(n * 100));
            }
            else if (name == "SystemMode")
            {
                ESP_LOGI(TAG, "SystemMode changed to : %d", n);
                app::Clusters::Thermostat::Attributes::OccupiedHeatingSetpoint::Set(1, n);
            }
            else if (name == "Current Lift")
            {
                // update the current lift here for hardcoded endpoint 1
                ESP_LOGI(TAG, "Current position lift percent 100ths changed to : %d", n * 100);
                app::Clusters::WindowCovering::Attributes::CurrentPositionLiftPercent100ths::Set(1, static_cast<uint16_t>(n * 100));
            }
            else if (name == "Current Tilt")
            {
                // update the current tilt here for hardcoded endpoint 1
                ESP_LOGI(TAG, "Current position tilt percent 100ths changed to : %d", n * 100);
                app::Clusters::WindowCovering::Attributes::CurrentPositionTiltPercent100ths::Set(1, static_cast<uint16_t>(n * 100));
            }
            else if (name == "Opr Status")
            {
                // update the operational status here for hardcoded endpoint 1
                ESP_LOGI(TAG, "Operational status changed to : %d", n);
                app::Clusters::WindowCovering::Attributes::OperationalStatus::Set(1, static_cast<uint8_t>(n));
            }
            else if (name == "Bat remaining")
            {
                // update the battery percent remaining here for hardcoded endpoint 1
                ESP_LOGI(TAG, "Battery percent remaining changed to : %d", n);
                app::Clusters::PowerSource::Attributes::BatPercentRemaining::Set(1, static_cast<uint8_t>(n * 2));
            }
            value = buffer;
        }
        else if (IsBooleanAttribute())
        {
            auto & name    = std::get<0>(attribute);
            auto & cluster = std::get<0>(std::get<1>(std::get<1>(devices[deviceIndex])[endpointIndex])[i]);

            if (name == "OnOff" && cluster == "OnOff")
            {
                value                  = (value == "On") ? "Off" : "On";
                uint8_t attributeValue = (value == "On") ? 1 : 0;
                emberAfWriteServerAttribute(endpointIndex + 1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID,
                                            (uint8_t *) &attributeValue, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
            }

            if (name == "Occupancy" && cluster == "Occupancy Sensor")
            {
                value                  = (value == "Yes") ? "No" : "Yes";
                uint8_t attributeValue = (value == "Yes") ? 1 : 0;
                ESP_LOGI(TAG, "Occupancy changed to : %s", value.c_str());
                // update the current occupancy here for hardcoded endpoint 1
                app::Clusters::OccupancySensing::Attributes::Occupancy::Set(1, attributeValue);
            }
        }
        else
        {
            auto & name    = std::get<0>(attribute);
            auto & cluster = std::get<0>(std::get<1>(std::get<1>(devices[deviceIndex])[endpointIndex])[0]);

            ESP_LOGI(TAG, "editing attribute as string: '%s' (%s)", value.c_str(), i == 0 ? "+" : "-");
            ESP_LOGI(TAG, "name and cluster: '%s' (%s)", name.c_str(), cluster.c_str());
            if (name == "State" && cluster == "Lock")
            {
                value = (value == "Closed") ? "Open" : "Closed";
                using namespace chip::app::Clusters;
                // update the doorlock attribute here
                auto attributeValue = value == "Closed" ? DoorLock::DlLockState::kLocked : DoorLock::DlLockState::kUnlocked;
                DoorLock::Attributes::LockState::Set(DOOR_LOCK_SERVER_ENDPOINT, attributeValue);
            }
            else if (name == "Charge level" && cluster == "Power Source")
            {
                using namespace chip::app::Clusters::PowerSource;
                auto attributeValue = BatChargeLevel::kOk;

                if (value == "OK")
                {
                    value          = "Warning";
                    attributeValue = BatChargeLevel::kWarning;
                }
                else if (value == "Warning")
                {
                    value          = "Critical";
                    attributeValue = BatChargeLevel::kCritical;
                }
                else
                {
                    value          = "OK";
                    attributeValue = BatChargeLevel::kOk;
                }

                // update the battery charge level here for hardcoded endpoint 1
                ESP_LOGI(TAG, "Battery charge level changed to : %u", static_cast<uint8_t>(attributeValue));
                app::Clusters::PowerSource::Attributes::BatChargeLevel::Set(1, attributeValue);
            }
            else
            {
                value = (value == "Closed") ? "Open" : "Closed";
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
            chip::Server::GetInstance().ScheduleFactoryReset();
        }
        else if (i == 2)
        {
            chip::Server::GetInstance().GetFabricTable().DeleteAllFabrics();
            auto & commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
            commissionMgr.OpenBasicCommissioningWindow(commissionMgr.MaxCommissioningTimeout(),
                                                       CommissioningWindowAdvertisement::kDnssdOnly);
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
    AddAttribute("Occupancy", "Yes");
    app::Clusters::OccupancySensing::Attributes::Occupancy::Set(1, 1);

    AddDevice("Contact Sensor");
    AddEndpoint("External");
    AddCluster("Contact Sensor");
    AddAttribute("Contact", "true");
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
    AddAttribute("Humidity", "30");
    app::Clusters::RelativeHumidityMeasurement::Attributes::MeasuredValue::Set(1, static_cast<int16_t>(30 * 100));

    AddDevice("Light Sensor");
    AddEndpoint("External");
    AddCluster("Illuminance Measurement");
    AddAttribute("Illuminance", "1000");
    app::Clusters::IlluminanceMeasurement::Attributes::MeasuredValue::Set(1, static_cast<int16_t>(1000));

    AddDevice("Color Light");
    AddEndpoint("1");
    AddCluster("OnOff");
    AddAttribute("OnOff", "Off");
    app::Clusters::OnOff::Attributes::OnOff::Set(1, false);
    AddCluster("Level Control");
    AddAttribute("Color Current Level", "255");
    app::Clusters::LevelControl::Attributes::CurrentLevel::Set(1, 255);
    AddEndpoint("2");
    AddCluster("Color Control");
    AddAttribute("Current Hue", "200");
    app::Clusters::ColorControl::Attributes::CurrentHue::Set(1, 200);
    AddAttribute("Current Saturation\n", "150");
    app::Clusters::ColorControl::Attributes::CurrentSaturation::Set(1, 150);

    AddDevice("Window Covering");
    AddEndpoint("1");
    AddCluster("Window Covering");
    AddAttribute("Current Lift", "5");
    app::Clusters::WindowCovering::Attributes::CurrentPositionLiftPercent100ths::Set(1, static_cast<uint16_t>(5 * 100));
    AddAttribute("Current Tilt", "5");
    app::Clusters::WindowCovering::Attributes::CurrentPositionTiltPercent100ths::Set(1, static_cast<uint16_t>(5 * 100));
    AddAttribute("Opr Status", "0");
    app::Clusters::WindowCovering::Attributes::OperationalStatus::Set(1, static_cast<uint8_t>(0));

    AddDevice("Battery");
    AddEndpoint("1");
    AddCluster("Power Source");
    AddAttribute("Bat remaining", "70");
    app::Clusters::PowerSource::Attributes::BatPercentRemaining::Set(1, static_cast<uint8_t>(70 * 2));
    AddAttribute("Charge level", "0");
    app::Clusters::PowerSource::Attributes::BatChargeLevel::Set(1, app::Clusters::PowerSource::BatChargeLevel::kOk);
}

esp_err_t InitM5Stack(std::string qrCodeText)
{
    esp_err_t err;
    // Initialize the buttons.
    err = gpio_install_isr_service(0);
    ESP_RETURN_ON_ERROR(err, TAG, "Button preInit failed: %s", esp_err_to_name(err));
    for (int i = 0; i < BUTTON_NUMBER; ++i)
    {
        err = gButtons[i].Init();
        ESP_RETURN_ON_ERROR(err, TAG, "Button.Init() failed: %s", esp_err_to_name(err));
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
    return ESP_OK;
}
#endif

void InitDeviceDisplay()
{
    // Create buffer for QR code that can fit max size and null terminator.
    char qrCodeBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
    chip::MutableCharSpan qrCodeText(qrCodeBuffer);

    // Get QR Code and emulate its content using NFC tag
    GetQRCode(qrCodeText, chip::RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE));

    // Initialize the display device.
    esp_err_t err = InitDisplay();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "InitDisplay() failed: %s", esp_err_to_name(err));
        return;
    }

    // Initialize the screen manager
    ScreenManager::Init();

    // Connect the status LED to VLEDs.
    int vled1 = ScreenManager::AddVLED(TFT_GREEN);
    int vled2 = ScreenManager::AddVLED(TFT_RED);
    statusLED1.SetVLED(vled1, vled2);

    int vled3 = ScreenManager::AddVLED(TFT_CYAN);
    int vled4 = ScreenManager::AddVLED(TFT_ORANGE);
    statusLED2.SetVLED(vled3, vled4);

    bluetoothLED.SetVLED(ScreenManager::AddVLED(TFT_BLUE));
    wifiLED.SetVLED(ScreenManager::AddVLED(TFT_YELLOW));
    pairingWindowLED.SetVLED(ScreenManager::AddVLED(TFT_ORANGE));

#if CONFIG_DEVICE_TYPE_M5STACK

    InitM5Stack(qrCodeText.data());

#elif CONFIG_DEVICE_TYPE_ESP32_WROVER_KIT

    // Display the QR Code
    QRCodeScreen qrCodeScreen(qrCodeText.data());
    qrCodeScreen.Display();

#endif
}
#endif

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

#include "DeviceWithDisplay.h"

#if CONFIG_HAVE_DISPLAY

#include "Globals.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;

static const char TAG[] = "DeviceWithDisplay";

#if CONFIG_DEVICE_TYPE_M5STACK

Button gButtons[BUTTON_NUMBER] = { Button(BUTTON_1_GPIO_NUM), Button(BUTTON_2_GPIO_NUM), Button(BUTTON_3_GPIO_NUM) };

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
            ->Item("Status", [=]() {
                ESP_LOGI(TAG, "Opening Status screen");
                ScreenManager::PushScreen(chip::Platform::New<StatusScreen>());
            })));
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
    wifiLED.SetVLED(ScreenManager::AddVLED(TFT_GREEN));

#if CONFIG_DEVICE_TYPE_M5STACK

    InitM5Stack(qrCodeText.data());

#elif CONFIG_DEVICE_TYPE_ESP32_WROVER_KIT

    // Display the QR Code
    QRCodeScreen qrCodeScreen(qrCodeText.data());
    qrCodeScreen.Display();

#endif
}
#endif

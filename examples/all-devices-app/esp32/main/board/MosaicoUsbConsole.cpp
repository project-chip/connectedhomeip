/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "MosaicoUsbConsole.h"

#include <atomic>
#include <cstdio>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <soc/lp_system_reg.h>
#include <soc/soc.h>
#include <tinyusb.h>
#include <tinyusb_cdc_acm.h>
#include <tinyusb_console.h>
#include <tinyusb_default_config.h>

namespace {

const char TAG[] = "UsbConsole";

// esptool and idf_monitor pick the USB Serial/JTAG reset strategy from this
// product ID; the CDC callback below implements the matching protocol.
constexpr uint16_t kUsbSerialJtagPid = 0x1001;
constexpr uint64_t kRestartDelayUs   = 50 * 1000;

const tusb_desc_device_t kDeviceDescriptor = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = TINYUSB_ESPRESSIF_VID,
    .idProduct          = kUsbSerialJtagPid,
    .bcdDevice          = CONFIG_TINYUSB_DESC_BCD_DEVICE,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01,
};

enum class Reboot
{
    kNone,
    kNormal,
    kBootloader,
};

std::atomic<Reboot> sRebootMode{ Reboot::kNone };
std::atomic<bool> sDownloadLatched{ false };
esp_timer_handle_t sRestartTimer = nullptr;
bool sInitialized                = false;

void BeforeRestart()
{
    switch (sRebootMode.load(std::memory_order_acquire))
    {
    case Reboot::kBootloader:
        REG_SET_BIT(LP_SYSTEM_REG_SYS_CTRL_REG, LP_SYSTEM_REG_FORCE_DOWNLOAD_BOOT);
        break;
    case Reboot::kNormal:
        REG_CLR_BIT(LP_SYSTEM_REG_SYS_CTRL_REG, LP_SYSTEM_REG_FORCE_DOWNLOAD_BOOT);
        break;
    case Reboot::kNone:
        break;
    }
}

void OnRestartTimer(void *)
{
    esp_restart();
}

// USB Serial/JTAG state table: RTS low latches DTR as the download-mode request,
// the first RTS high with DTR low resets using the latched mode. esptool repeats
// states while toggling, so only the first reset request is honoured.
void OnLineStateChanged(int, cdcacm_event_t * event)
{
    const bool dtr = event->line_state_changed_data.dtr;
    const bool rts = event->line_state_changed_data.rts;

    if (!rts)
    {
        sDownloadLatched.store(dtr, std::memory_order_release);
        return;
    }
    if (dtr)
    {
        return;
    }

    const Reboot requested = sDownloadLatched.load(std::memory_order_acquire) ? Reboot::kBootloader : Reboot::kNormal;
    Reboot expected        = Reboot::kNone;
    if (!sRebootMode.compare_exchange_strong(expected, requested, std::memory_order_acq_rel))
    {
        return;
    }

    // Let SET_CONTROL_LINE_STATE complete before the restart.
    if (esp_timer_start_once(sRestartTimer, kRestartDelayUs) != ESP_OK)
    {
        sRebootMode.store(Reboot::kNone, std::memory_order_release);
    }
}

esp_err_t InitResetEmulation()
{
    const esp_timer_create_args_t timerArgs = {
        .callback = OnRestartTimer,
        .name     = "usb_reboot",
    };
    esp_err_t err = esp_timer_create(&timerArgs, &sRestartTimer);
    if (err != ESP_OK)
    {
        return err;
    }
    err = esp_register_shutdown_handler(BeforeRestart);
    if (err != ESP_OK)
    {
        esp_timer_delete(sRestartTimer);
        sRestartTimer = nullptr;
    }
    return err;
}

void DeinitResetEmulation()
{
    esp_unregister_shutdown_handler(BeforeRestart);
    esp_timer_stop(sRestartTimer);
    esp_timer_delete(sRestartTimer);
    sRestartTimer = nullptr;
}

} // namespace

esp_err_t InitMosaicoUsbConsole()
{
    if (sInitialized)
    {
        return ESP_OK;
    }

    esp_err_t err = InitResetEmulation();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Reset emulation setup failed: %s", esp_err_to_name(err));
        return err;
    }

    tinyusb_config_t usbConfig  = TINYUSB_DEFAULT_CONFIG();
    usbConfig.descriptor.device = &kDeviceDescriptor;
    err                         = tinyusb_driver_install(&usbConfig);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "tinyusb_driver_install failed: %s", esp_err_to_name(err));
        DeinitResetEmulation();
        return err;
    }

    const tinyusb_config_cdcacm_t cdcConfig = {
        .cdc_port                    = TINYUSB_CDC_ACM_0,
        .callback_line_state_changed = OnLineStateChanged,
    };
    err = tinyusb_cdcacm_init(&cdcConfig);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "tinyusb_cdcacm_init failed: %s", esp_err_to_name(err));
        tinyusb_driver_uninstall();
        DeinitResetEmulation();
        return err;
    }

    err = tinyusb_console_init(TINYUSB_CDC_ACM_0);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "tinyusb_console_init failed: %s", esp_err_to_name(err));
        tinyusb_cdcacm_deinit(TINYUSB_CDC_ACM_0);
        tinyusb_driver_uninstall();
        DeinitResetEmulation();
        return err;
    }

    // The host may open the port after boot; unbuffered streams keep nothing back.
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);

    sInitialized = true;
    ESP_LOGI(TAG, "USB CDC console ready");
    return ESP_OK;
}

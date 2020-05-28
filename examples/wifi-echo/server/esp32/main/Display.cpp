/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#include <string.h>

#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "Display.h"

#if CONFIG_HAVE_DISPLAY

#define DEF_BRIGHTNESS_PERCENT 80

// with a duty resolution of LEDC_TIMER_8_BIT
// the highest possible brightness value is 255
#define BRIGHTNESS_MAX 255
// The M5Stack's backlight is on Channel 7
#define BACKLIGHT_CHANNEL LEDC_CHANNEL_7

extern const char * TAG;

uint16_t DisplayHeight = 0;
uint16_t DisplayWidth  = 0;

// FreeRTOS timer used to turn the display off after a short while
TimerHandle_t displayTimer = NULL;

static void TimerCallback(TimerHandle_t xTimer);
static void SetupBrightnessControl();
static void SetBrightness(uint16_t brightness);

esp_err_t InitDisplay()
{
    esp_err_t err;
    spi_lobo_device_handle_t spi;

    spi_lobo_bus_config_t buscfg;
    memset((void *) &buscfg, 0, sizeof(buscfg));
    buscfg.miso_io_num     = PIN_NUM_MISO; // set SPI MISO pin
    buscfg.mosi_io_num     = PIN_NUM_MOSI; // set SPI MOSI pin
    buscfg.sclk_io_num     = PIN_NUM_CLK;  // set SPI CLK pin
    buscfg.quadwp_io_num   = -1;
    buscfg.quadhd_io_num   = -1;
    buscfg.max_transfer_sz = 6 * 1024;

    spi_lobo_device_interface_config_t devcfg;
    memset((void *) &devcfg, 0, sizeof(devcfg));
    devcfg.clock_speed_hz   = 8000000;                  // Initial clock out at 8 MHz
    devcfg.mode             = 0;                        // SPI mode 0
    devcfg.spics_io_num     = -1;                       // we will use external CS pin
    devcfg.spics_ext_io_num = PIN_NUM_CS;               // external CS pin
    devcfg.flags            = LB_SPI_DEVICE_HALFDUPLEX; // ALWAYS SET  to HALF DUPLEX MODE!! for display spi

    tft_disp_type   = DISP_TYPE_ILI9341;
    tft_max_rdclock = 8000000;

    // Initialize all pins used by display driver.
    TFT_PinsInit();

    // Initialize SPI bus and add a device for the display.
    err = spi_lobo_bus_add_device(TFT_HSPI_HOST, &buscfg, &devcfg, &spi);
    if (err != ESP_OK)
        return err;

    // Configure the display to use the new SPI device.
    tft_disp_spi = spi;

    err = spi_lobo_device_select(spi, 1);
    if (err != ESP_OK)
        return err;
    err = spi_lobo_device_deselect(spi);
    if (err != ESP_OK)
        return err;

    // Initialize the display driver.
    TFT_display_init();

    // ---- Detect maximum read speed ----
    tft_max_rdclock = find_rd_speed();

    // Set the SPI clock speed.
    spi_lobo_set_speed(spi, DEFAULT_SPI_CLOCK);

    TFT_setGammaCurve(0);
    TFT_setRotation(LANDSCAPE);
    TFT_resetclipwin();

    DisplayWidth  = (uint16_t)(1 + tft_dispWin.x2 - tft_dispWin.x1);
    DisplayHeight = (uint16_t)(1 + tft_dispWin.y2 - tft_dispWin.y1);

    ESP_LOGI(TAG, "Display initialized (height %u, width %u)", DisplayHeight, DisplayWidth);

    // for some reason this is backwards (turns out this is because of a 2019 update to the m5stack hw)
    TFT_invertDisplay(INVERT_ON);

    // prepare the display for brightness control
    SetupBrightnessControl();
    displayTimer = xTimerCreate("DisplayTimer", pdMS_TO_TICKS(DISPLAY_TIMEOUT_MS), false, NULL, TimerCallback);

    // lower the brightness of the screen
    WakeDisplay();

    return err;
}

void SetBrightness(uint16_t brightness)
{
    uint16_t brightness_percent = (brightness * BRIGHTNESS_MAX) / 100;
    if (ledc_set_duty(LEDC_HIGH_SPEED_MODE, BACKLIGHT_CHANNEL, brightness_percent) ||
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, BACKLIGHT_CHANNEL))
    {
        ESP_LOGE(TAG, "Failed to set display brightness...");
    }
}

void WakeDisplay()
{
    SetBrightness(DEF_BRIGHTNESS_PERCENT);
    xTimerStart(displayTimer, 0);
    ESP_LOGI(TAG, "Display awake but will switch off automatically in %d seconds", DISPLAY_TIMEOUT_MS / 1000);
}

void ClearDisplay()
{
    TFT_fillRect(0, 0, (int) DisplayWidth, (int) DisplayHeight, TFT_BLACK);
}

void DisplayStatusMessage(char * msg, uint16_t vpos)
{
    TFT_setFont(SMALL_FONT, NULL);
    uint16_t msgX = 0;
    uint16_t msgY = (DisplayHeight * vpos) / 100;
    TFT_print(msg, msgX, msgY);
}

void TimerCallback(TimerHandle_t xTimer)
{
    ESP_LOGI(TAG, "Display going to sleep...");
    SetBrightness(0);
}

void SetupBrightnessControl()
{
    ledc_timer_config_t ledc_timer;
    memset(&ledc_timer, 0, sizeof(ledc_timer));

    ledc_timer.duty_resolution = LEDC_TIMER_8_BIT;     // resolution of PWM duty
    ledc_timer.freq_hz         = 1000;                 // frequency of PWM signal
    ledc_timer.speed_mode      = LEDC_HIGH_SPEED_MODE; // timer mode
    ledc_timer.timer_num       = LEDC_TIMER_0;         // timer index
    ledc_timer_config(&ledc_timer);

    ledc_timer_set(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, 1000, LEDC_TIMER_8_BIT, LEDC_REF_TICK);

    ledc_channel_config_t ledc_channel;
    memset(&ledc_channel, 0, sizeof(ledc_channel));
    ledc_channel.channel    = BACKLIGHT_CHANNEL;
    ledc_channel.duty       = BRIGHTNESS_MAX;
    ledc_channel.gpio_num   = PIN_NUM_BCKL;
    ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
    ledc_channel.timer_sel  = LEDC_TIMER_0;
    ledc_channel_config(&ledc_channel);
}

#endif // CONFIG_HAVE_DISPLAY

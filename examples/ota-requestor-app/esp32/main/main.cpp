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

#include "CHIPDeviceManager.h"
#include "DeviceCallbacks.h"
#include "app/util/af-enums.h"
#include "app/util/af.h"
#include "esp_heap_caps_init.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <app/server/Server.h>

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <lib/support/ErrorStr.h>

#include "OTARequesterImpl.h"
#include <argtable3/argtable3.h>
#include <esp_console.h>

using namespace ::chip;
using namespace ::chip::System;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;

struct CmdArgs
{
    struct arg_str * ipAddr;
    struct arg_int * nodeId;
    struct arg_end * end;
};

namespace {
const char * TAG = "ota-requester-app";
static DeviceCallbacks EchoCallbacks;
CmdArgs queryImageCmdArgs, applyUpdateCmdArgs;
} // namespace

void QueryImageTimerHandler(Layer * systemLayer, void * appState)
{
    ESP_LOGI(TAG, "Calling SendQueryImageCommand()");
    OTARequesterImpl::GetInstance().SendQueryImageCommand(queryImageCmdArgs.ipAddr->sval[0], queryImageCmdArgs.nodeId->ival[0]);
}

void ApplyUpdateTimerHandler(Layer * systemLayer, void * appState)
{
    ESP_LOGI(TAG, "Calling SendApplyUpdateRequestCommand()");
    OTARequesterImpl::GetInstance().SendApplyUpdateRequestCommand(queryImageCmdArgs.ipAddr->sval[0],
                                                                  queryImageCmdArgs.nodeId->ival[0]);
}

int ESPQueryImageCmdHandler(int argc, char ** argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &queryImageCmdArgs);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, queryImageCmdArgs.end, argv[0]);
        return 1;
    }
    ESP_LOGI(TAG, "ipAddr:%s nodeId:%x", queryImageCmdArgs.ipAddr->sval[0], queryImageCmdArgs.nodeId->ival[0]);

    /* Start one shot timer with 1 second timeout to send ApplyUpdateRequest command */
    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(1 * 1000), QueryImageTimerHandler, nullptr);
    return 0;
}

int ESPApplyUpdateCmdHandler(int argc, char ** argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &applyUpdateCmdArgs);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, applyUpdateCmdArgs.end, argv[0]);
        return 1;
    }
    ESP_LOGI(TAG, "ipAddr:%s nodeId:%x", applyUpdateCmdArgs.ipAddr->sval[0], applyUpdateCmdArgs.nodeId->ival[0]);

    /* Start one shot timer with 1 second timeout to Query for OTA image */
    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(1 * 1000), ApplyUpdateTimerHandler, nullptr);
    return 0;
}

void ESPInitConsole(void)
{
    esp_console_repl_t * repl                = NULL;
    esp_console_repl_config_t replConfig     = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    esp_console_dev_uart_config_t uartConfig = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    replConfig.prompt = "esp32 >";

    esp_console_register_help_command();

    esp_console_cmd_t queryImageCommand, applyUpdateCommand;
    memset(&queryImageCommand, 0, sizeof(queryImageCommand));
    memset(&applyUpdateCommand, 0, sizeof(applyUpdateCommand));

    queryImageCmdArgs.ipAddr = arg_str0(NULL, NULL, "<ipv4>", "OTA Provider IP Address");
    queryImageCmdArgs.nodeId = arg_int0(NULL, NULL, "<nodeID>", "OTA Provider Node ID in decimal");
    queryImageCmdArgs.end    = arg_end(1);

    queryImageCommand.command = "QueryImage", queryImageCommand.help = "Query for OTA image",
    queryImageCommand.func = &ESPQueryImageCmdHandler, queryImageCommand.argtable = &queryImageCmdArgs;

    applyUpdateCmdArgs.ipAddr = arg_str0(NULL, NULL, "<ipv4>", "OTA Provider IP Address");
    applyUpdateCmdArgs.nodeId = arg_int0(NULL, NULL, "<nodeID>", "OTA Provider Node ID in decimal");
    applyUpdateCmdArgs.end    = arg_end(1);

    applyUpdateCommand.command = "ApplyUpdateRequest", applyUpdateCommand.help = "Request to OTA update image",
    applyUpdateCommand.func = &ESPApplyUpdateCmdHandler, applyUpdateCommand.argtable = &applyUpdateCmdArgs;

    esp_console_cmd_register(&queryImageCommand);
    esp_console_cmd_register(&applyUpdateCommand);

    esp_console_new_repl_uart(&uartConfig, &replConfig, &repl);
    esp_console_start_repl(repl);
}

extern "C" void app_main()
{
    ESP_LOGI(TAG, "OTA Requester!");

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

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    CHIP_ERROR error = deviceMgr.Init(&EchoCallbacks);
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "device.Init() failed: %s", ErrorStr(error));
        return;
    }

    chip::Server::GetInstance().Init();

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    ESPInitConsole();
}

// TODO: We should use the function definition in /src/app/clusters/ota-requestor/ClusterInterface.cpp
// Temporarily add this function.

bool emberAfOtaSoftwareUpdateRequestorClusterAnnounceOtaProviderCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & commandData)
{
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

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

#include "esp_log.h"
#include "pw_sys_io/sys_io.h"
#include "pw_sys_io_esp32/init.h"

const char * TAG = "pw-rpc-app";

namespace hdlc_example {
extern void Start();
} // namespace hdlc_example
extern "C" void app_main()
{
    pw_sys_io_Init();

    ESP_LOGI(TAG, "----------- chip-esp32-pigweed-example starting -----------");
    hdlc_example::Start();
}

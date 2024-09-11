/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#include <esp_err.h>
#include <esp_openthread_types.h>
#include <lib/core/CHIPError.h>
#include <memory>

#if defined(CONFIG_OPENTHREAD_BORDER_ROUTER) && defined(CONFIG_AUTO_UPDATE_RCP)
#include <esp_rcp_update.h>
esp_err_t openthread_init_br_rcp(const esp_rcp_update_config_t * update_config);
#endif // CONFIG_OPENTHREAD_BORDER_ROUTER && CONFIG_AUTO_UPDATE_RCP
esp_err_t set_openthread_platform_config(esp_openthread_platform_config_t * config);
esp_err_t openthread_init_stack(void);
esp_err_t openthread_launch_task(void);
CHIP_ERROR cli_transmit_task_post(std::unique_ptr<char[]> && cli_str);

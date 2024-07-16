/**
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

#pragma once

#include <lib/core/CHIPConfig.h>

#if !CHIP_CONFIG_SKIP_APP_SPECIFIC_GENERATED_HEADER_INCLUDES

#include <zap-generated/gen_config.h>

#include <zap-generated/endpoint_config.h>

#endif // !CHIP_CONFIG_SKIP_APP_SPECIFIC_GENERATED_HEADER_INCLUDES

// User options for plugin Binding Table Library
// TODO: Make this a CHIP_CONFIG value.
#ifndef MATTER_BINDING_TABLE_SIZE
#define MATTER_BINDING_TABLE_SIZE 10
#endif // MATTER_BINDING_TABLE_SIZE

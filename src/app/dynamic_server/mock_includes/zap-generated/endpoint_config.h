/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/util/endpoint-config-defines.h>
#include <lib/core/CHIPConfig.h>

// Some overrides to see that this is not used as a real "endpoint_config" that contains data
#define GENERATED_DEFAULTS #error "Not a real codegen. This is a temporary include for dynamic-overrides only"
#define GENERATED_CLUSTERS #error "Not a real codegen. This is a temporary include for dynamic-overrides only"

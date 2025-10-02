/*
 *    Copyright (c) 2021-2025 Project CHIP Authors
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

/**
 * @file
 *   Legacy API for the Identify cluster.
 *
 *   This file provides backward compatibility for existing codegen-based implementations
 *   by redirecting to the new code-driven implementation located in IdentifyCluster.h/.cpp.
 *   New implementations should use IdentifyCluster.h/.cpp directly.
 */

#pragma once

#include <app/clusters/identify-server/CodegenIntegration.h>
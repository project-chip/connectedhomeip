/*
 *
 *    Copyright (c) 2021-2025 Project CHIP Authors
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

// Shim header for CodegenIntegration.h to allow indirect inclusion.
//
// This file preserves compatibility with existing examples that previously included
// diagnostic-logs-server.h. It serves as a drop-in replacement and internally includes
// CodegenIntegration.h to avoid changes in example code.

#pragma once
#include <app/clusters/diagnostic-logs-server/CodegenIntegration.h>

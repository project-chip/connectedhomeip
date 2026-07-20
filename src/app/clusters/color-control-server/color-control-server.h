/**
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

// Backward-compat shim. The legacy Ember `ColorControlServer` singleton has been replaced by the
// code-driven `ColorControlCluster`. Application code that included this header now gets the codegen
// integration surface (per-endpoint cluster lookup + delegate registration). Callers that used
// `ColorControlServer::Instance().<command>(endpoint, ...)` should switch to
// `ColorControl::FindClusterOnEndpoint(endpoint)-><command>(...)`.
#include "CodegenIntegration.h" // nogncheck

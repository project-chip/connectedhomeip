/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

/*
 * This function performs a memory leak check if the build flag `enable_leak_checking` is set to true
 * If leaks are detected, it overrides the provided exit code with `EXIT_FAILURE`.
 *
 * @param exitCode The initial exit code to return if no leaks are detected or if leak checking is disabled.
 * @return `EXIT_FAILURE` if leaks are detected and leak checking is enabled; otherwise, the original `exitCode`.
 */
int ConditionalLeaksCheck(int exitCode);

/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2017 Nest Labs, Inc.
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

/**
 *    @file
 *      Namespace redirection header file for CHIP Echo profile.
 *
 */

#ifndef CHIP_ECHO_H_
#define CHIP_ECHO_H_

#include <support/ManagedNamespace.h>

// Select "Current" version by default.
#ifndef CHIP_CONFIG_ECHO_NAMESPACE
#define CHIP_CONFIG_ECHO_NAMESPACE kChipManagedNamespace_Current
#endif

// Include the appropriate header file based on the requested version, or fail if the requested version doesn't exist.
#if CHIP_CONFIG_ECHO_NAMESPACE == kChipManagedNamespace_Current
#include <Profiles/echo/Current/CHIPEcho.h>
#elif CHIP_CONFIG_ECHO_NAMESPACE == kChipManagedNamespace_Next
#include <Profiles/echo/Next/CHIPEcho.h>
#else
#error "Invalid CHIP_CONFIG_ECHO_NAMESPACE selected"
#endif

#endif // CHIP_ECHO_H_

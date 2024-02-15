/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    Copyright (c) 2023 NXP
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
 *          Provides the SMU2 namespace for K32W1 platform using the NXP SDK.
 *          This namespace implements all the necessary function to allocate
 *          OpenThread buffers from SMU2 region.
 */

#pragma once

#include "fsl_component_mem_manager.h"
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/DefaultStorageKeyAllocator.h>

namespace chip::SMU2 {

CHIP_ERROR Init(PersistentStorageDelegate * storage);
CHIP_ERROR Deactivate(void);
void * Allocate(size_t size);

} // namespace chip::SMU2

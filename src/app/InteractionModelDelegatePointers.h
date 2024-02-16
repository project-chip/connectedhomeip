/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#pragma once

#include <app/AppConfig.h>
#include <lib/support/static_support_smart_ptr.h>

namespace chip {

#if CHIP_CONFIG_STATIC_GLOBAL_INTERACTION_MODEL_ENGINE

template <class T>
using InteractionModelDelegatePointer = chip::CheckedGlobalInstanceReference<T>;

#else

template <class T>
using InteractionModelDelegatePointer = chip::SimpleInstanceReference<T>;

#endif // CHIP_CONFIG_STATIC_GLOBAL_INTERATION_MODEL_ENGINE

} // namespace chip

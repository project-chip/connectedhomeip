/*
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

#if CHIP_CONFIG_USE_EMBER_DATA_MODEL && CHIP_CONFIG_USE_DATA_MODEL_INTERFACE
#include <app/reporting/Read-Checked.h>
#else
#if CHIP_CONFIG_USE_DATA_MODEL_INTERFACE
#include <app/reporting/Read-DataModel.h>
#else
#include <app/reporting/Read-Ember.h>
#endif // CHIP_CONFIG_USE_DATA_MODEL_INTERFACE
#endif // CHIP_CONFIG_USE_EMBER_DATA_MODEL && CHIP_CONFIG_USE_DATA_MODEL_INTERFACE

namespace chip {
namespace app {
namespace reporting {

#if CHIP_CONFIG_USE_EMBER_DATA_MODEL && CHIP_CONFIG_USE_DATA_MODEL_INTERFACE
namespace Impl = CheckedImpl;
#else
#if CHIP_CONFIG_USE_DATA_MODEL_INTERFACE
namespace Impl = DataModelImpl;
#else
namespace Impl = EmberImpl;
#endif // CHIP_CONFIG_USE_DATA_MODEL_INTERFACE
#endif // CHIP_CONFIG_USE_EMBER_DATA_MODEL && CHIP_CONFIG_USE_DATA_MODEL_INTERFACE

} // namespace reporting
} // namespace app
} // namespace chip

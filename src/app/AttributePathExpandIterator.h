/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <app/AttributePathExpandIterator-Checked.h>
#else
#if CHIP_CONFIG_USE_DATA_MODEL_INTERFACE
#include <app/AttributePathExpandIterator-DataModel.h>
#else
#include <app/AttributePathExpandIterator-Ember.h>
#endif // CHIP_CONFIG_USE_DATA_MODEL_INTERFACE
#endif // CHIP_CONFIG_USE_EMBER_DATA_MODEL && CHIP_CONFIG_USE_DATA_MODEL_INTERFACE

namespace chip {
namespace app {

#if CHIP_CONFIG_USE_EMBER_DATA_MODEL && CHIP_CONFIG_USE_DATA_MODEL_INTERFACE
using AttributePathExpandIterator = ::chip::app::AttributePathExpandIteratorChecked;
#else
#if CHIP_CONFIG_USE_DATA_MODEL_INTERFACE
using AttributePathExpandIterator = ::chip::app::AttributePathExpandIteratorDataModel;
#else
using AttributePathExpandIterator = ::chip::app::AttributePathExpandIteratorEmber;
#endif // CHIP_CONFIG_USE_DATA_MODEL_INTERFACE
#endif // CHIP_CONFIG_USE_EMBER_DATA_MODEL && CHIP_CONFIG_USE_DATA_MODEL_INTERFACE

} // namespace app
} // namespace chip

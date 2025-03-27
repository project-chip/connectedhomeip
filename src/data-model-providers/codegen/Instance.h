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

#include "lib/core/CHIPPersistentStorageDelegate.h"
#include <app/data-model-provider/Provider.h>

namespace chip {
namespace app {

/// Gets an instance of a global data model provider that is based off a code generated
/// (i.e. ember framework) data storage
///
/// @param delegate - determines the storage delegate to be used by the returned provider
///                   in case default ember storage is required. Applications can also
///                   call SetAttributeStorageProvider themselves and provide nullptr here
///                   if required.
DataModel::Provider * CodegenDataModelProviderInstance(PersistentStorageDelegate * delegate);

} // namespace app
} // namespace chip

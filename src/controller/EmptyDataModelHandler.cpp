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

// The controller library requires DataModelHandler functions which are usually
// provided by a specific controller application, since the functions depend on
// the code generated based on application-specific clusters. Implement empty
// defaults so that the controller library is self-sufficient and a controller
// which has not yet implemented ZCL handlers builds without linker errors.

#include <app/util/DataModelHandler.h>

__attribute__((weak)) void InitDataModelHandler(chip::Messaging::ExchangeManager * exchangeMgr) {}

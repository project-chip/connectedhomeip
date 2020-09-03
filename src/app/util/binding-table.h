/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @file API declarations for a binding table.
 */

#ifndef BINDING_TABLE_H
#define BINDING_TABLE_H

#include <app/util/af-types.h>

// Should this be configurable by the app somehow?
#define BINDING_TABLE_SIZE 10

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

EmberStatus emberGetBinding(uint8_t index, EmberBindingTableEntry * result);

EmberStatus emberSetBinding(uint8_t index, EmberBindingTableEntry * result);

EmberStatus emberDeleteBinding(uint8_t index);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // BINDING_TABLE_H

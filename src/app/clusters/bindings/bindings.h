/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app/clusters/bindings/BindingManager.h>
#include <app/util/binding-table.h>

/**
 * @brief appends a binding to the list of bindings
 *        This function is to be used when a device wants to add a binding to its own table
 *        If entry is a unicast binding, BindingManager will be notified and will establish a case session with the peer device
 *        Entry will be added to the binding table and persisted into storage
 *        BindingManager will be notified and the binding added callback will be called if it has been set
 *
 * @param entry binding to add
 */
CHIP_ERROR AddBindingEntry(const EmberBindingTableEntry & entry);

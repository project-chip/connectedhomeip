/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

struct AppEvent; // Needs to be implemented in the application code.
using EventHandler = void (*)(const AppEvent &);

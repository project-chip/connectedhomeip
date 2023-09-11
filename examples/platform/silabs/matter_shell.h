/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace chip {

void NotifyShellProcess();
void NotifyShellProcessFromISR();
void WaitForShellActivity();
void startShellTask();

} // namespace chip

/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace chip {

void NotifyShellProcess(void);
void NotifyShellProcessFromISR(void);
void WaitForShellActivity(void);
void startShellTask(void);

} // namespace chip

/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// This hack is needed because Ameba SDK is not linking against libstdc++ correctly.
extern "C" {
void * __dso_handle = 0;
}

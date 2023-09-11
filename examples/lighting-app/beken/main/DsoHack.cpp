/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// This hack is needed because Beken SDK is not linking against libstdc++ correctly.
extern "C" {
void * __dso_handle = 0;
}

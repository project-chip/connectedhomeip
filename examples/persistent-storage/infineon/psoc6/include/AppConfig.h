/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// P6 Logging
#ifdef __cplusplus
extern "C" {
#endif

void appError(int err);
void P6Log(const char * aFormat, ...);
#define P6_LOG(...) P6Log(__VA_ARGS__)

#ifdef __cplusplus
}
#endif

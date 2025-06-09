/*
 * Copyright (c) 2025 Telink Semiconductor (Shanghai) Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif

#define SPK_TEST_ENABLE 0
#define DATA_TRANS_INTERVAL_MS 500
#define SYNC_SAMPLE 16 * (DATA_TRANS_INTERVAL_MS)
#define CATEGORY_YES_TRIGGER_THRESHOLD 9000 // 90%
#define CATEGORY_NO_TRIGGER_THRESHOLD 7500  // 75%

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif

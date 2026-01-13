/*
 * Copyright (c) 2025 Telink Semiconductor (Shanghai) Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef VENDOR_AUDIO_DEMO_TFLM_APP_MICRO_SPEECH_MICRO_SPEECH_TEST_H_
#define VENDOR_AUDIO_DEMO_TFLM_APP_MICRO_SPEECH_MICRO_SPEECH_TEST_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int micro_speech_process(int16_t * p_buff, uint16_t len);

#ifdef __cplusplus
}
#endif
#endif

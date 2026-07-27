/*
 * Copyright (c) 2025 Telink Semiconductor (Shanghai) Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef VENDOR_AUDIO_DEMO_APP_CODEC_H_
#define VENDOR_AUDIO_DEMO_APP_CODEC_H_

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif

#include "../micro_speech/tflite_micro_micro_speech.h"
#include "app_audio.h"

void app_codec_init(void);

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif

#endif /* VENDOR_AUDIO_DEMO_APP_CODEC_H_ */

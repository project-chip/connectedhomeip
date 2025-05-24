/*
 * Copyright (c) 2025 Telink Semiconductor (Shanghai) Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef VENDOR_AUDIO_DEMO_APP_AUDIO_H_
#define VENDOR_AUDIO_DEMO_APP_AUDIO_H_

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif

#include "app_config.h"
#include <stdint.h>

#define PLAYBACK_FIFO_SIZE 1
#define RECORD_FIFO_SIZE 8192
#define PLAYBACK_FIFO_MAX (PLAYBACK_FIFO_SIZE - 1)
#define RECORD_FIFO_MAX (RECORD_FIFO_SIZE - 1)

typedef struct
{
    int16_t mic_rptr;
    int16_t mic_wptr;
    int16_t play_wptr;
    int16_t play_rptr;
    uint8_t status;
} codec_cfg_t;

__attribute__((section(".ram_code"))) __attribute__((noinline)) uint8_t codec_get_rx_data(int16_t * p_des, uint16_t len);
__attribute__((section(".ram_code"))) __attribute__((noinline)) uint32_t codec_get_mic_wptr(void);
__attribute__((section(".ram_code"))) __attribute__((noinline)) void codec_sync_mic_samples(uint16_t samples);
__attribute__((section(".ram_code"))) __attribute__((noinline)) void codec_post_pcm_2_playback_buffer(int16_t * pcm,
                                                                                                      uint16_t num_of_samples);

void tflite_micro_micro_speech_process_action(int32_t * result);

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif

#endif /* VENDOR_AUDIO_DEMO_APP_AUDIO_H_ */

/*
 * Copyright (c) 2025 Telink Semiconductor (Shanghai) Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef AUDIO_COMMON_H
#define AUDIO_COMMON_H

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif

#include "audio.h"

/**
 * @brief       This function serves to fade stream digital gain
 * @param[in]   gain          - audio channel select.
 * @return      none.
 */
void audio_stream0_fade_dig_gain(codec_in_path_digital_gain_e gain);

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif

#endif

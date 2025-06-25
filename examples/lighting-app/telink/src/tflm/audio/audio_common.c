/*
 * Copyright (c) 2025 Telink Semiconductor (Shanghai) Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app_config.h"
#include "audio.h"

/**
 * @brief       This function serves to fade stream digital gain
 * @param[in]   gain          - audio channel select.
 * @return      none.
 */
void audio_stream0_fade_dig_gain(codec_in_path_digital_gain_e gain)
{
    codec_in_path_digital_gain_e value = audio_get_stream0_dig_gain();

    if (gain > value)
    {
        for (value += 4; (signed int) value <= (signed int) gain; value += 4)
        {
            delay_ms(1);
            audio_set_stream0_dig_gain(value);
        }
    }
    else if (gain < value)
    {
        for (value -= 4; (signed int) value >= (signed int) gain; value -= 4)
        {
            delay_ms(1);
            audio_set_stream0_dig_gain(value);
        }
    }
}

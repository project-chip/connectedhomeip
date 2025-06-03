/*
 * Copyright (c) 2025 Telink Semiconductor (Shanghai) Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app_audio.h"
#include "../micro_speech/tflite_micro_micro_speech.h"
#include "app_codec.h"
#include "app_config.h"
#include "audio.h"
#include "audio_common.h"
#include <zephyr/kernel.h>

#define kCategoryCount 4

static codec_cfg_t g_codec_cfg;
extern int16_t buff_record[];
extern int16_t buff_playback[];
extern float category_predictions[];

extern int prediction_index;

static int16_t adc_int;

static int category_predictions_int[kCategoryCount];

static const char * kCategoryLabels[kCategoryCount] = {
    "silence",
    "unknown",
    "yes",
    "no",
};

__attribute__((section(".ram_code"))) __attribute__((noinline)) uint32_t codec_get_mic_wptr(void)
{
    return ((audio_get_rx_dma_wptr(DMA0) - (uint32_t) buff_record) / sizeof(adc_int));
}

__attribute__((section(".ram_code"))) __attribute__((noinline)) void codec_sync_mic_samples(uint16_t samples)
{
    uint16_t wptr = codec_get_mic_wptr();

    g_codec_cfg.mic_rptr = (wptr - samples) & RECORD_FIFO_MAX;
}

__attribute__((section(".ram_code"))) __attribute__((noinline)) uint8_t codec_get_rx_data(int16_t * p_des, uint16_t len)
{
    uint16_t wptr;
    uint16_t num;
    uint16_t i;

    short * pd = (short *) p_des;

    wptr = codec_get_mic_wptr();
    num  = (wptr - g_codec_cfg.mic_rptr) & RECORD_FIFO_MAX;

    if (num >= len)
    {
        for (i = 0; i < len; i++)
        {
            *pd++                = buff_record[g_codec_cfg.mic_rptr++] * 5; // L R
            g_codec_cfg.mic_rptr = g_codec_cfg.mic_rptr & RECORD_FIFO_MAX;
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

__attribute__((section(".ram_code"))) __attribute__((noinline)) void codec_post_pcm_2_playback_buffer(int16_t * pcm,
                                                                                                      uint16_t num_of_samples)
{
    int16_t s0;

    while (num_of_samples--)
    {
        s0                                     = *pcm++;
        buff_playback[g_codec_cfg.play_wptr++] = s0;
        g_codec_cfg.play_wptr &= PLAYBACK_FIFO_MAX;
    }
}

static int ret_main                  = 0;
static int16_t g_enc_buff[16 * 1000] = { 0 };
static int16_t enc_left_offset       = 0;

void tflite_micro_micro_speech_process_action(int32_t * result)
{
    uint8_t res = 0;

    *result = 0;

    if (0 == g_codec_cfg.status)
    {
        g_codec_cfg.status = 1;
        codec_sync_mic_samples(SYNC_SAMPLE);
    }

    int16_t * p_buff = g_enc_buff + enc_left_offset;

    codec_get_rx_data(p_buff, SYNC_SAMPLE);
    enc_left_offset += SYNC_SAMPLE;

    if (enc_left_offset == 16000)
    {
        enc_left_offset = 0;
        ret_main        = 1;
    }

#if SPK_TEST_ENABLE
    codec_post_pcm_2_playback_buffer((int16_t *) p_buff, SYNC_SAMPLE);
#else
    if (ret_main == 1)
    {
        res = micro_speech_process(g_enc_buff, 16000);

        category_predictions_int[2] = category_predictions[2] * 10000;
        category_predictions_int[3] = category_predictions[3] * 10000;

        if (res == 2 && category_predictions_int[2] >= CATEGORY_YES_TRIGGER_THRESHOLD)
        {
            // gpio_set_high_level(GPIO_PC0); //Blue LED
            printk("***********YES***********\n");
            *result = 2;
        }
        else if (res == 3 && category_predictions_int[3] >= CATEGORY_NO_TRIGGER_THRESHOLD)
        {
            // gpio_set_low_level(GPIO_PC0);
            printk("***********NO***********\n");
            *result = 3;
        }

        printk("%s: %d, ", kCategoryLabels[2], category_predictions_int[2]);
        printk("%s: %d\n", kCategoryLabels[3], category_predictions_int[3]);
    }
#endif
}

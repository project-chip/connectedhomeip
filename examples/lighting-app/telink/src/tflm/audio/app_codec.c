/*
 * Copyright (c) 2025 Telink Semiconductor (Shanghai) Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app_codec.h"
#include "app_config.h"
#include "audio.h"
#include "audio_common.h"

// #define AUDIO_BUFF_SIZE 4096 * 2 /* In order to support codec data fade-in process, define enough buff */

// When the codec data bit width is selected as 16bit, the following buf is used,
// when the data bit width is selected as 20bit,
// you can define a buf with the same size of the signed int type for use.
// signed short AUDIO_BUFF[AUDIO_BUFF_SIZE >> 1] __attribute__((aligned(4)));

int16_t buff_record[RECORD_FIFO_SIZE];
int16_t buff_playback[PLAYBACK_FIFO_SIZE];

sdm_pin_config_t sdm_pin_config = {
    .sdm0_p_pin = GPIO_FC_PA0, // Both the SDM and printf print functions use the PA0 pin. If the SDM function is used, modify the
                               // pin used for DEBUG_INFO_TX_PIN in printf.h.
    .sdm0_n_pin = GPIO_FC_PA1,
    .sdm1_p_pin = GPIO_FC_PF4,
    .sdm1_n_pin = GPIO_FC_PF5,
};

#define SAMPLE_RATE AUDIO_16K
#define DATA_WIDTH CODEC_BIT_16_DATA
#define RX_FIFO_NUM FIFO0
#define TX_FIFO_NUM FIFO0 // TX Hardware is fixed to FIFO and cannot be modified.
#define RX_DMA_CHN DMA0
#define TX_DMA_CHN DMA1
#define INPUT_SRC DMIC_STREAM0_MONO_L
#define OUTPUT_SRC SDM_MONO

audio_codec_stream0_input_t audio_codec_stream0_input = {
    .input_src     = INPUT_SRC,
    .sample_rate   = SAMPLE_RATE,
    .data_width    = DATA_WIDTH,
    .fifo_chn      = RX_FIFO_NUM,
    .dma_num       = RX_DMA_CHN,
    .data_buf      = buff_record,
    .data_buf_size = sizeof(buff_record),
};
audio_codec_output_t audio_stream_output = {
    .output_src    = OUTPUT_SRC,
    .sample_rate   = SAMPLE_RATE,
    .data_width    = DATA_WIDTH,
    .dma_num       = TX_DMA_CHN,
    .data_buf      = buff_playback,
    .data_buf_size = sizeof(buff_playback),
};

void app_codec_init()
{
    audio_init();
    if (CONFIG_SOC_RISCV_TELINK_TL721X)
    {
        audio_set_stream0_dmic_pin(GPIO_FC_PA2, GPIO_FC_PA3, GPIO_FC_PA4);
    }

    /****stream0 line in/amic/dmic init****/
    audio_codec_stream0_input_init(&audio_codec_stream0_input);
    /****line output init****/

    audio_codec_stream_output_init(&audio_stream_output);

    /****rx tx dma init****/
    audio_rx_dma_chain_init(audio_codec_stream0_input.fifo_chn, audio_codec_stream0_input.dma_num,
                            (unsigned short *) audio_codec_stream0_input.data_buf, audio_codec_stream0_input.data_buf_size);
    audio_tx_dma_chain_init(TX_FIFO_NUM, audio_stream_output.dma_num, (unsigned short *) audio_stream_output.data_buf,
                            audio_stream_output.data_buf_size);

    audio_mic_mute_en();                                             /* Step1 - mute audio*/
    audio_codec_stream0_input_en(audio_codec_stream0_input.dma_num); /* Step2 - enable audio codec */
    audio_codec_clr_input_pop(20);                                   /* Step3 - Clear codec input pop and dis mute audio */
    audio_codec_input_path_en(audio_codec_stream0_input.fifo_chn);   /* Step4 - enable codec input path, codec data come in */

    audio_stream0_fade_dig_gain(CODEC_IN_D_GAIN_m6_DB);

    audio_set_sdm_pin(&sdm_pin_config);
    audio_codec_stream_output_en(audio_stream_output.dma_num);
}

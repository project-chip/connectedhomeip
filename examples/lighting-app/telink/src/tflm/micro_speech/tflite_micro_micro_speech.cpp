/* Copyright 2024 The TensorFlow Authors. All Rights Reserved.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ==============================================================================*/

/*
 * Copyright (c) 2025 Telink Semiconductor (Shanghai) Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <algorithm>
#include <cstdint>
#include <iterator>

#include "audio_preprocessor_int8_model_data.h"
#include "micro_model_settings.h"
#include "micro_speech_quantized_model_data.h"
#include "tensorflow/lite/core/c/common.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/testing/micro_test.h"
#include "tflite_micro_micro_speech.h"

#define TIMER_100MS_EN 0
#define TIMER_200MS_EN 0
#define TIMER_500MS_EN 1

constexpr int kAudioLength = 16000;

static int16_t input_buffer[kAudioLength];

static int prediction_index;

extern "C" {
float category_predictions[kCategoryCount];
}

namespace {

// Arena size is a guesstimate, followed by use of
// MicroInterpreter::arena_used_bytes() on both the AudioPreprocessor and
// MicroSpeech models and using the larger of the two results.

constexpr size_t kArenaSize = 28584; // xtensa p6
alignas(16) uint8_t g_arena[kArenaSize];

using Features = int8_t[kFeatureCount][kFeatureSize];
Features g_features;

constexpr int kAudioSampleDurationCount = kFeatureDurationMs * kAudioSampleFrequency / 1000;
constexpr int kAudioSampleStrideCount   = kFeatureStrideMs * kAudioSampleFrequency / 1000;

using MicroSpeechOpResolver       = tflite::MicroMutableOpResolver<4>;
using AudioPreprocessorOpResolver = tflite::MicroMutableOpResolver<18>;

TfLiteStatus RegisterOps(MicroSpeechOpResolver & op_resolver)
{
    TF_LITE_ENSURE_STATUS(op_resolver.AddReshape());
    TF_LITE_ENSURE_STATUS(op_resolver.AddFullyConnected());
    TF_LITE_ENSURE_STATUS(op_resolver.AddDepthwiseConv2D());
    TF_LITE_ENSURE_STATUS(op_resolver.AddSoftmax());
    return kTfLiteOk;
}

TfLiteStatus RegisterOps(AudioPreprocessorOpResolver & op_resolver)
{
    TF_LITE_ENSURE_STATUS(op_resolver.AddReshape());
    TF_LITE_ENSURE_STATUS(op_resolver.AddCast());
    TF_LITE_ENSURE_STATUS(op_resolver.AddStridedSlice()); //
    TF_LITE_ENSURE_STATUS(op_resolver.AddConcatenation());
    TF_LITE_ENSURE_STATUS(op_resolver.AddMul());
    TF_LITE_ENSURE_STATUS(op_resolver.AddAdd());
    TF_LITE_ENSURE_STATUS(op_resolver.AddDiv());
    TF_LITE_ENSURE_STATUS(op_resolver.AddMinimum());
    TF_LITE_ENSURE_STATUS(op_resolver.AddMaximum());
    TF_LITE_ENSURE_STATUS(op_resolver.AddWindow());
    TF_LITE_ENSURE_STATUS(op_resolver.AddFftAutoScale());
    TF_LITE_ENSURE_STATUS(op_resolver.AddRfft()); //
    TF_LITE_ENSURE_STATUS(op_resolver.AddEnergy());
    TF_LITE_ENSURE_STATUS(op_resolver.AddFilterBank());
    TF_LITE_ENSURE_STATUS(op_resolver.AddFilterBankSquareRoot());
    TF_LITE_ENSURE_STATUS(op_resolver.AddFilterBankSpectralSubtraction());
    TF_LITE_ENSURE_STATUS(op_resolver.AddPCAN());
    TF_LITE_ENSURE_STATUS(op_resolver.AddFilterBankLog());
    return kTfLiteOk;
}

TfLiteStatus LoadMicroSpeechModelAndPerformInference(const Features & features)
{
    // Map the model into a usable data structure. This doesn't involve any
    // copying or parsing, it's a very lightweight operation.
    const tflite::Model * model = tflite::GetModel(g_micro_speech_quantized_model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        return kTfLiteError;
    }
    MicroSpeechOpResolver op_resolver;
    if (RegisterOps(op_resolver) != kTfLiteOk)
    {
        return kTfLiteError;
    }

    tflite::MicroInterpreter interpreter(model, op_resolver, g_arena, kArenaSize);

    if (interpreter.AllocateTensors() != kTfLiteOk)
    {
        return kTfLiteError;
    }

    TfLiteTensor * input = interpreter.input(0);
    if (input == nullptr)
    {
        return kTfLiteError;
    }
    // check input shape is compatible with our feature data size
    if (kFeatureElementCount != input->dims->data[input->dims->size - 1])
    {
        return kTfLiteError;
    }

    TfLiteTensor * output = interpreter.output(0);
    if (output == nullptr)
    {
        return kTfLiteError;
    }
    // check output shape is compatible with our number of prediction categories
    if (kCategoryCount != output->dims->data[output->dims->size - 1])
    {
        return kTfLiteError;
    }

    float output_scale    = output->params.scale;
    int output_zero_point = output->params.zero_point;

    std::copy_n(&features[0][0], kFeatureElementCount, tflite::GetTensorData<int8_t>(input));
    if (interpreter.Invoke() != kTfLiteOk)
    {
        return kTfLiteError;
    }

    // Dequantize output values
    for (int i = 0; i < kCategoryCount; i++)
    {
        category_predictions[i] = (tflite::GetTensorData<int8_t>(output)[i] - output_zero_point) * output_scale;
    }
    prediction_index = std::distance(std::begin(category_predictions),
                                     std::max_element(std::begin(category_predictions), std::end(category_predictions)));

    return kTfLiteOk;
}

TfLiteStatus GenerateSingleFeature(const int16_t * audio_data, const int audio_data_size, int8_t * feature_output,
                                   tflite::MicroInterpreter * interpreter)
{
    TfLiteTensor * input = interpreter->input(0);
    if (input == nullptr)
    {
        return kTfLiteError;
    }
    // check input shape is compatible with our audio sample size
    if (kAudioSampleDurationCount != audio_data_size)
    {
        return kTfLiteError;
    }
    if (kAudioSampleDurationCount != input->dims->data[input->dims->size - 1])
    {
        return kTfLiteError;
    }

    TfLiteTensor * output = interpreter->output(0);
    if (output == nullptr)
    {
        return kTfLiteError;
    }
    // check output shape is compatible with our feature size
    if (kFeatureSize != output->dims->data[output->dims->size - 1])
    {
        return kTfLiteError;
    }

    std::copy_n(audio_data, audio_data_size, tflite::GetTensorData<int16_t>(input));

    if (interpreter->Invoke() != kTfLiteOk)
    {
        return kTfLiteError;
    }
    std::copy_n(tflite::GetTensorData<int8_t>(output), kFeatureSize, feature_output);

    return kTfLiteOk;
}

TfLiteStatus GenerateFeatures(const int16_t * audio_data, const size_t audio_data_size, Features * features_output)
{
    // Map the model into a usable data structure. This doesn't involve any
    // copying or parsing, it's a very lightweight operation.
    const tflite::Model * model = tflite::GetModel(g_audio_preprocessor_int8_model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        return kTfLiteError;
    }

    AudioPreprocessorOpResolver op_resolver;

    if (RegisterOps(op_resolver) != kTfLiteOk)
    {
        return kTfLiteError;
    }

    tflite::MicroInterpreter interpreter(model, op_resolver, g_arena, kArenaSize);

    if (interpreter.AllocateTensors() != kTfLiteOk)
    {
        return kTfLiteError;
    }

    size_t remaining_samples = audio_data_size;
    size_t feature_index     = 0;
    while (remaining_samples >= kAudioSampleDurationCount && feature_index < kFeatureCount)
    {
        TF_LITE_ENSURE_STATUS(
            GenerateSingleFeature(audio_data, kAudioSampleDurationCount, (*features_output)[feature_index], &interpreter));
        feature_index++;
        audio_data += kAudioSampleStrideCount;
        remaining_samples -= kAudioSampleStrideCount;
    }

    return kTfLiteOk;
}

TfLiteStatus ProcessAudioSample(const int16_t * audio_data, const size_t audio_data_size)
{
    TF_LITE_ENSURE_STATUS(GenerateFeatures(audio_data, audio_data_size, &g_features));
    TF_LITE_ENSURE_STATUS(LoadMicroSpeechModelAndPerformInference(g_features));
    return kTfLiteOk;
}

} // namespace

static int16_t enc_right_offset = 0;
int micro_speech_process(int16_t * p_buff, uint16_t len)
{

    for (uint16_t i = 0; i < len; i++)
    {
        input_buffer[i] = p_buff[enc_right_offset++];
        enc_right_offset %= len;
    }

#if TIMER_100MS_EN
    enc_right_offset += 1600;
#elif TIMER_200MS_EN
    enc_right_offset += 3200;
#elif TIMER_500MS_EN
    enc_right_offset += 8000;
#endif

    ProcessAudioSample(input_buffer, kAudioLength);

    return prediction_index;
}

/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <app/CommandHandler.h>
#include <app/InteractionModelEngine.h>
#include <app/data-model/Nullable.h>
#include <app/util/af-enums.h>
#include <inttypes.h>
#include <lib/support/FunctionTraits.h>
#include <lib/support/Span.h>

// Global Response Callbacks
typedef void (*DefaultSuccessCallback)(void * context);
typedef void (*DefaultFailureCallback)(void * context, uint8_t status);
typedef void (*BooleanAttributeCallback)(void * context, bool value);
typedef void (*NullableBooleanAttributeCallback)(void * context, const chip::app::DataModel::Nullable<bool> & value);
typedef void (*Int8uAttributeCallback)(void * context, uint8_t value);
typedef void (*NullableInt8uAttributeCallback)(void * context, const chip::app::DataModel::Nullable<uint8_t> & value);
typedef void (*Int8sAttributeCallback)(void * context, int8_t value);
typedef void (*NullableInt8sAttributeCallback)(void * context, const chip::app::DataModel::Nullable<int8_t> & value);
typedef void (*Int16uAttributeCallback)(void * context, uint16_t value);
typedef void (*NullableInt16uAttributeCallback)(void * context, const chip::app::DataModel::Nullable<uint16_t> & value);
typedef void (*Int16sAttributeCallback)(void * context, int16_t value);
typedef void (*NullableInt16sAttributeCallback)(void * context, const chip::app::DataModel::Nullable<int16_t> & value);
typedef void (*Int32uAttributeCallback)(void * context, uint32_t value);
typedef void (*NullableInt32uAttributeCallback)(void * context, const chip::app::DataModel::Nullable<uint32_t> & value);
typedef void (*Int32sAttributeCallback)(void * context, int32_t value);
typedef void (*NullableInt32sAttributeCallback)(void * context, const chip::app::DataModel::Nullable<int32_t> & value);
typedef void (*Int64uAttributeCallback)(void * context, uint64_t value);
typedef void (*NullableInt64uAttributeCallback)(void * context, const chip::app::DataModel::Nullable<uint64_t> & value);
typedef void (*Int64sAttributeCallback)(void * context, int64_t value);
typedef void (*NullableInt64sAttributeCallback)(void * context, const chip::app::DataModel::Nullable<int64_t> & value);
typedef void (*FloatAttributeCallback)(void * context, float value);
typedef void (*NullableFloatAttributeCallback)(void * context, const chip::app::DataModel::Nullable<float> & value);
typedef void (*DoubleAttributeCallback)(void * context, double value);
typedef void (*NullableDoubleAttributeCallback)(void * context, const chip::app::DataModel::Nullable<double> & value);
typedef void (*OctetStringAttributeCallback)(void * context, const chip::ByteSpan value);
typedef void (*NullableOctetStringAttributeCallback)(void * context, const chip::app::DataModel::Nullable<chip::ByteSpan> & value);
typedef void (*CharStringAttributeCallback)(void * context, const chip::CharSpan value);
typedef void (*NullableCharStringAttributeCallback)(void * context, const chip::app::DataModel::Nullable<chip::CharSpan> & value);
typedef void (*AttributeResponseFilter)(chip::TLV::TLVReader * data, chip::Callback::Cancelable * onSuccess,
                                        chip::Callback::Cancelable * onFailure);

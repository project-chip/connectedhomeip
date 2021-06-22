/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/****************************************************************************
 * @file window-covering-server.h
 * @brief Routines and Helpers for the Window Covering Server cluster
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include "window-covering-common.h"

EmberAfStatus wcWriteAttribute(chip::EndpointId ep, chip::AttributeId attributeID, uint8_t * dataPtr, EmberAfAttributeType dataType);
EmberAfStatus wcReadAttribute(chip::EndpointId ep, chip::AttributeId attributeID, uint8_t * dataPtr, uint16_t readLength);

EmberAfStatus wcSetTargetPositionLift(chip::EndpointId ep, posPercent100ths_t liftPercent100ths);
EmberAfStatus wcGetTargetPositionLift(chip::EndpointId ep, posPercent100ths_t * p_liftPercent100ths);

EmberAfStatus wcSetTargetPositionTilt(chip::EndpointId ep, posPercent100ths_t tiltPercent100ths);
EmberAfStatus wcGetTargetPositionTilt(chip::EndpointId ep, posPercent100ths_t * p_tiltPercent100ths);

EmberAfStatus wcSetCurrentPositionLift(chip::EndpointId ep, posPercent100ths_t liftPercent100ths, uint16_t liftValue);
EmberAfStatus wcGetCurrentPositionLift(chip::EndpointId ep, posPercent100ths_t * p_liftPercent100ths);

EmberAfStatus wcSetCurrentPositionTilt(chip::EndpointId ep, posPercent100ths_t tiltPercent100ths, uint16_t tiltValue);
EmberAfStatus wcGetCurrentPositionTilt(chip::EndpointId ep, posPercent100ths_t * p_tiltPercent100ths);


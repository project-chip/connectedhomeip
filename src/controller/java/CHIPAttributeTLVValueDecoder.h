/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/ConcreteAttributePath.h>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>

namespace chip {

jobject DecodeAttributeValue(const chip::app::ConcreteAttributePath & aPath, chip::TLV::TLVReader & aReader, CHIP_ERROR * aError);

} // namespace chip

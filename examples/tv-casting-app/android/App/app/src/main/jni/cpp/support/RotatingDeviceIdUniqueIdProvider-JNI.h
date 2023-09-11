/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "core/Types.h"

#include <jni.h>

namespace matter {
namespace casting {
namespace support {

class RotatingDeviceIdUniqueIdProviderJNI : public MutableByteSpanDataProvider
{
public:
    CHIP_ERROR Initialize(jobject provider);
    chip::MutableByteSpan * Get() override;

private:
    CHIP_ERROR GetJavaByteByMethod(jmethodID method, chip::MutableByteSpan & out_buffer);
    jobject mJNIProviderObject = nullptr;
    jmethodID mGetMethod       = nullptr;

    chip::MutableByteSpan mRotatingDeviceIdUniqueIdSpan;
    uint8_t mRotatingDeviceIdUniqueId[chip::DeviceLayer::ConfigurationManager::kRotatingDeviceIDUniqueIDLength];
};

}; // namespace support
}; // namespace casting
}; // namespace matter

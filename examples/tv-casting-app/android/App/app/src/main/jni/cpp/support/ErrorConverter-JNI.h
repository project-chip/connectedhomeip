/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <lib/core/CHIPError.h>

#include <jni.h>

namespace matter {
namespace casting {
namespace support {

jobject createJMatterError(CHIP_ERROR inErr);

}; // namespace support
}; // namespace casting
}; // namespace matter

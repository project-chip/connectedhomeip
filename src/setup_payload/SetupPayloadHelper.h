/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      The implementation of the Setup Payload Helper. To be used by the chip command line tool.
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <string>

#include <string>

namespace chip {
CHIP_ERROR generateQRCodeFromFilePath(std::string filePath, std::string & outCode);
CHIP_ERROR generateManualCodeFromFilePath(std::string filePath, std::string & outCode);
} // namespace chip

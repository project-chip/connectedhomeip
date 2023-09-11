/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "TraceDecoderOptions.h"

#include <lib/support/CHIPArgParser.hpp>

struct TraceDecoderArgumentParser
{
    chip::trace::TraceDecoderOptions options;
    const char * logFile = nullptr;

    static TraceDecoderArgumentParser & GetInstance();
    CHIP_ERROR ParseArguments(int argc, char * const argv[]);
};

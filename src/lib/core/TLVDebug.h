/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2015-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file defines types and interfaces for debugging and
 *      logging CHIP TLV.
 *
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>

namespace chip {

namespace TLV {

/**
 *   @namespace chip::TLV::Debug
 *
 *   @brief
 *     This namespace includes types and interfaces for debugging and
 *     logging CHIP TLV.
 *
 */
namespace Debug {

typedef void (*DumpWriter)(const char * aFormat, ...);

struct DumpContext
{
    DumpWriter mWriter;
    void * mContext;
};

extern const char * DecodeType(TLVType aType);

extern const char * DecodeTagControl(TLVTagControl aTagControl);

extern CHIP_ERROR DumpIterator(DumpWriter aWriter, const TLVReader & aReader);

extern CHIP_ERROR DumpHandler(const TLVReader & aReader, size_t aDepth, void * aContext);

extern CHIP_ERROR Dump(const TLVReader & aReader, DumpWriter aWriter);

} // namespace Debug

} // namespace TLV

} // namespace chip

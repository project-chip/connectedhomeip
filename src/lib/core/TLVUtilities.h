/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2015-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file specifies types and utility interfaces for managing and
 *      working with CHIP TLV.
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
 *   @namespace chip::TLV::Utilities
 *
 *   @brief
 *     This namespace includes types and utility interfaces for managing and
 *     working with CHIP TLV.
 *
 */
namespace Utilities {

typedef CHIP_ERROR (*IterateHandler)(const TLVReader & aReader, size_t aDepth, void * aContext);

extern CHIP_ERROR Iterate(const TLVReader & aReader, IterateHandler aHandler, void * aContext);
extern CHIP_ERROR Iterate(const TLVReader & aReader, IterateHandler aHandler, void * aContext, bool aRecurse);

extern CHIP_ERROR Count(const TLVReader & aReader, size_t & aCount);
extern CHIP_ERROR Count(const TLVReader & aReader, size_t & aCount, bool aRecurse);

extern CHIP_ERROR Find(const TLVReader & aReader, const Tag & aTag, TLVReader & aResult);
extern CHIP_ERROR Find(const TLVReader & aReader, const Tag & aTag, TLVReader & aResult, bool aRecurse);

extern CHIP_ERROR Find(const TLVReader & aReader, IterateHandler aHandler, void * aContext, TLVReader & aResult);
extern CHIP_ERROR Find(const TLVReader & aReader, IterateHandler aHandler, void * aContext, TLVReader & aResult, bool aRecurse);
} // namespace Utilities

} // namespace TLV

} // namespace chip

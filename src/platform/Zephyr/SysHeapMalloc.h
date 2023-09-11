/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cstddef>

#include <lib/core/CHIPError.h>

namespace chip {
namespace DeviceLayer {
namespace Malloc {

struct Stats
{
    size_t free;
    size_t used;
    size_t maxUsed;
};

void * Malloc(size_t size);
void * Calloc(size_t num, size_t size);
void * Realloc(void * mem, size_t size);
void Free(void * mem);
CHIP_ERROR GetStats(Stats & stats);
void ResetMaxStats();

} // namespace Malloc
} // namespace DeviceLayer
} // namespace chip

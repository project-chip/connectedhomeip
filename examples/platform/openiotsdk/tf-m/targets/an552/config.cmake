#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set(PLATFORM_SLIH_IRQ_TEST_SUPPORT    ON    CACHE BOOL    "Platform supports SLIH IRQ tests")
set(PLATFORM_FLIH_IRQ_TEST_SUPPORT    ON    CACHE BOOL    "Platform supports FLIH IRQ tests")

# Make FLIH IRQ test as the default IRQ test on Corstone-310
set(TEST_NS_SLIH_IRQ                  OFF   CACHE BOOL    "Whether to build NS regression Second-Level Interrupt Handling tests")

if(BL2)
    set(BL2_TRAILER_SIZE 0x800 CACHE STRING "Trailer size")
else()
    #No header if no bootloader, but keep IMAGE_CODE_SIZE the same
    set(BL2_TRAILER_SIZE 0xC00 CACHE STRING "Trailer size")
endif()

/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "rand32.h"
#include <crypto/CHIPCryptoPAL.h>

/**
 * Fill the buffer given as an arg with random values
 * Returns 0 if success, -1 otherwise
 *
 */
int sys_csrand_get(void *dst, size_t len)
{
    CHIP_ERROR error;
    error = chip::Crypto::DRBG_get_bytes(reinterpret_cast<uint8_t *>(dst), len);

    return ((error == CHIP_NO_ERROR) ? 0 : -1);
}
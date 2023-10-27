/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RAND32_H_
#define RAND32_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/**
 * Fill the buffer given as an arg with random values
 * Returns 0 if success, -1 otherwise
 *
 */
int sys_csrand_get(void * dst, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* RAND32_H_ */

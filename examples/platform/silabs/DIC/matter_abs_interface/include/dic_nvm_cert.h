
/**
 * @file
 * @brief Matter abstraction layer for Direct Internet Connectivity.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc.
 *www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon
 *Laboratories Inc. Your use of this software is
 *governed by the terms of Silicon Labs Master
 *Software License Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 *This software is distributed to you in Source Code
 *format and is governed by the sections of the MSLA
 *applicable to Source Code.
 *
 ******************************************************************************/

#ifndef _DIC_NVM_CERT_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "lib/core/CHIPError.h"
#include <platform/silabs/SilabsConfig.h>

#define DIC_CA_CERT_LENGTH 1212
#define DIC_DEV_CERT_LENGTH 1212
#define DIC_DEV_KEY_LENGTH 1212
#define DIC_HOSTNAME_LENGTH 55
#define DIC_CLIENTID_LENGTH 30


CHIP_ERROR DICGetCACertificate(char * buf, size_t buf_len, size_t *bufSize);

CHIP_ERROR DICGetDeviceCertificate(char * buf, size_t buf_len, size_t *bufSize);

CHIP_ERROR DICGetDevicePrivKey(char * buf, size_t buf_len, size_t *bufSize);

CHIP_ERROR DICGetHostname(char * buf, size_t buf_len, size_t *bufSize);

CHIP_ERROR DICGetClientId(char * buf, size_t buf_len, size_t *bufSize);

#endif //_DIC_NVM_CERT_H
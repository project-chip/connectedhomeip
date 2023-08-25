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
#include "stddef.h"
#include <cstring>
#include "dic_config.h"
#include "dic_nvm_cert.h"
#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>

#define USE_AWS 1
#define USE_MOSQUITTO 0

#if USE_AWS

#define DIC_SERVER_HOST	""

/*
 * Certificate set for TLS Connection to public endpoint
*/
char ca_certificate[] = "";

char device_certificate[] = "";

char device_key[] = "";

#elif USE_MOSQUITTO

#define DIC_CLIENT_ID "test_client"
#define DIC_SERVER_HOST	""

/*
 * Certificate set for TLS Connection to local broker ex: mosquitto
*/
char ca_certificate[] = "";

char device_certificate[] = "";

char device_key[] = "";

#endif

using namespace chip::DeviceLayer::Internal;

CHIP_ERROR DICGetCACertificate(char * buf, size_t buf_len, size_t *bufSize)
{
    CHIP_ERROR status = CHIP_NO_ERROR;
#if SL_DIC_NVM_EMBED_CERT
    status = SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_CACerts,reinterpret_cast<uint8_t *>(buf), buf_len, *bufSize);
#else
    *bufSize = sizeof(ca_certificate);
    if(buf == NULL && buf_len < *bufSize) {
        return CHIP_ERROR_INTERNAL;
    }
    strncpy(buf,ca_certificate,*bufSize);
#endif
    return status;
}

CHIP_ERROR DICGetDeviceCertificate(char * buf, size_t buf_len, size_t *bufSize) 
{
    CHIP_ERROR status = CHIP_NO_ERROR;
#if SL_DIC_NVM_EMBED_CERT
    status = SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_DeviceCerts,reinterpret_cast<uint8_t *>(buf), buf_len, *bufSize);
#else
    *bufSize = sizeof(device_certificate);
    if(buf == NULL && buf_len < *bufSize) {
        return CHIP_ERROR_INTERNAL;
    }
    strncpy(buf,device_certificate,*bufSize);
#endif
    return status;
}

CHIP_ERROR DICGetDevicePrivKey(char * buf, size_t buf_len, size_t *bufSize) 
{
    CHIP_ERROR status = CHIP_NO_ERROR;
#if SL_DIC_NVM_EMBED_CERT
    status = SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_DeviceKey,reinterpret_cast<uint8_t *>(buf), buf_len, *bufSize);
#else
    *bufSize = sizeof(device_key);
    if(buf == NULL && buf_len < *bufSize) {
        return CHIP_ERROR_INTERNAL;
    }
    strncpy(buf,device_key,*bufSize);
#endif
    return status;
}

CHIP_ERROR DICGetHostname(char * buf, size_t buf_len, size_t *bufSize) 
{
    CHIP_ERROR status = CHIP_NO_ERROR;
#if SL_DIC_NVM_EMBED_CERT
    status = SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_hostname,reinterpret_cast<uint8_t *>(buf), buf_len, *bufSize);
#else
    *bufSize = sizeof(DIC_SERVER_HOST);
    if(buf == NULL && buf_len < *bufSize) {
        return CHIP_ERROR_INTERNAL;
    }
    strncpy(buf,DIC_SERVER_HOST,*bufSize);
#endif
    return status;
}

CHIP_ERROR DICGetClientId(char * buf, size_t buf_len, size_t *bufSize) 
{
    CHIP_ERROR status = CHIP_NO_ERROR;
#if SL_DIC_NVM_EMBED_CERT
    status = SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_clientid,reinterpret_cast<uint8_t *>(buf), buf_len, *bufSize);
#else
    *bufSize = sizeof(DIC_CLIENT_ID);
    if(buf == NULL && buf_len < *bufSize) {
        return CHIP_ERROR_INTERNAL;
    }
    strncpy(buf,DIC_CLIENT_ID,*bufSize);
#endif
    return status;
}
/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      This file implements utility functions for OpenSSL, base-64
 *      encoding and decoding, date and time parsing, EUI64 parsing,
 *      OID translation, and file reading.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <ctype.h>
#include <getopt.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "chip-cert.h"
#include <core/CHIPTLV.h>
#include <protocols/Protocols.h>
#include <support/Base64.h>

using namespace chip;
using namespace chip::Credentials;
using namespace chip::ASN1;

int gNIDChipNodeId;
int gNIDChipFirmwareSigningId;
int gNIDChipICAId;
int gNIDChipRootId;
int gNIDChipFabricId;
int gNIDChipAuthTag1;
int gNIDChipAuthTag2;

bool InitOpenSSL()
{
    bool res = true;

    OPENSSL_malloc_init();

    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();

    gNIDChipNodeId = OBJ_create("1.3.6.1.4.1.37244.1.1", "ChipNodeId", "ChipNodeId");
    if (gNIDChipNodeId == 0)
    {
        ReportOpenSSLErrorAndExit("OBJ_create", res = false);
    }

    gNIDChipFirmwareSigningId = OBJ_create("1.3.6.1.4.1.37244.1.2", "ChipFirmwareSigningId", "ChipFirmwareSigningId");
    if (gNIDChipFirmwareSigningId == 0)
    {
        ReportOpenSSLErrorAndExit("OBJ_create", res = false);
    }

    gNIDChipICAId = OBJ_create("1.3.6.1.4.1.37244.1.3", "ChipICAId", "ChipICAId");
    if (gNIDChipICAId == 0)
    {
        ReportOpenSSLErrorAndExit("OBJ_create", res = false);
    }

    gNIDChipRootId = OBJ_create("1.3.6.1.4.1.37244.1.4", "ChipRootId", "ChipRootId");
    if (gNIDChipICAId == 0)
    {
        ReportOpenSSLErrorAndExit("OBJ_create", res = false);
    }

    gNIDChipFabricId = OBJ_create("1.3.6.1.4.1.37244.1.5", "ChipFabricId", "ChipFabricId");
    if (gNIDChipFabricId == 0)
    {
        ReportOpenSSLErrorAndExit("OBJ_create", res = false);
    }

    gNIDChipAuthTag1 = OBJ_create("1.3.6.1.4.1.37244.1.6", "ChipAuthTag1", "ChipAuthTag1");
    if (gNIDChipAuthTag1 == 0)
    {
        ReportOpenSSLErrorAndExit("OBJ_create", res = false);
    }

    gNIDChipAuthTag2 = OBJ_create("1.3.6.1.4.1.37244.1.7", "ChipAuthTag2", "ChipAuthTag2");
    if (gNIDChipAuthTag2 == 0)
    {
        ReportOpenSSLErrorAndExit("OBJ_create", res = false);
    }

    ASN1_STRING_TABLE_add(gNIDChipNodeId, 16, 16, B_ASN1_UTF8STRING, 0);
    ASN1_STRING_TABLE_add(gNIDChipFirmwareSigningId, 16, 16, B_ASN1_UTF8STRING, 0);
    ASN1_STRING_TABLE_add(gNIDChipICAId, 16, 16, B_ASN1_UTF8STRING, 0);
    ASN1_STRING_TABLE_add(gNIDChipRootId, 16, 16, B_ASN1_UTF8STRING, 0);
    ASN1_STRING_TABLE_add(gNIDChipFabricId, 16, 16, B_ASN1_UTF8STRING, 0);
    ASN1_STRING_TABLE_add(gNIDChipAuthTag1, 8, 8, B_ASN1_UTF8STRING, 0);
    ASN1_STRING_TABLE_add(gNIDChipAuthTag2, 8, 8, B_ASN1_UTF8STRING, 0);

exit:
    return res;
}

char * Base64Encode(const uint8_t * inData, uint32_t inDataLen)
{
    uint32_t outDataLen;
    return Uint8::to_char(Base64Encode(inData, inDataLen, nullptr, 0, outDataLen));
}

uint8_t * Base64Encode(const uint8_t * inData, uint32_t inDataLen, uint8_t * outBuf, uint32_t outBufSize, uint32_t & outDataLen)
{
    uint8_t * res = nullptr;

    outDataLen = BASE64_ENCODED_LEN(inDataLen);

    if (outBuf != nullptr && (outDataLen + 1) > outBufSize)
    {
        fprintf(stderr, "Buffer overflow\n");
        ExitNow(res = nullptr);
    }

    res = reinterpret_cast<uint8_t *>(malloc(outDataLen + 1));
    if (res == nullptr)
    {
        fprintf(stderr, "Memory allocation error\n");
        ExitNow(res = nullptr);
    }
    memset(res, 0xAB, outDataLen + 1);

    outDataLen      = chip::Base64Encode32(inData, inDataLen, Uint8::to_char(res));
    res[outDataLen] = 0;

    if (outBuf != nullptr)
    {
        memcpy(outBuf, res, outDataLen + 1);
        free(res);
        res = outBuf;
    }

exit:
    return res;
}

uint8_t * Base64Decode(const uint8_t * inData, uint32_t inDataLen, uint8_t * outBuf, uint32_t outBufSize, uint32_t & outDataLen)
{
    uint8_t * res = nullptr;

    outDataLen = BASE64_MAX_DECODED_LEN(inDataLen);

    if (outBuf != nullptr)
    {
        if (outDataLen > outBufSize)
        {
            fprintf(stderr, "Buffer overflow\n");
            ExitNow();
        }
        res = outBuf;
    }
    else
    {
        res = reinterpret_cast<uint8_t *>(malloc(outDataLen));
        if (res == nullptr)
        {
            fprintf(stderr, "Memory allocation error\n");
            ExitNow();
        }
    }

    outDataLen = chip::Base64Decode32(Uint8::to_const_char(inData), inDataLen, res);
    if (outDataLen == UINT32_MAX)
    {
        fprintf(stderr, "Base-64 decode error\n");
        if (res != outBuf)
        {
            free(res);
        }
        ExitNow(res = nullptr);
    }

exit:
    return res;
}

bool IsBase64String(const char * str, uint32_t strLen)
{
    for (; strLen > 0; strLen--, str++)
    {
        if (!isalnum(*str) && *str != '+' && *str != '/' && *str != '=' && !isspace(*str))
        {
            return false;
        }
    }
    return true;
}

bool ContainsPEMMarker(const char * marker, const uint8_t * data, uint32_t dataLen)
{
    size_t markerLen = strlen(marker);

    if (dataLen > markerLen)
    {
        for (uint32_t i = 0; i <= dataLen - markerLen; i++)
        {
            if (strncmp(reinterpret_cast<char *>(const_cast<uint8_t *>(data + i)), marker, markerLen) == 0)
            {
                return true;
            }
        }
    }
    return false;
}

bool ParseChip64bitAttr(const char * str, uint64_t & val)
{
    char * parseEnd;

    errno = 0;
    val   = strtoull(str, &parseEnd, 16);
    return parseEnd > str && *parseEnd == 0 && (val != ULLONG_MAX || errno == 0);
}

bool ParseDateTime(const char * str, struct tm & date)
{
    const char * p;

    memset(&date, 0, sizeof(date));

    if ((p = strptime(str, "%Y-%m-%d %H:%M:%S", &date)) == nullptr && (p = strptime(str, "%Y/%m/%d %H:%M:%S", &date)) == nullptr &&
        (p = strptime(str, "%Y%m%d%H%M%SZ", &date)) == nullptr && (p = strptime(str, "%Y-%m-%d", &date)) == nullptr &&
        (p = strptime(str, "%Y/%m/%d", &date)) == nullptr && (p = strptime(str, "%Y%m%d", &date)) == nullptr)
    {
        return false;
    }

    if (*p != 0)
    {
        return false;
    }

    return true;
}

bool OpenFileToWrite(const char * fileName, FILE *& file)
{
    bool res = true;

    if (fileName != nullptr && strcmp(fileName, "-") != 0)
    {
        file = fopen(fileName, "w+");
        if (file == nullptr)
        {
            fprintf(stderr, "Unable to create %s\n%s\n", fileName, strerror(ferror(file) ? errno : ENOSPC));
            ExitNow(res = false);
        }
    }
    else
    {
        file = stdout;
    }

exit:
    return res;
}

void CloseFile(const char * fileName, FILE *& file)
{
    if (file != nullptr)
    {
        fclose(file);
        file = nullptr;
    }
}

bool ReadFileIntoMem(const char * fileName, uint8_t *& data, uint32_t & dataLen)
{
    bool res    = true;
    FILE * file = nullptr;
    long int fileLen;
    size_t readRes;

    data    = nullptr;
    dataLen = 0;

    file = fopen(fileName, "r");
    if (file == nullptr)
    {
        fprintf(stderr, "Unable to open %s: %s\n", fileName, strerror(errno));
        ExitNow(res = false);
    }

    fseek(file, 0, SEEK_END);
    fileLen = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (fileLen < 0 || ferror(file))
    {
        fprintf(stderr, "Error reading %s: %s\n", fileName, strerror(errno));
        ExitNow(res = false);
    }

    dataLen = static_cast<uint32_t>(fileLen);
    data    = reinterpret_cast<uint8_t *>(malloc((size_t) dataLen));
    if (data == nullptr)
    {
        fprintf(stderr, "Error reading %s: out of memory\n", fileName);
        ExitNow(res = false);
    }

    readRes = fread(data, 1, static_cast<size_t>(dataLen), file);
    if (readRes < static_cast<size_t>(dataLen) || ferror(file))
    {
        fprintf(stderr, "Error reading %s: %s\n", fileName, strerror(errno));
        ExitNow(res = false);
    }

exit:
    if (file != nullptr)
    {
        fclose(file);
    }
    if (!res && data != nullptr)
    {
        free(data);
        data = nullptr;
    }

    return res;
}

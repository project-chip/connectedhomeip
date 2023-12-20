/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "FactoryDataParser.h"

#include <zcbor_common.h>
#include <zcbor_decode.h>

#include <ctype.h>
#include <string.h>

#define MAX_FACTORY_DATA_NESTING_LEVEL 2

static inline bool uint16_decode(zcbor_state_t * states, uint16_t * value)
{
    uint32_t u32;

    if (zcbor_uint32_decode(states, &u32))
    {
        *value = (uint16_t) u32;
        return true;
    }

    return false;
}

static inline bool uint8_decode(zcbor_state_t * states, uint8_t * value)
{
    uint32_t u32;

    if (zcbor_uint32_decode(states, &u32))
    {
        *value = (uint8_t) u32;
        return true;
    }

    return false;
}

static bool DecodeEntry(zcbor_state_t * states, void * buffer, size_t bufferSize, size_t * outlen)
{
    struct zcbor_string tempString;
    int32_t tempInt = 0;

    // Try to decode entry as string
    bool res = zcbor_tstr_decode(states, &tempString);
    if (res)
    {
        if (bufferSize < tempString.len)
        {
            return false;
        }
        memcpy(buffer, tempString.value, tempString.len);
        *outlen = tempString.len;
        return res;
    }

    // Try to decode entry as int32
    res = zcbor_int32_decode(states, &tempInt);
    if (res)
    {
        if (bufferSize < sizeof(tempInt))
        {
            return false;
        }
        memcpy(buffer, &tempInt, sizeof(tempInt));
        *outlen = sizeof(tempInt);
        return res;
    }

    return res;
}

bool FindUserDataEntry(struct FactoryData * factoryData, const char * entry, void * buffer, size_t bufferSize, size_t * outlen)
{
    if ((!factoryData) || (!factoryData->user.data) || (!buffer) || (!outlen))
    {
        return false;
    }

    ZCBOR_STATE_D(states, MAX_FACTORY_DATA_NESTING_LEVEL - 1, factoryData->user.data, factoryData->user.len, 1, 0);

    bool res      = zcbor_map_start_decode(states);
    bool keyFound = false;
    struct zcbor_string currentString;

    while (res)
    {
        res = zcbor_tstr_decode(states, &currentString);

        if (!res)
        {
            break;
        }

        if (strncmp(entry, (const char *) currentString.value, currentString.len) == 0)
        {
            res      = DecodeEntry(states, buffer, bufferSize, outlen);
            keyFound = true;
            break;
        }
        else
        {
            res = res && zcbor_any_skip(states, NULL);
        }
    }

    return res && keyFound && zcbor_list_map_end_force_decode(states);
}

bool ParseFactoryData(uint8_t * buffer, uint16_t bufferSize, struct FactoryData * factoryData)
{
    if (!buffer || !factoryData || bufferSize == 0)
    {
        return false;
    }

    memset(factoryData, 0, sizeof(*factoryData));
    ZCBOR_STATE_D(states, MAX_FACTORY_DATA_NESTING_LEVEL, buffer, bufferSize, 1, 0);

    bool res = zcbor_map_start_decode(states);
    struct zcbor_string currentString;

    while (res)
    {
        res = zcbor_tstr_decode(states, &currentString);

        if (!res)
        {
            res = true;
            break;
        }

        if (strncmp("version", (const char *) currentString.value, currentString.len) == 0)
        {
            res = res && uint16_decode(states, &factoryData->version);
        }
        else if (strncmp("hw_ver", (const char *) currentString.value, currentString.len) == 0)
        {
            res                       = res && uint16_decode(states, &factoryData->hw_ver);
            factoryData->hwVerPresent = res;
        }
        else if (strncmp("spake2_it", (const char *) currentString.value, currentString.len) == 0)
        {
            res = res && zcbor_uint32_decode(states, &factoryData->spake2_it);
        }
        else if (strncmp("vendor_id", (const char *) currentString.value, currentString.len) == 0)
        {
            res                          = res && uint16_decode(states, &factoryData->vendor_id);
            factoryData->vendorIdPresent = res;
        }
        else if (strncmp("product_id", (const char *) currentString.value, currentString.len) == 0)
        {
            res                           = res && uint16_decode(states, &factoryData->product_id);
            factoryData->productIdPresent = res;
        }
        else if (strncmp("discriminator", (const char *) currentString.value, currentString.len) == 0)
        {
            res                               = res && uint16_decode(states, &factoryData->discriminator);
            factoryData->discriminatorPresent = res;
        }
        else if (strncmp("passcode", (const char *) currentString.value, currentString.len) == 0)
        {
            res = res && zcbor_uint32_decode(states, &factoryData->passcode);
        }
        else if (strncmp("sn", (const char *) currentString.value, currentString.len) == 0)
        {
            res = res && zcbor_bstr_decode(states, (struct zcbor_string *) &factoryData->sn);
        }
        else if (strncmp("date", (const char *) currentString.value, currentString.len) == 0)
        {
            // Date format is YYYY-MM-DD, so format needs to be validated and string parse to integer parts.
            struct zcbor_string date;
            res = res && zcbor_bstr_decode(states, &date);
            if (date.len == 10 && isdigit(date.value[0]) && isdigit(date.value[1]) && isdigit(date.value[2]) &&
                isdigit(date.value[3]) && date.value[4] == '-' && isdigit(date.value[5]) && isdigit(date.value[6]) &&
                date.value[7] == '-' && isdigit(date.value[8]) && isdigit(date.value[9]))
            {
                factoryData->date_year =
                    (uint16_t) (1000 * (uint16_t) (date.value[0] - '0') + 100 * (uint16_t) (date.value[1] - '0') +
                                10 * (uint16_t) (date.value[2] - '0') + (uint16_t) (date.value[3] - '0'));
                factoryData->date_month = (uint8_t) (10 * (uint16_t) (date.value[5] - '0') + (uint16_t) (date.value[6] - '0'));
                factoryData->date_day   = (uint8_t) (10 * (uint16_t) (date.value[8] - '0') + (uint16_t) (date.value[9] - '0'));
            }
            else
            {
                res = false;
            }
        }
        else if (strncmp("hw_ver_str", (const char *) currentString.value, currentString.len) == 0)
        {
            res = res && zcbor_bstr_decode(states, (struct zcbor_string *) &factoryData->hw_ver_str);
        }
        else if (strncmp("rd_uid", (const char *) currentString.value, currentString.len) == 0)
        {
            res = res && zcbor_bstr_decode(states, (struct zcbor_string *) &factoryData->rd_uid);
        }
        else if (strncmp("dac_cert", (const char *) currentString.value, currentString.len) == 0)
        {
            res = res && zcbor_bstr_decode(states, (struct zcbor_string *) &factoryData->dac_cert);
        }
        else if (strncmp("dac_key", (const char *) currentString.value, currentString.len) == 0)
        {
            res                              = res && zcbor_bstr_decode(states, (struct zcbor_string *) &factoryData->dac_priv_key);
            factoryData->dacPrivateKeyOffset = (size_t) ((uint8_t *) factoryData->dac_priv_key.data - buffer);
        }
        else if (strncmp("pai_cert", (const char *) currentString.value, currentString.len) == 0)
        {
            res = res && zcbor_bstr_decode(states, (struct zcbor_string *) &factoryData->pai_cert);
        }
        else if (strncmp("spake2_salt", (const char *) currentString.value, currentString.len) == 0)
        {
            res = res && zcbor_bstr_decode(states, (struct zcbor_string *) &factoryData->spake2_salt);
        }
        else if (strncmp("spake2_verifier", (const char *) currentString.value, currentString.len) == 0)
        {
            res = res && zcbor_bstr_decode(states, (struct zcbor_string *) &factoryData->spake2_verifier);
        }
        else if (strncmp("vendor_name", (const char *) currentString.value, currentString.len) == 0)
        {
            res = res && zcbor_bstr_decode(states, (struct zcbor_string *) &factoryData->vendor_name);
        }
        else if (strncmp("product_name", (const char *) currentString.value, currentString.len) == 0)
        {
            res = res && zcbor_bstr_decode(states, (struct zcbor_string *) &factoryData->product_name);
        }
        else if (strncmp("part_number", (const char *) currentString.value, currentString.len) == 0)
        {
            res = res && zcbor_bstr_decode(states, (struct zcbor_string *) &factoryData->part_number);
        }
        else if (strncmp("product_url", (const char *) currentString.value, currentString.len) == 0)
        {
            res = res && zcbor_bstr_decode(states, (struct zcbor_string *) &factoryData->product_url);
        }
        else if (strncmp("product_label", (const char *) currentString.value, currentString.len) == 0)
        {
            res = res && zcbor_bstr_decode(states, (struct zcbor_string *) &factoryData->product_label);
        }
        else if (strncmp("enable_key", (const char *) currentString.value, currentString.len) == 0)
        {
            res = res && zcbor_bstr_decode(states, (struct zcbor_string *) &factoryData->enable_key);
        }
        else if (strncmp("product_finish", (const char *) currentString.value, currentString.len) == 0)
        {
            res                               = res && uint8_decode(states, &factoryData->product_finish);
            factoryData->productFinishPresent = res;
        }
        else if (strncmp("primary_color", (const char *) currentString.value, currentString.len) == 0)
        {
            res                              = res && uint8_decode(states, &factoryData->primary_color);
            factoryData->primaryColorPresent = res;
        }
        else if (strncmp("user", (const char *) currentString.value, currentString.len) == 0)
        {
            factoryData->user.data = (void *) states->payload;
            res                    = res && zcbor_any_skip(states, NULL);
            factoryData->user.len  = (size_t) ((void *) states->payload - factoryData->user.data);
        }
        else
        {
            res = res && zcbor_any_skip(states, NULL);
        }
    }

    return res && zcbor_list_map_end_force_decode(states);
}

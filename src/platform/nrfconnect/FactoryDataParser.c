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

#include <logging/log.h>
#include <zcbor_decode.h>

#include <ctype.h>

#define MAX_FACTORY_DATA_ELEMENTS 19
#define MIN_FACTORY_DATA_ELEMENTS 15

LOG_MODULE_DECLARE(app, CONFIG_MATTER_LOG_LEVEL);

bool GetFactoryData(uint8_t * buffer, uint16_t bufferSize, struct FactoryData * factoryData)
{
    uint32_t elementsCount = 0;
    bool keyNotFound;

    ZCBOR_STATE_D(states, MAX_FACTORY_DATA_ELEMENTS, buffer, bufferSize, 1);

    bool res = zcbor_map_start_decode(states);
    struct zcbor_string currentString;

    while (res)
    {
        keyNotFound = false;
        res         = res && zcbor_tstr_decode(states, &currentString);

        if (!res)
        {
            break;
        }

        if (strncmp("hw_ver", (const char *) currentString.value, currentString.len) == 0)
        {
            uint32_t hw_ver;
            res                 = res && zcbor_uint32_decode(states, &hw_ver);
            factoryData->hw_ver = hw_ver;
        }
        else if (strncmp("spake2_it", (const char *) currentString.value, currentString.len) == 0)
        {
            res = res && zcbor_uint32_decode(states, &factoryData->spake2_it);
        }
        else if (strncmp("vendor_id", (const char *) currentString.value, currentString.len) == 0)
        {
            uint32_t vendor_id;
            res                    = res && zcbor_uint32_decode(states, &vendor_id);
            factoryData->vendor_id = vendor_id;
        }
        else if (strncmp("product_id", (const char *) currentString.value, currentString.len) == 0)
        {
            uint32_t product_id;
            res                     = res && zcbor_uint32_decode(states, &product_id);
            factoryData->product_id = product_id;
        }
        else if (strncmp("discriminator", (const char *) currentString.value, currentString.len) == 0)
        {
            uint32_t discriminator;
            res                        = res && zcbor_uint32_decode(states, &discriminator);
            factoryData->discriminator = discriminator;
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
                    1000 * (date.value[0] - '0') + 100 * (date.value[1] - '0') + 10 * (date.value[2] - '0') + date.value[3] - '0';
                factoryData->date_month = 10 * (date.value[5] - '0') + date.value[6] - '0';
                factoryData->date_day   = 10 * (date.value[8] - '0') + date.value[9] - '0';
            }
            else
            {
                LOG_ERR("Parsing error - wrong date format");
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
            res = res && zcbor_bstr_decode(states, (struct zcbor_string *) &factoryData->dac_priv_key);
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
        else if (strncmp("user", (const char *) currentString.value, currentString.len) == 0)
        {
            res = res && zcbor_bstr_decode(states, (struct zcbor_string *) &factoryData->user);
        }
        else
        {
            keyNotFound = true;
            res         = res && zcbor_any_skip(states, NULL);
        }

        if (!keyNotFound && res)
        {
            elementsCount++;
        }
    }

    res = zcbor_list_map_end_force_decode(states);

    return res && elementsCount >= MIN_FACTORY_DATA_ELEMENTS;
}

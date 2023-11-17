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

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct FactoryDataString
{
    void * data;
    size_t len;
};

struct FactoryData
{
    uint16_t version;
    struct FactoryDataString sn;
    uint16_t date_year;
    uint8_t date_month;
    uint8_t date_day;
    uint16_t vendor_id;
    uint16_t product_id;
    struct FactoryDataString vendor_name;
    struct FactoryDataString product_name;
    struct FactoryDataString part_number;
    struct FactoryDataString product_url;
    struct FactoryDataString product_label;
    uint16_t hw_ver;
    struct FactoryDataString hw_ver_str;
    struct FactoryDataString rd_uid;
    struct FactoryDataString dac_cert;
    struct FactoryDataString dac_priv_key;
    struct FactoryDataString pai_cert;
    uint32_t spake2_it;
    struct FactoryDataString spake2_salt;
    struct FactoryDataString spake2_verifier;
    uint16_t discriminator;
    uint32_t passcode;
    struct FactoryDataString enable_key;
    struct FactoryDataString user;
    struct FactoryDataString certificate_declaration;

    bool vendorIdPresent;
    bool productIdPresent;
    bool hwVerPresent;
    bool discriminatorPresent;
};

/**
 * @brief Parses raw factory data into the factory data structure.
 *
 * @param[in] buffer Buffer containing raw factory data.
 * @param[in] bufferSize Size of factory data.
 * @param[out] factoryData address of object to be filled with parsed factory data.
 *
 * @returns true on success, false otherwise.
 */
bool ParseFactoryData(uint8_t * buffer, uint16_t bufferSize, struct FactoryData * factoryData);

#ifdef __cplusplus
}
#endif

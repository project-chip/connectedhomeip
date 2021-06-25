/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include "EndpointConfigurationStorage.h"
#include <fstream>
#include <support/CodeUtils.h>

constexpr const char kEndpointConfigurationPath[] = "/tmp/chip_tv_config.ini";

CHIP_ERROR EndpointConfigurationStorage::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::ifstream ifs;
    ifs.open(kEndpointConfigurationPath, std::ifstream::in);
    if (!ifs.good())
    {
        ifs.open(kEndpointConfigurationPath, std::ifstream::in);
    }
    VerifyOrExit(ifs.is_open(), err = CHIP_ERROR_OPEN_FAILED);

    endpointConfig.parse(ifs);
    ifs.close();

exit:
    return err;
}

CHIP_ERROR EndpointConfigurationStorage::get(std::string sectionName, const char * key, char * value, uint16_t & size)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    std::string iniValue;
    size_t iniValueLength = 0;

    auto section = endpointConfig.sections[sectionName];
    auto it      = section.find(key);
    VerifyOrExit(it != section.end(), err = CHIP_ERROR_KEY_NOT_FOUND);
    VerifyOrExit(inipp::extract(section[key], iniValue), err = CHIP_ERROR_INVALID_ARGUMENT);

    iniValueLength = iniValue.size();
    VerifyOrExit(iniValueLength <= static_cast<size_t>(size) - 1, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    iniValueLength        = iniValue.copy(value, iniValueLength);
    value[iniValueLength] = '\0';

exit:
    return err;
}

CHIP_ERROR EndpointConfigurationStorage::get(std::string sectionName, const char * key, uint16_t & value)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    auto section = endpointConfig.sections[sectionName];
    auto it      = section.find(key);
    VerifyOrExit(it != section.end(), err = CHIP_ERROR_KEY_NOT_FOUND);
    VerifyOrExit(inipp::extract(section[key], value), err = CHIP_ERROR_INVALID_ARGUMENT);

exit:
    return err;
}

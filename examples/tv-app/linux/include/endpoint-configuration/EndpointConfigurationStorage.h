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

#pragma once

#include <inipp/inipp.h>
#include <lib/core/CHIPCore.h>
#include <string>

class EndpointConfigurationStorage
{
public:
    CHIP_ERROR Init();
    CHIP_ERROR get(std::string sectionName, const char * key, char * value, uint16_t & size);
    CHIP_ERROR get(std::string sectionName, const char * key, uint16_t & value);

    static EndpointConfigurationStorage & GetInstance()
    {
        static EndpointConfigurationStorage instance;
        return instance;
    }

private:
    inipp::Ini<char> endpointConfig;
};

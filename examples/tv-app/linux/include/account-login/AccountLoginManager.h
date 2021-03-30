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

#include "af-types.h"
#include <core/CHIPError.h>
#include <map>
#include <string>

using namespace std;

class AccountLoginManager
{
public:
    bool isUserLoggedIn(string requestTempAccountIdentifier, string requestSetupPin);
    string proxySetupPinRequest(string requestTempAccountIdentifier, chip::EndpointId endpoint);
    void setTempAccountIdentifierForPin(string requestTempAccountIdentifier, string requestSetupPin);

    static AccountLoginManager & GetInstance()
    {
        static AccountLoginManager instance;
        return instance;
    }

private:
    map<string, string> accounts;
};
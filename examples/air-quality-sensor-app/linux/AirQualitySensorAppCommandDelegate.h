/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "NamedPipeCommands.h"

#include <json/json.h>
#include <platform/DiagnosticDataProvider.h>

class AirQualitySensorAppCommandHandler
{
public:
    static AirQualitySensorAppCommandHandler * FromJSON(const char * json);

    static void HandleCommand(intptr_t context);

    AirQualitySensorAppCommandHandler(Json::Value && jasonValue) : mJsonValue(std::move(jasonValue)) {}

private:
    Json::Value mJsonValue;

    /**
     * Should be called when the momentary switch starts to be pressed.
     */
    void OnVocChangeHandler(uint8_t newPosition);

    /**
     * Should be called when the momentary switch has been released.
     */
    void OnCo2ChangeHandler(uint8_t previousPosition);

};

class AirQualitySensorAppCommandDelegate : public NamedPipeCommandDelegate
{
public:
    void OnEventCommandReceived(const char * json) override;
};

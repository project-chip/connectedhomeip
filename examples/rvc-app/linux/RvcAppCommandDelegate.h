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
#include "rvc-device.h"
#include <json/json.h>
#include <platform/DiagnosticDataProvider.h>

#include <string>

class RvcAppCommandHandler
{
public:
    static RvcAppCommandHandler * FromJSON(const char * json);

    static void HandleCommand(intptr_t context);

    RvcAppCommandHandler(Json::Value && jasonValue) : mJsonValue(std::move(jasonValue)) {}

    void SetRvcDevice(chip::app::Clusters::RvcDevice * aRvcDevice);

private:
    Json::Value mJsonValue;
    chip::app::Clusters::RvcDevice * mRvcDevice;

    /**
     * Should be called to notify that the device has finished charging.
     */
    void OnChargedHandler();

    void OnChargingHandler();

    void OnDockedHandler();

    void OnChargerFoundHandler();

    void OnLowChargeHandler();

    void OnActivityCompleteHandler();

    void OnAreaCompleteHandler();

    void OnAddServiceAreaMap(Json::Value jsonValue);

    void OnAddServiceAreaArea(Json::Value jsonValue);

    void OnRemoveServiceAreaMap(uint32_t mapId);

    void OnRemoveServiceAreaArea(uint32_t areaId);

    void OnErrorEventHandler(const std::string & error);

    void OnClearErrorHandler();

    void OnResetHandler();
};

class RvcAppCommandDelegate : public NamedPipeCommandDelegate
{
private:
    chip::app::Clusters::RvcDevice * mRvcDevice;

public:
    void SetRvcDevice(chip::app::Clusters::RvcDevice * aRvcDevice);
    void OnEventCommandReceived(const char * json) override;
};

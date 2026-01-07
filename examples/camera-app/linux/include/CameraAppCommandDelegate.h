/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include "camera-device.h"
#include <json/json.h>
#include <platform/DiagnosticDataProvider.h>

#include <string>

class CameraAppCommandHandler
{
public:
    static CameraAppCommandHandler * FromJSON(const char * json);

    static void HandleCommand(intptr_t context);

    CameraAppCommandHandler(Json::Value && jasonValue) : mJsonValue(std::move(jasonValue)) {}

    void SetCameraDevice(Camera::CameraDevice * aCameraDevice);

private:
    Json::Value mJsonValue;
    Camera::CameraDevice * mCameraDevice = nullptr;

    void OnZoneTriggeredHandler(uint16_t zoneId);
    void OnSetHardPrivacyModeOnHandler(bool value);
};

class CameraAppCommandDelegate : public NamedPipeCommandDelegate
{
private:
    Camera::CameraDevice * mCameraDevice = nullptr;

public:
    void SetCameraDevice(Camera::CameraDevice * aCameraDevice);
    void OnEventCommandReceived(const char * json) override;
};

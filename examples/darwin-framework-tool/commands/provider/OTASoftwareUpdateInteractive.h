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
#include "../common/CHIPCommandBridge.h"

static constexpr uint16_t SW_VER_STR_MAX_LEN = 64;
static constexpr uint16_t OTA_URL_MAX_LEN    = 512;

typedef struct DeviceSoftwareVersionModel
{
    chip::VendorId vendorId;
    uint16_t productId;
    uint32_t softwareVersion;
    char softwareVersionString[SW_VER_STR_MAX_LEN];
    uint16_t cDVersionNumber;
    bool softwareVersionValid;
    uint32_t minApplicableSoftwareVersion;
    uint32_t maxApplicableSoftwareVersion;
    char otaURL[OTA_URL_MAX_LEN];
} DeviceSoftwareVersionModel;

class OTASoftwareUpdateBase : public CHIPCommandBridge {
public:
    OTASoftwareUpdateBase(const char * _Nonnull commandName)
        : CHIPCommandBridge(commandName) {}
    void SetCandidatesFromFilePath(char * _Nonnull filePath);
    bool SelectOTACandidate(const uint16_t requestorVendorID, 
    const uint16_t requestorProductID,
    const uint32_t requestorSoftwareVersion,
    DeviceSoftwareVersionModel & finalCandidate);

    /////////// CHIPCommandBridge Interface /////////
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(120); }
private:
    std::vector<DeviceSoftwareVersionModel> mCandidates;

};

class OTASoftwareUpdateSetFilePath : public OTASoftwareUpdateBase {
public:
    OTASoftwareUpdateSetFilePath()
        : OTASoftwareUpdateBase("candidate-file-path")
    {
         AddArgument("path", &mOTACandidatesFilePath);
    }

    /////////// CHIPCommandBridge Interface /////////
    CHIP_ERROR RunCommand() override;
private:
    char * _Nonnull mOTACandidatesFilePath;
};

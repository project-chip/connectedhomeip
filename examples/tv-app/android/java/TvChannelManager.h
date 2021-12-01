/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app-common/zap-generated/af-structs.h>
#include <app/AttributeAccessInterface.h>

#include <jni.h>
#include <lib/core/CHIPError.h>
#include <string>
#include <vector>

class TvChannelManager
{
public:
    void InitializeWithObjects(jobject managerObject);
    CHIP_ERROR getTvChannelList(chip::app::AttributeValueEncoder & aEncoder);
    CHIP_ERROR getTvChannelLineup(chip::app::AttributeValueEncoder & aEncoder);
    CHIP_ERROR getCurrentTvChannel(chip::app::AttributeValueEncoder & aEncoder);

    TvChannelInfo ChangeChannelByMatch(std::string name);
    bool changeChannelByNumber(uint16_t majorNumber, uint16_t minorNumber);
    bool skipChannnel(uint16_t count);

private:
    friend TvChannelManager & TvChannelMgr();

    static TvChannelManager sInstance;
    jobject mTvChannelManagerObject    = nullptr;
    jmethodID mGetChannelListMethod    = nullptr;
    jmethodID mGetLineupMethod         = nullptr;
    jmethodID mGetCurrentChannelMethod = nullptr;

    jmethodID mChangeChannelMethod         = nullptr;
    jmethodID mchangeChannelByNumberMethod = nullptr;
    jmethodID mskipChannelMethod           = nullptr;
};

inline class TvChannelManager & TvChannelMgr()
{
    return TvChannelManager::sInstance;
}

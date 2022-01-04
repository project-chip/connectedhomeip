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

#include <app/clusters/channel-server/channel-server.h>
#include <jni.h>

class ChannelManager : public chip::app::Clusters::Channel::Delegate
{
public:
    void InitializeWithObjects(jobject managerObject);

    std::list<chip::app::Clusters::Channel::Structs::ChannelInfo::Type> HandleGetChannelList() override;
    chip::app::Clusters::Channel::Structs::LineupInfo::Type HandleGetLineup() override;
    chip::app::Clusters::Channel::Structs::ChannelInfo::Type HandleGetCurrentChannel() override;

    chip::app::Clusters::Channel::Commands::ChangeChannelResponse::Type HandleChangeChannel(const chip::CharSpan & match) override;
    bool HandleChangeChannelByNumber(const uint16_t & majorNumber, const uint16_t & minorNumber) override;
    bool HandleSkipChannel(const uint16_t & count) override;

private:
    friend ChannelManager & ChannelMgr();

    static ChannelManager sInstance;
    jobject mChannelManagerObject      = nullptr;
    jmethodID mGetChannelListMethod    = nullptr;
    jmethodID mGetLineupMethod         = nullptr;
    jmethodID mGetCurrentChannelMethod = nullptr;

    jmethodID mChangeChannelMethod         = nullptr;
    jmethodID mChangeChannelByNumberMethod = nullptr;
    jmethodID mSkipChannelMethod           = nullptr;
};

inline class ChannelManager & ChannelMgr()
{
    return ChannelManager::sInstance;
}

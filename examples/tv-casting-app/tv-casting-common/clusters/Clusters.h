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

#include "core/Attribute.h"
#include "core/Command.h"
#include "core/Endpoint.h"

#include <app-common/zap-generated/cluster-objects.h>

/**
 * @brief This file contains classes representing all the Matter clusters supported by the Matter TV Casting library
 */
namespace matter {
namespace casting {
namespace clusters {
namespace application_basic {
class ApplicationBasicCluster : public core::BaseCluster
{
public:
    ApplicationBasicCluster(memory::Weak<core::Endpoint> endpoint) : core::BaseCluster(endpoint) {}

    void SetUp()
    {
        ChipLogProgress(AppServer, "Setting up ApplicationBasicCluster on EndpointId: %d", GetEndpoint().lock()->GetId());

        RegisterAttribute(
            chip::app::Clusters::ApplicationBasic::Attributes::VendorName::Id,
            new core::Attribute<chip::app::Clusters::ApplicationBasic::Attributes::VendorName::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::ApplicationBasic::Attributes::VendorID::Id,
            new core::Attribute<chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::ApplicationBasic::Attributes::ApplicationName::Id,
            new core::Attribute<chip::app::Clusters::ApplicationBasic::Attributes::ApplicationName::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::ApplicationBasic::Attributes::ProductID::Id,
            new core::Attribute<chip::app::Clusters::ApplicationBasic::Attributes::ProductID::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::ApplicationBasic::Attributes::Application::Id,
            new core::Attribute<chip::app::Clusters::ApplicationBasic::Attributes::Application::TypeInfo>(GetEndpoint()));
        RegisterAttribute(chip::app::Clusters::ApplicationBasic::Attributes::Status::Id,
                          new core::Attribute<chip::app::Clusters::ApplicationBasic::Attributes::Status::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::ApplicationBasic::Attributes::ApplicationVersion::Id,
            new core::Attribute<chip::app::Clusters::ApplicationBasic::Attributes::ApplicationVersion::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::ApplicationBasic::Attributes::AllowedVendorList::Id,
            new core::Attribute<chip::app::Clusters::ApplicationBasic::Attributes::AllowedVendorList::TypeInfo>(GetEndpoint()));
    }
};
}; // namespace application_basic

namespace account_login {
class AccountLoginCluster : public core::BaseCluster
{
public:
    AccountLoginCluster(memory::Weak<core::Endpoint> endpoint) : core::BaseCluster(endpoint) {}

    void SetUp()
    {
        ChipLogProgress(AppServer, "Setting up AccountLoginCluster on EndpointId: %d", GetEndpoint().lock()->GetId());

        RegisterCommand(chip::app::Clusters::AccountLogin::Commands::GetSetupPIN::Id,
                        new core::Command<chip::app::Clusters::AccountLogin::Commands::GetSetupPIN::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::AccountLogin::Commands::GetSetupPINResponse::Id,
                        new core::Command<chip::app::Clusters::AccountLogin::Commands::GetSetupPINResponse::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::AccountLogin::Commands::Login::Id,
                        new core::Command<chip::app::Clusters::AccountLogin::Commands::Login::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::AccountLogin::Commands::Logout::Id,
                        new core::Command<chip::app::Clusters::AccountLogin::Commands::Logout::Type>(GetEndpoint()));
    }
};
}; // namespace account_login

namespace application_launcher {
class ApplicationLauncherCluster : public core::BaseCluster
{
public:
    ApplicationLauncherCluster(memory::Weak<core::Endpoint> endpoint) : core::BaseCluster(endpoint) {}

    void SetUp()
    {
        ChipLogProgress(AppServer, "Setting up ApplicationLauncherCluster on EndpointId: %d", GetEndpoint().lock()->GetId());

        RegisterCommand(chip::app::Clusters::ApplicationLauncher::Commands::LaunchApp::Id,
                        new core::Command<chip::app::Clusters::ApplicationLauncher::Commands::LaunchApp::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::ApplicationLauncher::Commands::StopApp::Id,
                        new core::Command<chip::app::Clusters::ApplicationLauncher::Commands::StopApp::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::ApplicationLauncher::Commands::HideApp::Id,
                        new core::Command<chip::app::Clusters::ApplicationLauncher::Commands::HideApp::Type>(GetEndpoint()));

        RegisterAttribute(
            chip::app::Clusters::ApplicationLauncher::Attributes::CatalogList::Id,
            new core::Attribute<chip::app::Clusters::ApplicationLauncher::Attributes::CatalogList::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::ApplicationLauncher::Attributes::CurrentApp::Id,
            new core::Attribute<chip::app::Clusters::ApplicationLauncher::Attributes::CurrentApp::TypeInfo>(GetEndpoint()));
    }
};
}; // namespace application_launcher

namespace content_launcher {
class ContentLauncherCluster : public core::BaseCluster
{
public:
    ContentLauncherCluster(memory::Weak<core::Endpoint> endpoint) : core::BaseCluster(endpoint) {}

    void SetUp()
    {
        ChipLogProgress(AppServer, "Setting up ContentLauncherCluster on EndpointId: %d", GetEndpoint().lock()->GetId());

        RegisterCommand(chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Id,
                        new core::Command<chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::ContentLauncher::Commands::LaunchContent::Id,
                        new core::Command<chip::app::Clusters::ContentLauncher::Commands::LaunchContent::Type>(GetEndpoint()));

        RegisterAttribute(
            chip::app::Clusters::ContentLauncher::Attributes::SupportedStreamingProtocols::Id,
            new core::Attribute<chip::app::Clusters::ContentLauncher::Attributes::SupportedStreamingProtocols::TypeInfo>(
                GetEndpoint()));
    }
};
}; // namespace content_launcher

namespace keypad_input {
class KeypadInputCluster : public core::BaseCluster
{
public:
    KeypadInputCluster(memory::Weak<core::Endpoint> endpoint) : core::BaseCluster(endpoint) {}

    void SetUp()
    {
        ChipLogProgress(AppServer, "Setting up KeypadInputCluster on EndpointId: %d", GetEndpoint().lock()->GetId());
        RegisterCommand(chip::app::Clusters::KeypadInput::Commands::SendKey::Id,
                        new core::Command<chip::app::Clusters::KeypadInput::Commands::SendKey::Type>(GetEndpoint()));
    }
};
}; // namespace keypad_input

namespace level_control {
class LevelControlCluster : public core::BaseCluster
{
public:
    LevelControlCluster(memory::Weak<core::Endpoint> endpoint) : core::BaseCluster(endpoint) {}

    void SetUp()
    {
        ChipLogProgress(AppServer, "Setting up LevelControlCluster on EndpointId: %d", GetEndpoint().lock()->GetId());
        RegisterCommand(chip::app::Clusters::LevelControl::Commands::MoveToLevel::Id,
                        new core::Command<chip::app::Clusters::LevelControl::Commands::MoveToLevel::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::LevelControl::Commands::Move::Id,
                        new core::Command<chip::app::Clusters::LevelControl::Commands::Move::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::LevelControl::Commands::Step::Id,
                        new core::Command<chip::app::Clusters::LevelControl::Commands::Step::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::LevelControl::Commands::Stop::Id,
                        new core::Command<chip::app::Clusters::LevelControl::Commands::Stop::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Id,
                        new core::Command<chip::app::Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::LevelControl::Commands::MoveWithOnOff::Id,
                        new core::Command<chip::app::Clusters::LevelControl::Commands::MoveWithOnOff::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::LevelControl::Commands::StepWithOnOff::Id,
                        new core::Command<chip::app::Clusters::LevelControl::Commands::StepWithOnOff::Type>(GetEndpoint()));
        RegisterCommand(
            chip::app::Clusters::LevelControl::Commands::MoveToClosestFrequency::Id,
            new core::Command<chip::app::Clusters::LevelControl::Commands::MoveToClosestFrequency::Type>(GetEndpoint()));

        RegisterAttribute(
            chip::app::Clusters::LevelControl::Attributes::CurrentLevel::Id,
            new core::Attribute<chip::app::Clusters::LevelControl::Attributes::CurrentLevel::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::LevelControl::Attributes::RemainingTime::Id,
            new core::Attribute<chip::app::Clusters::LevelControl::Attributes::RemainingTime::TypeInfo>(GetEndpoint()));
        RegisterAttribute(chip::app::Clusters::LevelControl::Attributes::MinLevel::Id,
                          new core::Attribute<chip::app::Clusters::LevelControl::Attributes::MinLevel::TypeInfo>(GetEndpoint()));
        RegisterAttribute(chip::app::Clusters::LevelControl::Attributes::MaxLevel::Id,
                          new core::Attribute<chip::app::Clusters::LevelControl::Attributes::MaxLevel::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::LevelControl::Attributes::CurrentFrequency::Id,
            new core::Attribute<chip::app::Clusters::LevelControl::Attributes::CurrentFrequency::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::LevelControl::Attributes::MinFrequency::Id,
            new core::Attribute<chip::app::Clusters::LevelControl::Attributes::MinFrequency::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::LevelControl::Attributes::MaxFrequency::Id,
            new core::Attribute<chip::app::Clusters::LevelControl::Attributes::MaxFrequency::TypeInfo>(GetEndpoint()));
        RegisterAttribute(chip::app::Clusters::LevelControl::Attributes::Options::Id,
                          new core::Attribute<chip::app::Clusters::LevelControl::Attributes::Options::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::LevelControl::Attributes::OnOffTransitionTime::Id,
            new core::Attribute<chip::app::Clusters::LevelControl::Attributes::OnOffTransitionTime::TypeInfo>(GetEndpoint()));
        RegisterAttribute(chip::app::Clusters::LevelControl::Attributes::OnLevel::Id,
                          new core::Attribute<chip::app::Clusters::LevelControl::Attributes::OnLevel::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::LevelControl::Attributes::OnTransitionTime::Id,
            new core::Attribute<chip::app::Clusters::LevelControl::Attributes::OnTransitionTime::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::LevelControl::Attributes::DefaultMoveRate::Id,
            new core::Attribute<chip::app::Clusters::LevelControl::Attributes::DefaultMoveRate::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::LevelControl::Attributes::StartUpCurrentLevel::Id,
            new core::Attribute<chip::app::Clusters::LevelControl::Attributes::StartUpCurrentLevel::TypeInfo>(GetEndpoint()));
    }
};
}; // namespace level_control

namespace media_playback {
class MediaPlaybackCluster : public core::BaseCluster
{
public:
    MediaPlaybackCluster(memory::Weak<core::Endpoint> endpoint) : core::BaseCluster(endpoint) {}

    void SetUp()
    {
        ChipLogProgress(AppServer, "Setting up MediaPlaybackCluster on EndpointId: %d", GetEndpoint().lock()->GetId());
        RegisterCommand(chip::app::Clusters::MediaPlayback::Commands::Play::Id,
                        new core::Command<chip::app::Clusters::MediaPlayback::Commands::Play::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::MediaPlayback::Commands::Pause::Id,
                        new core::Command<chip::app::Clusters::MediaPlayback::Commands::Pause::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::MediaPlayback::Commands::Stop::Id,
                        new core::Command<chip::app::Clusters::MediaPlayback::Commands::Stop::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::MediaPlayback::Commands::StartOver::Id,
                        new core::Command<chip::app::Clusters::MediaPlayback::Commands::StartOver::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::MediaPlayback::Commands::Previous::Id,
                        new core::Command<chip::app::Clusters::MediaPlayback::Commands::Previous::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::MediaPlayback::Commands::Next::Id,
                        new core::Command<chip::app::Clusters::MediaPlayback::Commands::Next::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::MediaPlayback::Commands::Rewind::Id,
                        new core::Command<chip::app::Clusters::MediaPlayback::Commands::Rewind::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::MediaPlayback::Commands::FastForward::Id,
                        new core::Command<chip::app::Clusters::MediaPlayback::Commands::FastForward::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::MediaPlayback::Commands::SkipForward::Id,
                        new core::Command<chip::app::Clusters::MediaPlayback::Commands::SkipForward::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::MediaPlayback::Commands::SkipBackward::Id,
                        new core::Command<chip::app::Clusters::MediaPlayback::Commands::SkipBackward::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Id,
                        new core::Command<chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::MediaPlayback::Commands::Seek::Id,
                        new core::Command<chip::app::Clusters::MediaPlayback::Commands::Seek::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::MediaPlayback::Commands::ActivateAudioTrack::Id,
                        new core::Command<chip::app::Clusters::MediaPlayback::Commands::ActivateAudioTrack::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::MediaPlayback::Commands::ActivateTextTrack::Id,
                        new core::Command<chip::app::Clusters::MediaPlayback::Commands::ActivateTextTrack::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::MediaPlayback::Commands::DeactivateTextTrack::Id,
                        new core::Command<chip::app::Clusters::MediaPlayback::Commands::DeactivateTextTrack::Type>(GetEndpoint()));

        RegisterAttribute(
            chip::app::Clusters::MediaPlayback::Attributes::CurrentState::Id,
            new core::Attribute<chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo>(GetEndpoint()));
        RegisterAttribute(chip::app::Clusters::MediaPlayback::Attributes::StartTime::Id,
                          new core::Attribute<chip::app::Clusters::MediaPlayback::Attributes::StartTime::TypeInfo>(GetEndpoint()));
        RegisterAttribute(chip::app::Clusters::MediaPlayback::Attributes::Duration::Id,
                          new core::Attribute<chip::app::Clusters::MediaPlayback::Attributes::Duration::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::MediaPlayback::Attributes::SampledPosition::Id,
            new core::Attribute<chip::app::Clusters::MediaPlayback::Attributes::SampledPosition::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::MediaPlayback::Attributes::PlaybackSpeed::Id,
            new core::Attribute<chip::app::Clusters::MediaPlayback::Attributes::PlaybackSpeed::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::MediaPlayback::Attributes::SeekRangeEnd::Id,
            new core::Attribute<chip::app::Clusters::MediaPlayback::Attributes::SeekRangeEnd::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::MediaPlayback::Attributes::SeekRangeStart::Id,
            new core::Attribute<chip::app::Clusters::MediaPlayback::Attributes::SeekRangeStart::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::MediaPlayback::Attributes::ActiveAudioTrack::Id,
            new core::Attribute<chip::app::Clusters::MediaPlayback::Attributes::ActiveAudioTrack::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::MediaPlayback::Attributes::AvailableAudioTracks::Id,
            new core::Attribute<chip::app::Clusters::MediaPlayback::Attributes::AvailableAudioTracks::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::MediaPlayback::Attributes::ActiveTextTrack::Id,
            new core::Attribute<chip::app::Clusters::MediaPlayback::Attributes::ActiveTextTrack::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::MediaPlayback::Attributes::AvailableTextTracks::Id,
            new core::Attribute<chip::app::Clusters::MediaPlayback::Attributes::AvailableTextTracks::TypeInfo>(GetEndpoint()));
    }
};
}; // namespace media_playback

namespace on_off {
class OnOffCluster : public core::BaseCluster
{
public:
    OnOffCluster(memory::Weak<core::Endpoint> endpoint) : core::BaseCluster(endpoint) {}

    void SetUp()
    {
        ChipLogProgress(AppServer, "Setting up OnOffCluster on EndpointId: %d", GetEndpoint().lock()->GetId());

        RegisterCommand(chip::app::Clusters::OnOff::Commands::Off::Id,
                        new core::Command<chip::app::Clusters::OnOff::Commands::Off::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::OnOff::Commands::On::Id,
                        new core::Command<chip::app::Clusters::OnOff::Commands::On::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::OnOff::Commands::Toggle::Id,
                        new core::Command<chip::app::Clusters::OnOff::Commands::Toggle::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::OnOff::Commands::OffWithEffect::Id,
                        new core::Command<chip::app::Clusters::OnOff::Commands::OffWithEffect::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::OnOff::Commands::OnWithRecallGlobalScene::Id,
                        new core::Command<chip::app::Clusters::OnOff::Commands::OnWithRecallGlobalScene::Type>(GetEndpoint()));
        RegisterCommand(chip::app::Clusters::OnOff::Commands::OnWithTimedOff::Id,
                        new core::Command<chip::app::Clusters::OnOff::Commands::OnWithTimedOff::Type>(GetEndpoint()));

        RegisterAttribute(chip::app::Clusters::OnOff::Attributes::OnOff::Id,
                          new core::Attribute<chip::app::Clusters::OnOff::Attributes::OnOff::TypeInfo>(GetEndpoint()));
        RegisterAttribute(chip::app::Clusters::OnOff::Attributes::GlobalSceneControl::Id,
                          new core::Attribute<chip::app::Clusters::OnOff::Attributes::GlobalSceneControl::TypeInfo>(GetEndpoint()));
        RegisterAttribute(chip::app::Clusters::OnOff::Attributes::OnTime::Id,
                          new core::Attribute<chip::app::Clusters::OnOff::Attributes::OnTime::TypeInfo>(GetEndpoint()));
        RegisterAttribute(chip::app::Clusters::OnOff::Attributes::OffWaitTime::Id,
                          new core::Attribute<chip::app::Clusters::OnOff::Attributes::OffWaitTime::TypeInfo>(GetEndpoint()));
        RegisterAttribute(chip::app::Clusters::OnOff::Attributes::StartUpOnOff::Id,
                          new core::Attribute<chip::app::Clusters::OnOff::Attributes::StartUpOnOff::TypeInfo>(GetEndpoint()));
    }
};
}; // namespace on_off

namespace target_navigator {
class TargetNavigatorCluster : public core::BaseCluster
{
public:
    TargetNavigatorCluster(memory::Weak<core::Endpoint> endpoint) : core::BaseCluster(endpoint) {}

    void SetUp()
    {
        ChipLogProgress(AppServer, "Setting up TargetNavigatorCluster on EndpointId: %d", GetEndpoint().lock()->GetId());
        RegisterCommand(chip::app::Clusters::TargetNavigator::Commands::NavigateTarget::Id,
                        new core::Command<chip::app::Clusters::TargetNavigator::Commands::NavigateTarget::Type>(GetEndpoint()));

        RegisterAttribute(
            chip::app::Clusters::TargetNavigator::Attributes::TargetList::Id,
            new core::Attribute<chip::app::Clusters::TargetNavigator::Attributes::TargetList::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::TargetNavigator::Attributes::CurrentTarget::Id,
            new core::Attribute<chip::app::Clusters::TargetNavigator::Attributes::CurrentTarget::TypeInfo>(GetEndpoint()));
    }
};
}; // namespace target_navigator

namespace wake_on_lan {
class WakeOnLanCluster : public core::BaseCluster
{
public:
    WakeOnLanCluster(memory::Weak<core::Endpoint> endpoint) : core::BaseCluster(endpoint) {}

    void SetUp()
    {
        ChipLogProgress(AppServer, "Setting up WakeOnLanCluster on EndpointId: %d", GetEndpoint().lock()->GetId());
        RegisterAttribute(chip::app::Clusters::WakeOnLan::Attributes::MACAddress::Id,
                          new core::Attribute<chip::app::Clusters::WakeOnLan::Attributes::MACAddress::TypeInfo>(GetEndpoint()));
        RegisterAttribute(
            chip::app::Clusters::WakeOnLan::Attributes::LinkLocalAddress::Id,
            new core::Attribute<chip::app::Clusters::WakeOnLan::Attributes::LinkLocalAddress::TypeInfo>(GetEndpoint()));
    }
};
}; // namespace wake_on_lan

}; // namespace clusters
}; // namespace casting
}; // namespace matter

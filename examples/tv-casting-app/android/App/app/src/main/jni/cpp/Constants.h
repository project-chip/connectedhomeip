/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#pragma once

enum MediaCommandName
{
    ContentLauncher_LaunchURL,
    ContentLauncher_LaunchContent,
    LevelControl_Step,
    LevelControl_MoveToLevel,
    OnOff_On,
    OnOff_Off,
    OnOff_Toggle,
    MediaPlayback_Play,
    MediaPlayback_Pause,
    MediaPlayback_StopPlayback,
    MediaPlayback_Next,
    MediaPlayback_Previous,
    MediaPlayback_Rewind,
    MediaPlayback_FastForward,
    MediaPlayback_StartOver,
    MediaPlayback_Seek,
    MediaPlayback_SkipForward,
    MediaPlayback_SkipBackward,
    Messages_PresentMessagesRequest,
    ApplicationLauncher_LaunchApp,
    ApplicationLauncher_StopApp,
    ApplicationLauncher_HideApp,
    TargetNavigator_NavigateTarget,
    KeypadInput_SendKey,

    MEDIA_COMMAND_COUNT
};

enum MediaAttributeName
{
    ContentLauncher_SupportedStreamingProtocols,
    ContentLauncher_AcceptHeader,
    LevelControl_CurrentLevel,
    LevelControl_MinLevel,
    LevelControl_MaxLevel,
    MediaPlayback_CurrentState,
    MediaPlayback_StartTime,
    MediaPlayback_Duration,
    MediaPlayback_SampledPosition,
    MediaPlayback_PlaybackSpeed,
    MediaPlayback_SeekRangeEnd,
    MediaPlayback_SeekRangeStart,
    ApplicationLauncher_CurrentApp,
    TargetNavigator_TargetList,
    TargetNavigator_CurrentTarget,
    ApplicationBasic_VendorName,
    ApplicationBasic_VendorID,
    ApplicationBasic_ApplicationName,
    ApplicationBasic_ProductID,
    ApplicationBasic_ApplicationVersion,

    MEDIA_ATTRIBUTE_COUNT
};

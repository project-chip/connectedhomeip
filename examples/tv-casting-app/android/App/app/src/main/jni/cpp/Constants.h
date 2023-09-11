/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

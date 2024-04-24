#
#   Copyright (c) 2023 Project CHIP Authors
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

cmake_minimum_required(VERSION 3.21)

set(CHIP_TV_COMMON_BASE_DIR ${CMAKE_CURRENT_LIST_DIR})
if (NOT CHIP_ROOT)
    get_filename_component(CHIP_ROOT ${CHIP_TV_COMMON_BASE_DIR}/../../.. REALPATH)
endif()

# Add common lock sources to the specific target
# [Args]:
#   target - target name
    # Available options are:
#   SCOPE   sources scope for the target, PRIVATE as default
#   SHELL_CMD add shell commands support
macro(chip_add_tv_app_common target)
    set(SCOPE PRIVATE)
    set(oneValueArgs
        SCOPE
        SHELL_CMD
    )
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "" ${ARGN})
    if (ARG_SCOPE)
        set(SCOPE ${ARG_SCOPE})
    endif()

    target_include_directories(${target}
        ${SCOPE}
            ${CHIP_TV_COMMON_BASE_DIR}/include
            ${CHIP_TV_COMMON_BASE_DIR}/clusters
            ${CHIP_ROOT}/src/app/app-platform
    )

    target_sources(${target}
        ${SCOPE}
            ${CHIP_TV_COMMON_BASE_DIR}/src/AppTv.cpp
            ${CHIP_TV_COMMON_BASE_DIR}/src/ZCLCallbacks.cpp

            ${CHIP_TV_COMMON_BASE_DIR}/clusters/account-login/AccountLoginManager.cpp
            ${CHIP_TV_COMMON_BASE_DIR}/clusters/application-basic/ApplicationBasicManager.cpp
            ${CHIP_TV_COMMON_BASE_DIR}/clusters/application-launcher/ApplicationLauncherManager.cpp
            ${CHIP_TV_COMMON_BASE_DIR}/clusters/audio-output/AudioOutputManager.cpp
            ${CHIP_TV_COMMON_BASE_DIR}/clusters/channel/ChannelManager.cpp
            ${CHIP_TV_COMMON_BASE_DIR}/clusters/content-launcher/ContentLauncherManager.cpp
            ${CHIP_TV_COMMON_BASE_DIR}/clusters/content-control/ContentController.cpp
            ${CHIP_TV_COMMON_BASE_DIR}/clusters/content-app-observer/ContentAppObserver.cpp
            ${CHIP_TV_COMMON_BASE_DIR}/clusters/keypad-input/KeypadInputManager.cpp
            ${CHIP_TV_COMMON_BASE_DIR}/clusters/low-power/LowPowerManager.cpp
            ${CHIP_TV_COMMON_BASE_DIR}/clusters/media-input/MediaInputManager.cpp
            ${CHIP_TV_COMMON_BASE_DIR}/clusters/media-playback/MediaPlaybackManager.cpp
            ${CHIP_TV_COMMON_BASE_DIR}/clusters/messages/MessagesManager.cpp
            ${CHIP_TV_COMMON_BASE_DIR}/clusters/target-navigator/TargetNavigatorManager.cpp
            ${CHIP_TV_COMMON_BASE_DIR}/clusters/wake-on-lan/WakeOnLanManager.cpp

            ${CHIP_ROOT}/src/app/app-platform/ContentAppPlatform.cpp
            ${CHIP_ROOT}/src/app/app-platform/ContentApp.cpp
    )

    if (${ARG_SHELL_CMD})
        target_include_directories(${target}
            ${SCOPE}
                ${CHIP_TV_COMMON_BASE_DIR}/shell
        )

        target_sources(${target}
            ${SCOPE}
                ${CHIP_TV_COMMON_BASE_DIR}/shell/AppTvShellCommands.cpp
        )

        target_compile_definitions(${target}
            ${SCOPE}
                CHIP_TV_APP_SHELL_CMD_ENABLE
        )
    endif()
endmacro()

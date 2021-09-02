# Copyright (c) 2021 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from typing import Set

from builders.android import AndroidBoard, AndroidBuilder
from builders.efr32 import Efr32Builder, Efr32App, Efr32Board
from builders.esp32 import Esp32Builder, Esp32Board, Esp32App
from builders.host import HostBuilder, HostApp
from builders.nrf import NrfApp, NrfBoard, NrfConnectBuilder
from builders.qpg import QpgBuilder
from builders.infineon import InfineonBuilder, InfineonApp, InfineonBoard
from builders.telink import TelinkApp, TelinkBoard, TelinkBuilder

from .targets import Application, Board, Platform


class MatchApplication:

    def __init__(self, app, board=None):
        self.app = app
        self.board = board

    def Match(self, board: Board, app: Application):
        if app != self.app:
            return False
        return self.board is None or board == self.board


class Matcher():
    """Figures out if a proper builder can be created for a platform/board/app combination."""

    def __init__(self, builder_class):
        self.builder_class = builder_class
        self.app_arguments = {}
        self.board_arguments = {}

    def AcceptApplication(self, __app_key: Application, **kargs):
        self.app_arguments[MatchApplication(__app_key)] = kargs

    def AcceptApplicationForBoard(self, __app_key: Application, __board: Board,
                                  **kargs):
        self.app_arguments[MatchApplication(__app_key, __board)] = kargs

    def AcceptBoard(self, __board_key: Board, **kargs):
        self.board_arguments[__board_key] = kargs

    def Create(self, runner, __board_key: Board, __app_key: Application,
               repo_path: str, **kargs):
        """Creates a new builder for the given board/app. """
        if not __board_key in self.board_arguments:
            return None

        extra_app_args = None
        for key, value in self.app_arguments.items():
            if key.Match(__board_key, __app_key):
                extra_app_args = value
                break

        if extra_app_args is None:
            return None

        kargs.update(self.board_arguments[__board_key])
        kargs.update(extra_app_args)

        return self.builder_class(repo_path, runner=runner, **kargs)


# Builds a list of acceptable application/board combination for every platform
_MATCHERS = {
    Platform.HOST: Matcher(HostBuilder),
    Platform.ESP32: Matcher(Esp32Builder),
    Platform.QPG: Matcher(QpgBuilder),
    Platform.EFR32: Matcher(Efr32Builder),
    Platform.NRF: Matcher(NrfConnectBuilder),
    Platform.ANDROID: Matcher(AndroidBuilder),
    Platform.INFINEON: Matcher(InfineonBuilder),
    Platform.TELINK: Matcher(TelinkBuilder),
}

# Matrix of what can be compiled and what build options are required
# by such compilation
_MATCHERS[Platform.HOST].AcceptBoard(Board.NATIVE)
_MATCHERS[Platform.HOST].AcceptApplication(
    Application.ALL_CLUSTERS, app=HostApp.ALL_CLUSTERS)
_MATCHERS[Platform.HOST].AcceptApplication(
    Application.CHIP_TOOL, app=HostApp.CHIP_TOOL)
_MATCHERS[Platform.HOST].AcceptApplication(
    Application.THERMOSTAT, app=HostApp.THERMOSTAT)

_MATCHERS[Platform.ESP32].AcceptBoard(Board.DEVKITC, board=Esp32Board.DevKitC)
_MATCHERS[Platform.ESP32].AcceptBoard(Board.M5STACK, board=Esp32Board.M5Stack)
_MATCHERS[Platform.ESP32].AcceptBoard(
    Board.C3DEVKIT, board=Esp32Board.C3DevKit)
_MATCHERS[Platform.ESP32].AcceptApplication(
    Application.ALL_CLUSTERS, app=Esp32App.ALL_CLUSTERS)
_MATCHERS[Platform.ESP32].AcceptApplicationForBoard(
    Application.SHELL, Board.DEVKITC, app=Esp32App.SHELL)
_MATCHERS[Platform.ESP32].AcceptApplicationForBoard(
    Application.LOCK, Board.DEVKITC, app=Esp32App.LOCK)
_MATCHERS[Platform.ESP32].AcceptApplicationForBoard(
    Application.BRIDGE, Board.DEVKITC, app=Esp32App.BRIDGE)
_MATCHERS[Platform.ESP32].AcceptApplicationForBoard(
    Application.TEMPERATURE_MEASUREMENT, Board.DEVKITC, app=Esp32App.TEMPERATURE_MEASUREMENT)

_MATCHERS[Platform.QPG].AcceptApplication(Application.LOCK)
_MATCHERS[Platform.QPG].AcceptBoard(Board.QPG6100)

_MATCHERS[Platform.EFR32].AcceptBoard(
    Board.BRD4161A, board=Efr32Board.BRD4161A)
_MATCHERS[Platform.EFR32].AcceptApplication(
    Application.LIGHT, app=Efr32App.LIGHT)
_MATCHERS[Platform.EFR32].AcceptApplication(
    Application.LOCK, app=Efr32App.LOCK)
_MATCHERS[Platform.EFR32].AcceptApplication(
    Application.WINDOW_COVERING, app=Efr32App.WINDOW_COVERING)

_MATCHERS[Platform.NRF].AcceptBoard(Board.NRF5340, board=NrfBoard.NRF5340)
_MATCHERS[Platform.NRF].AcceptBoard(Board.NRF52840, board=NrfBoard.NRF52840)
_MATCHERS[Platform.NRF].AcceptApplication(Application.LOCK, app=NrfApp.LOCK)
_MATCHERS[Platform.NRF].AcceptApplication(Application.LIGHT, app=NrfApp.LIGHT)
_MATCHERS[Platform.NRF].AcceptApplication(Application.SHELL, app=NrfApp.SHELL)
_MATCHERS[Platform.NRF].AcceptApplication(Application.PUMP, app=NrfApp.PUMP)
_MATCHERS[Platform.NRF].AcceptApplication(
    Application.PUMP_CONTROLLER, app=NrfApp.PUMP_CONTROLLER)

_MATCHERS[Platform.TELINK].AcceptBoard(
    Board.TLSR9518ADK80D, board=TelinkBoard.TLSR9518ADK80D)
_MATCHERS[Platform.TELINK].AcceptApplication(
    Application.LIGHT, app=TelinkApp.LIGHT)

_MATCHERS[Platform.ANDROID].AcceptBoard(Board.ARM, board=AndroidBoard.ARM)
_MATCHERS[Platform.ANDROID].AcceptBoard(Board.ARM64, board=AndroidBoard.ARM64)
_MATCHERS[Platform.ANDROID].AcceptBoard(Board.X64, board=AndroidBoard.X64)
_MATCHERS[Platform.ANDROID].AcceptApplication(Application.CHIP_TOOL)

_MATCHERS[Platform.INFINEON].AcceptApplication(
    Application.LOCK, app=InfineonApp.LOCK)
_MATCHERS[Platform.INFINEON].AcceptBoard(
    Board.P6BOARD, board=InfineonBoard.P6BOARD)


class BuilderFactory:
    """Creates application builders."""

    def __init__(self, runner, repository_path: str, output_prefix: str):
        self.runner = runner
        self.repository_path = repository_path
        self.output_prefix = output_prefix

    def Create(self, platform: Platform, board: Board, app: Application, enable_flashbundle: bool = False):
        """Creates a builder object for the specified arguments. """

        builder = _MATCHERS[platform].Create(
            self.runner,
            board,
            app,
            self.repository_path,
            output_prefix=self.output_prefix)

        if builder:
            builder.SetIdentifier(platform.name.lower(),
                                  board.name.lower(), app.name.lower())
            builder.enable_flashbundle(enable_flashbundle)

        return builder


class TargetRelations:
    """Figures out valid combinations of boards/platforms/applications."""

    @staticmethod
    def BoardsForPlatform(platform: Platform) -> Set[Board]:
        global _MATCHERS
        return set(_MATCHERS[platform].board_arguments.keys())

    @staticmethod
    def PlatformsForBoard(board: Board) -> Set[Platform]:
        """Return the platforms that are using the specified board."""
        global _MATCHERS
        platforms = set()
        for platform, matcher in _MATCHERS.items():
            if board in matcher.board_arguments:
                platforms.add(platform)
        return platforms

    @staticmethod
    def ApplicationsForPlatform(platform: Platform) -> Set[Application]:
        """What applications are buildable for a specific platform."""
        global _MATCHERS
        return set(
            [matcher.app for matcher in _MATCHERS[platform].app_arguments.keys()])

    @staticmethod
    def PlatformsForApplication(application: Application) -> Set[Platform]:
        """For what platforms can the given application be compiled."""
        global _MATCHERS
        platforms = set()
        for platform, matcher in _MATCHERS.items():
            for app_matcher in matcher.app_arguments:
                if application == app_matcher.app:
                    platforms.add(platform)
                    break
        return platforms

import os

from typing import Set

from builders.builder import Builder
from builders.linux import LinuxBuilder
from builders.qpg import QpgBuilder
from builders.esp32 import Esp32Builder, Esp32Board, Esp32App
from builders.efr32 import Efr32Builder, Efr32App, Efr32Board

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

  def Create(self, __board_key: Board, __app_key: Application, repo_path: str,
             **kargs):
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

    return self.builder_class(repo_path, **kargs)


# Builds a list of acceptable application/board combination for every platform
_MATCHERS = {
    Platform.LINUX: Matcher(LinuxBuilder),
    Platform.ESP32: Matcher(Esp32Builder),
    Platform.QPG: Matcher(QpgBuilder),
    Platform.EFR32: Matcher(Efr32Builder),
}

# Matrix of what can be compiled and what build options are required
# by such compilation
_MATCHERS[Platform.LINUX].AcceptApplication(Application.ALL_CLUSTERS)
_MATCHERS[Platform.LINUX].AcceptBoard(Board.NATIVE)

_MATCHERS[Platform.ESP32].AcceptBoard(Board.DEVKITC, board=Esp32Board.DevKitC)
_MATCHERS[Platform.ESP32].AcceptBoard(Board.M5STACK, board=Esp32Board.M5Stack)
_MATCHERS[Platform.ESP32].AcceptApplication(
    Application.ALL_CLUSTERS, app=Esp32App.ALL_CLUSTERS)
_MATCHERS[Platform.ESP32].AcceptApplicationForBoard(
    Application.LOCK, Board.DEVKITC, app=Esp32App.LOCK)

_MATCHERS[Platform.QPG].AcceptApplication(Application.LOCK)
_MATCHERS[Platform.QPG].AcceptBoard(Board.QPG6100)

_MATCHERS[Platform.EFR32].AcceptBoard(Board.BRD4161A, board=Efr32Board.BRD4161A)
_MATCHERS[Platform.EFR32].AcceptApplication(
    Application.LIGHT, app=Efr32App.LIGHT)
_MATCHERS[Platform.EFR32].AcceptApplication(Application.LOCK, app=Efr32App.LOCK)
_MATCHERS[Platform.EFR32].AcceptApplication(
    Application.WINDOW_COVERING, app=Efr32App.WINDOW_COVERING)


class BuilderFactory:
  """Creates application builders."""

  def __init__(self, repository_path: str, output_prefix: str):
    self.repository_path = repository_path
    self.output_prefix = output_prefix

  def Create(self, platform: Platform, board: Board, app: Application):
    """Creates a builder object for the specified arguments. """

    identifier = '%s-%s-%s' % (platform.name.lower(), board.name.lower(),
                               app.name.lower())

    output_directory = os.path.join(self.output_prefix, identifier)
    builder = _MATCHERS[platform].Create(
        board, app, self.repository_path, output_dir=output_directory)

    if builder:
      builder.identifier = identifier

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

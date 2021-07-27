import logging
import os
import shutil

from enum import IntEnum, auto


class Platform(IntEnum):
  """Represents a supported build platform for compilation."""
  LINUX = auto()
  QPG = auto()
  ESP32 = auto()
  EFR32 = auto()
  NRF = auto()

  @property
  def ArgName(self):
    return self.name.lower()

  @staticmethod
  def FromArgName(name):
    for value in Platform:
      if name == value.ArgName:
        return value
    raise KeyError()


class Board(IntEnum):
  """Represents Specific boards within a platform."""
  # Host builds
  NATIVE = auto()

  # QPG platform
  QPG6100 = auto()

  # ESP32 platform
  M5STACK = auto()
  DEVKITC = auto()

  # EFR32 platform
  BRD4161A = auto()

  # NRF platform
  NRF52840 = auto()
  NRF5340 = auto()

  @property
  def ArgName(self):
    return self.name.lower()

  @staticmethod
  def FromArgName(name):
    for value in Board:
      if name == value.ArgName:
        return value
    raise KeyError()


class Application(IntEnum):
  """Example applications that can be built."""
  ALL_CLUSTERS = auto()
  LIGHT = auto()
  LOCK = auto()
  WINDOW_COVERING = auto()
  SHELL = auto()

  @property
  def ArgName(self):
    return self.name.lower().replace('_', '-')

  @staticmethod
  def FromArgName(name):
    for value in Application:
      if name == value.ArgName:
        return value
    raise KeyError()

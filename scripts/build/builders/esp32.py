import logging
import os
import shlex

from enum import Enum, auto

from .builder import Builder


class Esp32Board(Enum):
  DevKitC = auto()
  M5Stack = auto()


class Esp32App(Enum):
  ALL_CLUSTERS = auto()
  LOCK = auto()

  @property
  def ExampleName(self):
    if self == Esp32App.ALL_CLUSTERS:
      return 'all-clusters-app'
    elif self == Esp32App.LOCK:
      return 'lock-app'
    else:
      raise Exception('Unknown app type: %r' % self)

  @property
  def AppNamePrefix(self):
    if self == Esp32App.ALL_CLUSTERS:
      return 'chip-all-clusters-app'
    elif self == Esp32App.LOCK:
      return 'chip-lock-app'
    else:
      raise Exception('Unknown app type: %r' % self)


def DefaultsFileName(board: Esp32Board, app: Esp32App):
  if app != Esp32App.ALL_CLUSTERS:
    # only all-clusters has a specific defaults name
    return None

  if board == Esp32Board.DevKitC:
    return 'sdkconfig_devkit.defaults'
  elif board == Esp32Board.M5Stack:
    return 'sdkconfig_m5stack.defaults'
  else:
    raise Exception('Unknown board type')


class Esp32Builder(Builder):

  def __init__(self,
               root,
               output_dir: str,
               board: Esp32Board = Esp32Board.M5Stack,
               app: Esp32App = Esp32App.ALL_CLUSTERS):
    super(Esp32Builder, self).__init__(root, output_dir)
    self.board = board
    self.app = app

  def _IdfEnvExecute(self, cmd, **kargs):
    self._Execute(
        ['bash', '-c', 'source $IDF_PATH/export.sh; %s' % cmd], **kargs)

  def generate(self):
    if os.path.exists(os.path.join(self.output_dir, 'build.ninja')):
      return

    defaults = DefaultsFileName(self.board, self.app)

    cmd = 'idf.py'

    if defaults:
      cmd += " -D SDKCONFIG_DEFAULTS='%s'" % defaults

    cmd += ' -C examples/%s/esp32 -B %s reconfigure' % (self.app.ExampleName, shlex.quote(self.output_dir))

    # This will do a 'cmake reconfigure' which will create ninja files without rebuilding
    self._IdfEnvExecute(cmd, cwd=self.root)

  def build(self):
    logging.info('Compiling Esp32 at %s', self.output_dir)

    self.generate()
    self._IdfEnvExecute("ninja -C '%s'" % self.output_dir)

  def outputs(self):
    return {
        self.app.AppNamePrefix + '.elf':
            os.path.join(self.output_dir, self.app.AppNamePrefix + '.elf'),
        self.app.AppNamePrefix + '.map':
            os.path.join(self.output_dir, self.app.AppNamePrefix + '.map'),
    }

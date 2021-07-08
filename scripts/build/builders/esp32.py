import logging
import os
from enum import Enum

from .builder import Builder


class Esp32Board(Enum):
  DevKitC = 1
  M5Stack = 2

  def DefaultsFileName(self):
    if self == Esp32Board.DevKitC:
      return 'sdkconfig_devkit.defaults'
    elif self == Esp32Board.M5Stack:
      return 'sdkconfig_m5stack.defaults'
    raise Exception('Unknown board type')


class Esp32Builder(Builder):

  def __init__(self,
               root,
               output_dir: str,
               board: Esp32Board = Esp32Board.M5Stack):
    super(Esp32Builder, self).__init__(root, output_dir)
    self.board = board

  def _IdfEnvExecute(self, cmd, **kargs):
    self._ActivatedExecute('source "$IDF_PATH/export.sh"; %s' % cmd, **kargs)

  def generate(self):
    if not os.path.exists(os.path.join(self.output_dir, 'build.ninja')):
      # This will do a 'cmake reconfigure' which will create ninja files without rebuilding
      self._IdfEnvExecute(
          "idf.py -D SDKCONFIG_DEFAULTS='%s' -C examples/all-clusters-app/esp32 -B %s reconfigure"
          % (self.board.DefaultsFileName(), self.output_dir),
          cwd=self.root)

  def build(self):
    logging.info('Compiling Esp32 at %s', self.output_dir)

    self.generate()
    self._IdfEnvExecute("ninja -C '%s'" % self.output_dir)

  def outputs(self):
    return {
        'chip-all-clusters-app.elf':
            os.path.join(self.output_dir, 'chip-all-clusters-app.elf'),
        'chip-all-clusters-app.map':
            os.path.join(self.output_dir, 'chip-all-clusters-app.map'),
    }

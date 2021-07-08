import logging
import os
from enum import Enum, auto

from .builder import Builder


class Efr32App(Enum):
  LIGHT = auto()
  LOCK = auto()
  WINDOW_COVERING = auto()

  def ExampleName(self):
    if self == Efr32App.LIGHT:
      return 'lighting-app'
    elif self == Efr32App.LOCK:
      return 'lock-app'
    elif self == Efr32App.WINDOW_COVERING:
      return 'window-app'
    else:
      raise Exception('Unknown app type: %r' % self)

  def AppNamePrefix(self):
    if self == Efr32App.LIGHT:
      return 'chip-efr32-lighting-example'
    elif self == Efr32App.LOCK:
      return 'chip-efr32-lock-example'
    elif self == Efr32App.WINDOW_COVERING:
      return 'chip-efr32-window-example'
    else:
      raise Exception('Unknown app type: %r' % self)

  def FlashBundleName(self):
    if self == Efr32App.LIGHT:
      return 'lighting_app.flashbundle.txt'
    elif self == Efr32App.LOCK:
      return 'lock_app.flashbundle.txt'
    elif self == Efr32App.WINDOW_COVERING:
      return 'window_app.flashbundle.txt'
    else:
      raise Exception('Unknown app type: %r' % self)


class Efr32Board(Enum):
  BRD4161A = 1

  def GnArgName(self):
    if self == Efr32Board.BRD4161A:
      return 'BRD4161A'


class Efr32Builder(Builder):

  def __init__(self,
               root,
               output_dir: str,
               app: Efr32App = Efr32App.LIGHT,
               board: Efr32Board = Efr32Board.BRD4161A):
    super(Efr32Builder, self).__init__(root, output_dir)

    self.app = app
    self.board = board

  def generate(self):
    if not os.path.exists(self.output_dir):
      self._Execute([
          'gn', 'gen', '--check', '--fail-on-unused-args',
          '--root=%s' %
          os.path.join(self.root, 'examples', self.app.ExampleName(), 'efr32'),
          '--args=efr32_board="%s"' % self.board.GnArgName(), self.output_dir
      ])

  def build(self):
    logging.info('Compiling EFR32 at %s', self.output_dir)

    self.generate()
    self._Execute(['ninja', '-C', self.output_dir])

  def outputs(self):
    items = {
        '%s.out' % self.app.AppNamePrefix():
            os.path.join(self.output_dir, '%s.out' % self.app.AppNamePrefix()),
        '%s.out.map' % self.app.AppNamePrefix():
            os.path.join(self.output_dir,
                         '%s.out.map' % self.app.AppNamePrefix()),
    }

    # Figure out flash bundle files and build accordingly
    with open(os.path.join(self.output_dir, self.app.FlashBundleName())) as f:
      for line in f.readlines():
        name = line.strip()
        items['flashbundle/%s' % name] = os.path.join(self.output_dir, name)

    return items

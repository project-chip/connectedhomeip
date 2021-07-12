#!/usr/bin/env python3

import logging
import os

from .builder import Builder


class QpgBuilder(Builder):

  def __init__(self, root, output_dir):
    super(QpgBuilder, self).__init__(root, output_dir)

  def generate(self):
    if not os.path.exists(self.output_dir):
      self._Execute(['gn', 'gen', self.output_dir],
                    cwd=os.path.join(self.root, 'examples/lock-app/qpg/'))

  def build(self):
    logging.info('Compiling QPG at %s', self.output_dir)

    self.generate()
    self._Execute(['ninja', '-C', self.output_dir])

  def outputs(self):
    return {
        'chip-qpg-lock-example.out':
            os.path.join(self.output_dir, 'chip-qpg6100-lock-example.out'),
        'chip-qpg-lock-example.out.map':
            os.path.join(self.output_dir, 'chip-qpg6100-lock-example.out.map'),
    }

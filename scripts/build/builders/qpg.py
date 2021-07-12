#!/usr/bin/env python3

import logging
import os

from .gn import GnBuilder


class QpgBuilder(GnBuilder):

  def __init__(self, root, runner, output_dir):
    super(QpgBuilder, self).__init__(
        root=os.path.join(root, 'examples/lock-app/qpg/'),
        runner=runner,
        output_dir=output_dir)

  def outputs(self):
    return {
        'chip-qpg-lock-example.out':
            os.path.join(self.output_dir, 'chip-qpg6100-lock-example.out'),
        'chip-qpg-lock-example.out.map':
            os.path.join(self.output_dir, 'chip-qpg6100-lock-example.out.map'),
    }

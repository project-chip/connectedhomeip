import logging
import os

from .gn import GnBuilder


class LinuxBuilder(GnBuilder):

  def __init__(self, root, runner, output_dir):
    super(LinuxBuilder, self).__init__(
        root=os.path.join(root, 'examples/all-clusters-app/linux/'),
        runner=runner,
        output_dir=output_dir)

  def outputs(self):
    return {
        'chip-all-clusters-app':
            os.path.join(self.output_dir, 'chip-all-clusters-app'),
        'chip-all-clusters-app.map':
            os.path.join(self.output_dir, 'chip-all-clusters-app.map'),
    }

import logging
import os

from .builder import Builder


class LinuxBuilder(Builder):

  def __init__(self, root, output_dir):
    super(LinuxBuilder, self).__init__(root, output_dir)

  def generate(self):
    if not os.path.exists(self.output_dir):
      self._Execute(['gn', 'gen', self.output_dir],
                    cwd=os.path.join(self.root,
                                     'examples/all-clusters-app/linux/'))

  def build(self):
    logging.info('Compiling Linux at %s', self.output_dir)

    self.generate()
    self._Execute(['ninja', '-C', self.output_dir])

  def outputs(self):
    return {
        'chip-all-clusters-app':
            os.path.join(self.output_dir, 'chip-all-clusters-app'),
        'chip-all-clusters-app.map':
            os.path.join(self.output_dir, 'chip-all-clusters-app.map'),
    }

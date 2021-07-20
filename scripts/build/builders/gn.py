import logging
import os

from .builder import Builder


class GnBuilder(Builder):

  def __init__(self, root, runner, output_dir):
    """Creates  a generic ninja builder.

    Args:
       root: the root where to run GN into
       runner: what to use to execute shell commands
       output_dir: where ninja files are to be generated
    """
    super(GnBuilder, self).__init__(root, runner, output_dir)

    self.gn_build_args = None

  def generate(self):
    if not os.path.exists(self.output_dir):
      cmd = [
          'gn', 'gen', '--check', '--fail-on-unused-args',
          '--root=%s' % self.root
      ]

      if self.gn_build_args:
        cmd += ['--args=%s' % ' '.join(self.gn_build_args)]

      cmd += [self.output_dir]

      self._Execute(cmd, title='Generating ' + self.identifier)

  def build(self):
    self.generate()
    self._Execute(['ninja', '-C', self.output_dir],
                  title='Building ' + self.identifier)

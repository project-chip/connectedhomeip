#!/usr/bin/env python3

import logging
import os
import shutil
from abc import ABC, abstractmethod

from shellrunner import ShellRunner


class Builder(ABC):
  """Generic builder base class for CHIP.

  Provides ability to boostrap and copy output artifacts and subclasses can use
  a generic shell runner.

  """

  def __init__(self, root, output_dir='out'):
    self.root = os.path.abspath(root)
    self._runner = ShellRunner()
    self.output_dir = output_dir

  @abstractmethod
  def generate(self):
    """Generate the build files - generally the ninja/makefiles"""
    raise NotImplementedError()

  @abstractmethod
  def build(self):
    """Perform an actual build"""
    raise NotImplementedError()

  @abstractmethod
  def outputs(self):
    """Return a list of relevant output files after a build.

       May use build output data (e.g. manifests), so this should be invoked
       only after a build has succeeded.
    """
    raise NotImplementedError()

  def _Execute(self, cmdarray, **args):
    self._runner.Run(cmdarray, **args)

  def _ActivatedExecute(self, cmdstring: str, **args):
    """Switches to top level to activate then runs the given command in the original run directory."""
    self._runner.Run([
        'bash', '-c',
        'cd "%s"; source ./scripts/activate.sh; cd -; %s' %
        (self.root, cmdstring)
    ], **args)

  def Bootstrap(self):
    envpath = os.environ.get('PW_ENVIRONMENT_ROOT',
                             os.path.join(self.root, '.environment'))

    if not os.path.exists(os.path.join(envpath, 'activate.sh')):
      logging.info('Bootstrapping in %s (%s does not look valid )', self.root,
                   envpath)
      self._Execute(['bash', '-c', 'source ./scripts/bootstrap.sh'],
                    cwd=self.root)
    else:
      logging.info('Project already bootstrapped in %s (environment in %s)',
                   self.root, envpath)

  def CopyArtifacts(self, target_dir: str):
    for target_name, source_name in self.outputs().items():
      target_full_name = os.path.join(target_dir, target_name)

      logging.info('Copying %s into %s', source_name, target_name)

      target_dir_full_name = os.path.dirname(target_full_name)

      if not os.path.exists(target_dir_full_name):
        logging.info('   Creating subdirectory %s first', target_dir_full_name)
        os.makedirs(target_dir_full_name)

      shutil.copyfile(source_name, target_full_name)

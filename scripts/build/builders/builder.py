# Copyright (c) 2021 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import logging
import os
import shutil
from abc import ABC, abstractmethod


class Builder(ABC):
  """Generic builder base class for CHIP.

  Provides ability to boostrap and copy output artifacts and subclasses can use
  a generic shell runner.

  """

  def __init__(self, root, runner, output_prefix: str ='out'):
    self.root = os.path.abspath(root)
    self._runner = runner
    self.output_prefix = output_prefix

    # Set post-init once actual build target is known
    self.identifier = None
    self.output_dir = None

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

  def _Execute(self, cmdarray, cwd=None, title=None):
    self._runner.Run(cmdarray, cwd=cwd, title=title)

  def CopyArtifacts(self, target_dir: str):
    for target_name, source_name in self.outputs().items():
      target_full_name = os.path.join(target_dir, target_name)

      logging.info('Copying %s into %s', source_name, target_name)

      target_dir_full_name = os.path.dirname(target_full_name)

      if not os.path.exists(target_dir_full_name):
        logging.info('Creating subdirectory %s first', target_dir_full_name)
        os.makedirs(target_dir_full_name)

      shutil.copyfile(source_name, target_full_name)

  def SetIdentifier(self, platform: str, board: str, app: str):
    self.identifier = '-'.join([platform, board, app])
    self.output_dir = os.path.join(self.output_prefix, self.identifier)

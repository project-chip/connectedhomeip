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

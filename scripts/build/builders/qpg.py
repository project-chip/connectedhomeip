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

import os

from .gn import GnBuilder


class QpgBuilder(GnBuilder):

    def __init__(self, root, runner, output_prefix):
        super(QpgBuilder, self).__init__(
            root=os.path.join(root, 'examples/lock-app/qpg/'),
            runner=runner,
            output_prefix=output_prefix)

    def build_outputs(self):
        return {
            'chip-qpg-lock-example.out':
                os.path.join(self.output_dir, 'chip-qpg6100-lock-example.out'),
            'chip-qpg-lock-example.out.map':
                os.path.join(self.output_dir,
                             'chip-qpg6100-lock-example.out.map'),
        }

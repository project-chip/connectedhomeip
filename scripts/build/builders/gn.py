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
import shlex

from .builder import Builder


class GnBuilder(Builder):

    def __init__(self, root, runner, output_prefix):
        """Creates  a generic ninja builder.

        Args:
           root: the root where to run GN into
           runner: what to use to execute shell commands
           output_prefix: where ninja files are to be generated
        """
        super(GnBuilder, self).__init__(root, runner, output_prefix)

    def GnBuildArgs(self):
        """Extra gn build `--args`

        If used, returns a list of arguments.
        """
        return None

    def GnBuildEnv(self):
        """Extra environment variables needed for the GN build to run.

        If used, returns a dictionary of environment variables.
        """
        return None

    def generate(self):
        if not os.path.exists(self.output_dir):
            cmd = [
                'gn', 'gen', '--check', '--fail-on-unused-args',
                '--root=%s' % self.root
            ]

            extra_args = self.GnBuildArgs()
            if extra_args:
                cmd += ['--args=%s' % ' '.join(extra_args)]

            cmd += [self.output_dir]

            title = 'Generating ' + self.identifier
            extra_env = self.GnBuildEnv()

            if extra_env:
                # convert the command into a bash command that includes
                # setting environment variables
                cmd = [
                    'bash', '-c', '\n' + ' '.join(
                        ['%s="%s" \\\n' % (key, value) for key, value in extra_env.items()] +
                        [shlex.join(cmd)]
                    )
                ]

            self._Execute(cmd, title=title)

    def _build(self):
        self._Execute(['ninja', '-C', self.output_dir],
                      title='Building ' + self.identifier)

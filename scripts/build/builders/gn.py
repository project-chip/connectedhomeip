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

    def __init__(self, root, runner):
        """Creates  a generic ninja builder.

        Args:
           root: the root where to run GN into
           runner: what to use to execute shell commands
        """
        super(GnBuilder, self).__init__(root, runner)

        self.build_command = None
        self._output_dir = None

    @property
    def output_dir(self):
        """
        In a unified build, the output directory is based on the unified_id, otherwise
        it is based on the builder identifier.
        """
        if self._output_dir is not None:
            return self._output_dir
        if self.unified_id:
            return os.path.join(self.root, 'out', self.unified_id)
        return os.path.join(self.root, 'out', self.identifier)

    @output_dir.setter
    def output_dir(self, value):
        self._output_dir = value

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

    def PreBuildCommand(self):
        """Extra steps to run before 'build'"""
        pass

    def PostBuildCommand(self):
        """Extra steps to run after 'build'"""
        pass

    def generate(self, dedup=False):
        cmd = [
            'gn', 'gen', '--check', '--fail-on-unused-args',
            '--add-export-compile-commands=*',
            '--root=%s' % self.root
        ]

        extra_args = []

        if self.options.pw_command_launcher:
            extra_args.append('pw_command_launcher="%s"' % self.options.pw_command_launcher)

        if self.options.enable_link_map_file:
            extra_args.append('chip_generate_link_map_file=true')

        if self.options.pregen_dir:
            extra_args.append('chip_code_pre_generated_directory="%s"' % self.options.pregen_dir)

        extra_args.extend(self.GnBuildArgs() or [])
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

        self._Execute(cmd, title=title, dedup=dedup)

    def _build(self):
        cmd = ['ninja', '-C', self.output_dir]

        if self.verbose:
            cmd.append('-v')

        if self.ninja_jobs:
            cmd.extend(['-j', str(self.ninja_jobs)])

        if getattr(self, 'unified', False):
            # For unified builds, we require building all targets, not just a
            # specific app. As such the build command is not used.
            pass
        elif self.build_command:
            cmd.append(self.build_command)

        title = 'Building ' + self.identifier
        if self.build_command:
            title += ' (target: %s)' % self.build_command
        self._Execute(cmd, title=title)

        self.PostBuildCommand()

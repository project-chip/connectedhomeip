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

import shlex

from .builder import Builder, BuildProfile


class GnBuilder(Builder):

    def __init__(self, root, runner):
        """Creates  a generic ninja builder.

        Args:
           root: the root where to run GN into
           runner: what to use to execute shell commands
        """
        super(GnBuilder, self).__init__(root, runner)

        self.build_command = None

    def GnBuildArgs(self) -> list[str]:
        """Argument list passed to GN `--args` option."""
        args = []
        match self.options.build_profile:
            case BuildProfile.DEFAULT:
                pass  # Profile defined by the build system, no overrides.
            case BuildProfile.DEBUG:
                args.extend(["is_debug=true", "optimize_debug=false"])
            case BuildProfile.DEBUG_OPTIMIZED:
                args.extend(["is_debug=true", "optimize_debug=true"])
            case BuildProfile.RELEASE:
                args.extend(["is_debug=false", "optimize_for_size=false"])
            case BuildProfile.RELEASE_SIZE:
                args.extend(["is_debug=false", "optimize_for_size=true"])
        if self.options.pw_command_launcher:
            args.append(f'pw_command_launcher="{self.options.pw_command_launcher}"')
        if self.options.enable_link_map_file:
            args.append('chip_generate_link_map_file=true')
        if self.options.pregen_dir:
            args.append(f'chip_code_pre_generated_directory="{self.options.pregen_dir}"')
        return args

    def GnBuildEnv(self) -> dict[str, str]:
        """Extra environment variables needed for the GN build to run."""
        return {}

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
            '--root=%s' % self.root,
        ]

        if self.quiet:
            cmd.append("--ninja-extra-args=--quiet")

        if args := self.GnBuildArgs():
            cmd += ['--args=%s' % ' '.join(args)]

        cmd += [self.output_dir]

        if env := self.GnBuildEnv():
            # convert the command into a bash command that includes
            # setting environment variables
            cmd = [
                'bash', '-c', '\n' + ' '.join(
                    ['%s="%s" \\\n' % (key, value) for key, value in env.items()] +
                    [shlex.join(cmd)]
                )
            ]

        self._Execute(cmd, title=f"Generating {self.identifier}", dedup=dedup)

    def _build(self):
        self.PreBuildCommand()

        cmd = ['ninja', '-C', self.output_dir]
        if self.verbose:
            cmd.append('-v')
        if self.quiet:
            cmd.append('--quiet')
        if self.ninja_jobs is not None:
            cmd.append('-j' + str(self.ninja_jobs))
        if self.build_command:
            cmd.append(self.build_command)

        if env := self.GnBuildEnv():
            # convert the command into a bash command that includes
            # setting environment variables
            cmd = [
                'bash', '-c', '\n' + ' '.join(
                    ['%s="%s" \\\n' % (key, value) for key, value in env.items()] +
                    [shlex.join(cmd)]
                )
            ]

        self._Execute(cmd, title=f"Building {self.identifier}")

        self.PostBuildCommand()

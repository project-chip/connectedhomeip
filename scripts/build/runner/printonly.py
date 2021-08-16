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


class PrintOnlyRunner:
    def __init__(self, output_file):
        self.output_file = output_file
        self.dry_run = True

    def Run(self, cmd, cwd=None, title=None):
        if title:
            self.output_file.write("# " + title + "\n")

        if cwd:
            self.output_file.write('cd "%s"\n' % cwd)

        self.output_file.write(
            " ".join([shlex.quote(part) for part in cmd]) + "\n")

        if cwd:
            self.output_file.write("cd -\n")

        self.output_file.write("\n")

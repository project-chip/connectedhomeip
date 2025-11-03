# Copyright (c) 2025 Project CHIP Authors
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


class CommandDedup:
    def __init__(self):
        self.commands = set()

    def is_duplicate(self, cmd: list) -> bool:
        s = " ".join([shlex.quote(part) for part in cmd])
        if s in self.commands:
            return True
        self.commands.add(s)
        return False

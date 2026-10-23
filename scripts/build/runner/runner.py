# Copyright (c) 2026 Project CHIP Authors
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

from abc import ABC, abstractmethod


class Runner(ABC):
    """Generic runner base class."""

    @abstractmethod
    def StartCommandExecution(self):
        """Perform any setup needed before executing commands."""
        raise NotImplementedError

    @abstractmethod
    def Run(self, cmd: list[str], title: str | None = None, dedup: bool = False, quiet: bool = False):
        """Execute the given command.

        Args:
            cmd: the command to execute, as a list of strings (e.g. ['ninja', '-C', 'out/'])
            title: an optional title to log before executing the command
            dedup: whether to attempt to deduplicate this command with previous ones (if supported by the runner)
            quiet: whether to suppress logging output from this command (if supported by the runner)
        """
        raise NotImplementedError

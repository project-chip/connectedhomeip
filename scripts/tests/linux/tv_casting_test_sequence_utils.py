#!/usr/bin/env -S python3 -B

# Copyright (c) 2024 Project CHIP Authors
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

from enum import Enum
from typing import List, Optional

"""
This file defines the utility classes for creating and managing test sequences to validate the casting experience between 
the Linux tv-casting-app and the Linux tv-app. It includes an enumeration for the supported applications (App), a class to 
represent individual steps in a test sequence (Step), and a class to represent a complete test sequence (Sequence). 
Additionally, it provides helper functions to retrieve specific test sequences or all defined test sequences.
"""


class App(Enum):
    """An enumeration of the supported applications."""

    TV_APP = 'tv-app'
    TV_CASTING_APP = 'tv-casting-app'


class Step:
    """A class to represent a step in a test sequence for validation.

    A `Step` object contains attributes relevant to a test step where each object contains:
    - `app` subprocess to parse for `output_msg` or send `input_cmd`
    - `timeout_sec` specified the timeout duration for parsing the `output_msg` (optional, defaults to DEFAULT_TIMEOUT_SEC)
    - `output_msg` or `input_cmd` (mutually exclusive)

    For output message blocks, define the start line, relevant lines, and the last line. If the last line contains trivial closing 
    characters (e.g., closing brackets, braces, or commas), include the line before it with actual content. For example:
        `Step(subprocess_='tv-casting-app', output_msg=['InvokeResponseMessage =', 'exampleData', 'InteractionModelRevision =', '},'])`

    For input commands, define the command string with placeholders for variables that need to be updated. For example:
        `Step(subprocess_='tv-casting-app', input_cmd='cast request 0\n')`
    """

    # The maximum default time to wait while parsing for output string(s).
    DEFAULT_TIMEOUT_SEC = 10

    def __init__(
        self,
        app: App,
        timeout_sec: Optional[int] = DEFAULT_TIMEOUT_SEC,
        output_msg: Optional[List[str]] = None,
        input_cmd: Optional[str] = None,
    ):
        # Validate that either `output_msg` or `input_cmd` is provided, but not both.
        if output_msg is not None and input_cmd is not None:
            raise ValueError(
                'Step cannot contain both `output_msg` and `input_cmd`. Either `output_msg` or `input_cmd` should be provided.')
        elif output_msg is None and input_cmd is None:
            raise ValueError('Step must contain either `output_msg` or `input_cmd`. Both are `None`.')

        # Define either `App.TV_APP` or `App.TV_CASTING_APP` on which we need to parse for `output_msg` or send `input_cmd`.
        self.app = app

        # Define the maximum time in seconds for timeout while parsing for the `output_msg`. If not provided, then we use the DEFAULT_TIMEOUT_SEC.
        self.timeout_sec = timeout_sec

        # Define the `output_msg` that we need to parse for in a list format.
        self.output_msg = output_msg

        # Define the `input_cmd` that we need to send to either the `App.TV_APP` or `App.TV_CASTING_APP`.
        self.input_cmd = input_cmd


class Sequence:
    """A class representing a sequence of steps for testing the casting experience between the Linux tv-casting-app and the tv-app.

    A Sequence object needs to be defined with an appropriate test sequence `name` along with its list of `Step` objects that will
    be used for validating the casting experience.
    """

    def __init__(self, name: str, steps: List[Step]):
        self.name = name
        self.steps = steps

    @staticmethod
    def get_test_sequence_by_name(test_sequences: List['Sequence'], test_sequence_name: str) -> Optional['Sequence']:
        """Retrieve a test sequence from a list of sequences by its name."""

        for sequence in test_sequences:
            if sequence.name == test_sequence_name:
                return sequence
        return None

    @staticmethod
    def get_test_sequences() -> List['Sequence']:
        """Retrieve all the test sequences to validate the casting experience between the Linux tv-casting-app and the Linux tv-app."""

        from linux.tv_casting_test_sequences import test_sequences

        return test_sequences

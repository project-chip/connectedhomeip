# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import logging
import os
from typing import Optional

LOGGER = logging.getLogger(__name__)


class GeneratorStorage:
    """
    Handles file operations for generator output. Specifically can create
    required files for output.

    Is overridden for unit tests.
    """

    def __init__(self):
        self._generated_paths = set()

    @property
    def generated_paths(self):
        return self._generated_paths

    def report_output_file(self, relative_path: str):
        self._generated_paths.add(relative_path)

    def get_existing_data(self, relative_path: str):
        """Gets the existing data at the given path.
        If such data does not exist, will return None.
        """
        raise NotImplementedError()

    def write_new_data(self, relative_path: str, content: str):
        """Write new data to the given path."""
        raise NotImplementedError()


class FileSystemGeneratorStorage(GeneratorStorage):
    """
    A storage generator which will physically write files to disk into
    a given output folder.
    """

    def __init__(self, output_dir: str):
        super().__init__()
        self.output_dir = output_dir

    def get_existing_data(self, relative_path: str):
        """Gets the existing data at the given path.
        If such data does not exist, will return None.
        """
        target = os.path.join(self.output_dir, relative_path)

        if not os.path.exists(target):
            return None

        LOGGER.info("Checking existing data in %s" % target)
        with open(target, 'rt') as existing:
            return existing.read()

    def write_new_data(self, relative_path: str, content: str):
        """Write new data to the given path."""

        target = os.path.join(self.output_dir, relative_path)
        target_dir = os.path.dirname(target)
        if not os.path.exists(target_dir):
            LOGGER.info("Creating output directory: %s" % target_dir)
            os.makedirs(target_dir)

        LOGGER.info("Writing new data to: %s" % target)
        with open(target, "wt") as out:
            out.write(content)


class InMemoryStorage(GeneratorStorage):
    """A storage generator which will keep files in memory."""

    def __init__(self):
        super().__init__()
        self.content: Optional[str] = None

    def get_existing_data(self, relative_path: str):
        # Force re-generation each time
        return None

    def write_new_data(self, relative_path: str, content: str):
        if self.content:
            raise Exception("Unexpected extra data: single file generation expected")
        self.content = content

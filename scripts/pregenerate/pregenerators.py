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
import shlex
import subprocess

from .types import InputIdlFile, IdlFileType

CODEGEN_PY_PATH = os.path.join(os.path.dirname(__file__), '..', 'codegen.py')


class CodegenTarget:
    """A target that uses `scripts/codegen.py` to generate files."""

    def __init__(self, idl: InputIdlFile, generator: str, sdk_root: str):
        self.idl = idl
        self.generator = generator
        self.sdk_root = sdk_root

        if idl.file_type != IdlFileType.MATTER:
            raise Exception(f"Can only code generate for `*.matter` input files, not for {idl}")

    def Generate(self, output_root: str):
        '''Runs codegen.py to generate in the specified directory'''

        output_dir = os.path.join(output_root, self.idl.pregen_subdir, self.generator)

        logging.info(f"Generating: {self.generator}:{self.idl.relative_path} into {output_dir}")

        cmd = [
            CODEGEN_PY_PATH,
            '--log-level', 'fatal',
            '--generator', self.generator,
            '--output-dir', output_dir,
            os.path.join(self.sdk_root, self.idl.relative_path)
        ]

        logging.debug(f"Executing {cmd}")
        subprocess.check_call(cmd)


class CodegenBridgePregenerator:
    """Pregeneration logic for "bridge" codegen.py outputs"""

    def __init__(self, sdk_root):
        self.sdk_root = sdk_root

    def Accept(self, idl: InputIdlFile):
        # Bridge is highly specific, a single path is acceptable for dynamic
        # bridge codegen
        return idl.relative_path == "examples/dynamic-bridge-app/bridge-common/bridge-app.matter"

    def CreateTarget(self, idl: InputIdlFile):
        return CodegenTarget(sdk_root=self.sdk_root, idl=idl, generator="bridge")


class CodegenJavaPregenerator:
    """Pregeneration logic for "java" codegen.py outputs"""

    def __init__(self, sdk_root):
        self.sdk_root = sdk_root

    def Accept(self, idl: InputIdlFile):
        # Java is highly specific, a single path is acceptable for dynamic
        # bridge codegen
        return idl.relative_path == "src/controller/data_model/controller-clusters.matter"

    def CreateTarget(self, idl: InputIdlFile):
        return CodegenTarget(sdk_root=self.sdk_root, idl=idl, generator="java")


class CodegenCppAppPregenerator:
    """Pregeneration logic for "cpp-app" codegen.py outputs"""

    def __init__(self, sdk_root):
        self.sdk_root = sdk_root

    def Accept(self, idl: InputIdlFile):
        if idl.file_type != IdlFileType.MATTER:
            return False

        # we should not be checked for these, but verify just in case
        if '/tests/' in idl.relative_path:
            return False

        return True

    def CreateTarget(self, idl: InputIdlFile):
        return CodegenTarget(sdk_root=self.sdk_root, idl=idl, generator="cpp-app")

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
from enum import Enum, auto

from .type_definitions import IdlFileType, InputIdlFile

ZAP_GENERATE_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'tools', 'zap', 'generate.py'))


class ZapGeneratorType(Enum):
    APPLICATION_TEMPLATES = auto()

    @property
    def generation_template(self):
        if self == ZapGeneratorType.APPLICATION_TEMPLATES:
            return 'src/app/zap-templates/app-templates.json'
        else:
            raise Exception("Missing ZAP Generation type implementation")

    @property
    def subdir(self):
        if self == ZapGeneratorType.APPLICATION_TEMPLATES:
            return 'app-templates/zap-generated'
        else:
            raise Exception("Missing ZAP Generation type implementation")


class ZapTarget:
    def __init__(self, idl: InputIdlFile, generation_type: ZapGeneratorType, sdk_root: str, runner):
        self.idl = idl
        self.sdk_root = sdk_root
        self.generation_type = generation_type
        self.runner = runner

        if idl.file_type != IdlFileType.ZAP:
            raise Exception(f"Can only code generate for `*.zap` input files, not for {idl}")

    def Generate(self, output_root: str):
        '''Runs generate.py to generate in the specified directory root'''

        output_dir = os.path.join(output_root, self.idl.pregen_subdir, self.generation_type.subdir)

        logging.info(f"Generating: {self.generation_type}:{self.idl.full_path} into {output_dir}")

        self.runner.ensure_directory_exists(output_dir)

        if self.idl.full_path.startswith(self.sdk_root):
            idl_path = self.idl.relative_path
        else:
            idl_path = self.idl.full_path

        cmd = [
            ZAP_GENERATE_PATH,
            '--templates', self.generation_type.generation_template,
            '--output-dir', output_dir,
            '--parallel',
            idl_path
        ]
        logging.debug(f"Executing {cmd}")
        self.runner.run(cmd, cwd=self.sdk_root)


class ZapApplicationPregenerator:
    """Pregeneration logic for `src/app/zap-templates/app-templates.json` """

    def __init__(self, sdk_root):
        self.sdk_root = sdk_root

    def Accept(self, idl: InputIdlFile):
        if idl.file_type != IdlFileType.ZAP:
            return False

        # FIXME: implement a proper check
        if 'test_files' in idl.relative_path:
            return False
        return True

    def CreateTarget(self, idl: InputIdlFile, runner):
        # TODO: add additional arguments: tell how to invoke zap/codegen
        return ZapTarget(sdk_root=self.sdk_root, generation_type=ZapGeneratorType.APPLICATION_TEMPLATES, idl=idl, runner=runner)

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

from typing import Iterator

from .types import InputIdlFile, IdlFileType


from .pregenerators import CodegenJavaPregenerator
from .pregenerators import CodegenBridgePregenerator
from .pregenerators import CodegenCppAppPregenerator


def FindAllIdls(sdk_root: str) -> Iterator[InputIdlFile]:
    relevant_subdirs = [
        'examples',  # all example apps
        'src',      # realistically only controller/data_model
    ]

    while sdk_root.endswith('/'):
        sdk_root = sdk_root[:-1]
    sdk_root_length = len(sdk_root)

    for subdir_name in relevant_subdirs:
        top_directory_name = os.path.join(sdk_root, subdir_name)
        logging.debug(f"Searching {top_directory_name}")
        for root, dirs, files in os.walk(top_directory_name):
            for file in files:
                if file.endswith('.zap'):
                    yield InputIdlFile(file_type=IdlFileType.ZAP,
                                       relative_path=os.path.join(root[sdk_root_length+1:], file))
                if file.endswith('.matter'):
                    yield InputIdlFile(file_type=IdlFileType.MATTER,
                                       relative_path=os.path.join(root[sdk_root_length+1:], file))


def FindPregenerationTargets(sdk_root: str):
    """Finds all relevand pre-generation targets in the given
       SDK root.

       Pre-generation targets are based on zap and matter files with options
       on what rules to pregenerate and how.
    """

    generators = [
        CodegenBridgePregenerator(sdk_root),
        CodegenJavaPregenerator(sdk_root),
        CodegenCppAppPregenerator(sdk_root),
    ]

    for idl in FindAllIdls(sdk_root):
        for generator in generators:
            if generator.Accept(idl):
                yield generator.CreateTarget(idl)

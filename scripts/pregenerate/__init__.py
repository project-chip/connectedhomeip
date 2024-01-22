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


import fnmatch
import logging
import os
from dataclasses import dataclass, field
from typing import Iterator, List, Optional

from .type_definitions import IdlFileType, InputIdlFile
from .using_codegen import (CodegenCppAppPregenerator, CodegenCppClustersTLVMetaPregenerator,
                            CodegenCppProtocolsTLVMetaPregenerator, CodegenJavaClassPregenerator, CodegenJavaJNIPregenerator)
from .using_zap import ZapApplicationPregenerator


def _IdlsInDirectory(top_directory_name: str, truncate_length: int):
    for root, dirs, files in os.walk(top_directory_name):
        for file in files:
            if file.endswith('.zap'):
                yield InputIdlFile(file_type=IdlFileType.ZAP,
                                   full_path=os.path.join(root, file),
                                   relative_path=os.path.join(root[truncate_length:], file))
            if file.endswith('.matter'):
                yield InputIdlFile(file_type=IdlFileType.MATTER,
                                   full_path=os.path.join(root, file),
                                   relative_path=os.path.join(root[truncate_length:], file))


def _FindAllIdls(sdk_root: str, external_roots: Optional[List[str]]) -> Iterator[InputIdlFile]:
    relevant_subdirs = [
        'examples',  # all example apps
        'src',      # realistically only controller/data_model
    ]

    while sdk_root.endswith('/'):
        sdk_root = sdk_root[:-1]
    sdk_root_length = len(sdk_root)

    # first go over SDK items
    for subdir_name in relevant_subdirs:
        top_directory_name = os.path.join(sdk_root, subdir_name)
        logging.debug(f"Searching {top_directory_name}")
        for idl in _IdlsInDirectory(top_directory_name, sdk_root_length+1):
            yield idl

    # next external roots
    if external_roots:
        for root in external_roots:
            root = os.path.normpath(root)
            for idl in _IdlsInDirectory(root, len(root) + 1):
                yield idl


@dataclass
class TargetFilter:
    # If set, only the specified files are accepted for codegen
    file_type: Optional[IdlFileType] = None

    # If non-empty only the given paths will be code-generated
    path_glob: List[str] = field(default_factory=list)


# TODO: the build GlobMatcher is more complete by supporting `{}` grouping
#       For now this limited glob seems sufficient.
class GlobMatcher:
    def __init__(self, pattern: str):
        self.pattern = pattern

    def matches(self, s: str):
        return fnmatch.fnmatch(s, self.pattern)


def FindPregenerationTargets(sdk_root: str, external_roots: Optional[List[str]], filter: TargetFilter, runner):
    """Finds all relevand pre-generation targets in the given
       SDK root.

       Pre-generation targets are based on zap and matter files with options
       on what rules to pregenerate and how.
    """

    generators = [
        # Jinja-based codegen
        CodegenJavaJNIPregenerator(sdk_root),
        CodegenJavaClassPregenerator(sdk_root),
        CodegenCppAppPregenerator(sdk_root),
        CodegenCppClustersTLVMetaPregenerator(sdk_root),
        CodegenCppProtocolsTLVMetaPregenerator(sdk_root),

        # ZAP codegen
        ZapApplicationPregenerator(sdk_root),
    ]

    path_matchers = [GlobMatcher(pattern) for pattern in filter.path_glob]

    for idl in _FindAllIdls(sdk_root, external_roots):
        if filter.file_type is not None:
            if idl.file_type != filter.file_type:
                logging.debug(f"Will not process file of type {idl.file_type}: {idl.relative_path}")
                continue

        if path_matchers:
            if all([not matcher.matches(idl.relative_path) for matcher in path_matchers]):
                logging.debug(f"Glob not matched for {idl.relative_path}")
                continue

        for generator in generators:
            if generator.Accept(idl):
                yield generator.CreateTarget(idl, runner=runner)

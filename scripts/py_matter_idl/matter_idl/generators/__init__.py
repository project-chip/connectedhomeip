#!/usr/bin/env python
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
from typing import Dict, Optional

import jinja2
from matter_idl.matter_idl_types import Idl

from .filters import RegisterCommonFilters


class GeneratorStorage:
    """
    Handles file operations for generator output. Specifically can create
    required files for output.

    Is overriden for unit tests.
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

        logging.info("Checking existing data in %s" % target)
        with open(target, 'rt') as existing:
            return existing.read()

    def write_new_data(self, relative_path: str, content: str):
        """Write new data to the given path."""

        target = os.path.join(self.output_dir, relative_path)
        target_dir = os.path.dirname(target)
        if not os.path.exists(target_dir):
            logging.info("Creating output directory: %s" % target_dir)
            os.makedirs(target_dir)

        logging.info("Writing new data to: %s" % target)
        with open(target, "wt") as out:
            out.write(content)


class CodeGenerator:
    """
    Defines the general interface for things that can generate code output.

    A CodeGenerator takes a AST as input (a `Idl` type) and generates files
    as output (like java/cpp/mm/other).

    Its public interface surface is reasonably small:
      'storage' init argument specifies where generated code goes
      'idl' is the input AST to generate
      'render' will perform a rendering of all files.

    As special optimizations, CodeGenerators generally will try to read
    existing data and will not re-write content if not changed (so that
    write time of files do not change and rebuilds are not triggered).
    """

    def __init__(self, storage: GeneratorStorage, idl: Idl, loader: Optional[jinja2.BaseLoader] = None, fs_loader_searchpath: Optional[str] = None):
        """
        A code generator will render a parsed IDL (a AST) into a given storage.

        Args:
           storage: Storage to use to read/save data
           loader: if given, use a custom loader for templates
           fs_loader_searchpath: if a loader is NOT given, this controls the search path
              of a default FileSystemLoader that will be used
        """
        if not loader:
            if not fs_loader_searchpath:
                fs_loader_searchpath = os.path.dirname(__file__)
            loader = jinja2.FileSystemLoader(searchpath=fs_loader_searchpath)

        self.storage = storage
        self.idl = idl
        self.jinja_env = jinja2.Environment(
            loader=loader, keep_trailing_newline=True)
        self.dry_run = False

        RegisterCommonFilters(self.jinja_env.filters)

    def render(self, dry_run=False):
        """
        Renders  all required files given the idl contained in the code generator.
        Reset the list of generated outputs.

        Args:
          dry_run: if true, outputs are not actually written to disk.
                   if false, outputs are actually written to disk.
        """
        self.dry_run = dry_run
        self.internal_render_all()

    def internal_render_all(self):
        """This method is to be implemented by subclasses to run all generation
           as needed.
        """
        raise NotImplementedError("Method should be implemented by subclasses")

    def internal_render_one_output(self, template_path: str, output_file_name: str, vars: Dict):
        """
        Method to be called by subclasses to mark that a template is to be generated.

        File will either actually do a jinja2 generation or just log things
        if dry-run was requested during `render`.

        NOTE: to make this method suitable for rebuilds, this file will NOT alter
              the timestamp of the output file if the file content would not
              change (i.e. no write will be invoked in that case.)

        Args:
          template_path - the path to the template to be loaded for file generation.
                          Template MUST be a jinja2 template.
          output_file_name - File name that the template is to be generated to.
          vars             - variables used for template generation
        """
        logging.info("File to be generated: %s" % output_file_name)
        if self.dry_run:
            return

        rendered = self.jinja_env.get_template(template_path).render(vars)

        # Report regardless if it has changed or not. This is because even if
        # files are unchanged, validation of what the correct output is should
        # still be done.
        self.storage.report_output_file(output_file_name)

        if rendered == self.storage.get_existing_data(output_file_name):
            logging.info("File content not changed")
        else:
            self.storage.write_new_data(output_file_name, rendered)

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

import os
import logging
import jinja2

from typing import Dict
from idl.matter_idl_types import Idl

from .filters import RegisterCommonFilters


class GeneratorStorage:
    """
    Handles file operations for generator output. Specificall can create
    required files for output.

    Is overriden for unit tests.
    """

    def get_existing_data(self, relative_path: str):
        """Gets the existing data at the given path.
        If such data does not exist, will return None.
        """
        raise NotImplementedError()

    def write_new_data(self, relative_path: str, content: str):
        """Write new data to the given path."""
        raise NotImplementedError()


class FileSystemGeneratorStorage(GeneratorStorage):
    def __init__(self, output_dir: str):
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
    Defines the general interface for things that can
    generate code output.
    """

    def __init__(self, storage: GeneratorStorage, idl: Idl):
        self.storage = storage
        self.idl = idl
        self.jinja_env = jinja2.Environment(
            loader=jinja2.FileSystemLoader(searchpath=os.path.dirname(__file__)))
        self.dry_run = False
        self.output_file_names = []

        RegisterCommonFilters(self.jinja_env.filters)

    def render(self, dry_run=False):
        """
        Renders  all required files given the idl contained in the code generator.
        Reset the list of generated outputs.

        Args:
          dry_run: if true, outputs are not actually written to disk.
                   if false, outputs are actually written to disk.
        """
        self.output_file_names = []
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

        File will either actually do a jinja2 generation or, if only output  file
        names are desired, will only record the output.

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
        self.output_file_names.append(output_file_name)
        if self.dry_run:
            return

        rendered = self.jinja_env.get_template(template_path).render(vars)

        if rendered == self.storage.get_existing_data(output_file_name):
            logging.info("File content not changed")
        else:
            self.storage.write_new_data(output_file_name, rendered)

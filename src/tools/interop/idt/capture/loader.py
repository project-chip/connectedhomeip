#
#    Copyright (c) 2023 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import importlib
import inspect
import os
import traceback
from typing import Any

from utils import log

logger = log.get_logger(__file__)


class CaptureImplsLoader:

    def __init__(self, root_dir: str, root_package: str, search_type: type):
        self.logger = logger
        self.root_dir = root_dir
        self.root_package = root_package
        self.search_type = search_type
        self.impl_names = []
        self.impls = {}
        self.fetch_impls()

    @staticmethod
    def is_package(potential_package: str) -> bool:
        init_path = os.path.join(potential_package,
                                 "__init__.py")
        return os.path.exists(init_path)

    def verify_coroutines(self, subclass) -> bool:
        # ABC does not verify coroutines on subclass instantiation, it merely checks the presence of methods
        for item in dir(self.search_type):
            item_attr = getattr(self.search_type, item)
            if inspect.iscoroutinefunction(item_attr):
                if not hasattr(subclass, item):
                    self.logger.warning(f"Missing coroutine in {subclass}")
                    return False
                if not inspect.iscoroutinefunction(getattr(subclass, item)):
                    self.logger.warning(f"Missing coroutine in {subclass}")
                    return False
        for item in dir(subclass):
            item_attr = getattr(subclass, item)
            if inspect.iscoroutinefunction(item_attr) and hasattr(self.search_type, item):
                if not inspect.iscoroutinefunction(getattr(self.search_type, item)):
                    self.logger.warning(f"Unexpected coroutine in {subclass}")
                    return False
        return True

    def is_type_match(self, potential_class_match: Any) -> bool:
        if inspect.isclass(potential_class_match):
            self.logger.debug(f"Checking {self.search_type} match against {potential_class_match}")
            if issubclass(potential_class_match, self.search_type):
                self.logger.debug(f"Found type match search: {self.search_type} match: {potential_class_match}")
                if self.verify_coroutines(potential_class_match):
                    return True
        return False

    def load_module(self, to_load):
        self.logger.debug(f"Loading module {to_load}")
        saw_more_than_one_impl = False
        saw_one_impl = False
        found_class = None
        for module_item in dir(to_load):
            loaded_item = getattr(to_load, module_item)
            if self.is_type_match(loaded_item):
                found_class = module_item
                found_impl = loaded_item
                if not saw_one_impl:
                    saw_one_impl = True
                else:
                    saw_more_than_one_impl = True
        if saw_one_impl and not saw_more_than_one_impl:
            self.impl_names.append(found_class)
            self.impls[found_class] = found_impl
        elif saw_more_than_one_impl:
            self.logger.warning(f"more than one impl in {module_item}")

    def fetch_impls(self):
        self.logger.debug(f"Searching for implementations in {self.root_dir}")
        for item in os.listdir(self.root_dir):
            dir_content = os.path.join(self.root_dir, item)
            if self.is_package(dir_content):
                self.logger.debug(f"Found package in {dir_content}")
                try:
                    module = importlib.import_module("." + item, self.root_package)
                    self.load_module(module)
                except ModuleNotFoundError:
                    self.logger.warning(f"No module matching package name for {item}\n{traceback.format_exc()}")

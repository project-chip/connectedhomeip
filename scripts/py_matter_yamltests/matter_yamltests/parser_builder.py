#
#    Copyright (c) 2023 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the 'License');
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an 'AS IS' BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

import copy
import time
from dataclasses import dataclass, field
from typing import List

from .hooks import TestParserHooks
from .parser import TestParser, TestParserConfig


@dataclass
class TestParserBuilderOptions:
    """
    TestParserBuilderOptions allows configuring the behavior of a
    TestParserBuilder instance.

    stop_on_error: If set to False the parser will continue parsing
                   the next test instead of aborting if an error is
                   encountered while parsing a particular test file.
    """
    stop_on_error: bool = True


@dataclass
class TestParserBuilderConfig:
    """
    TestParserBuilderConfig defines a set of common properties that will be used
    by a TestParserBuilder instance for parsing the given list of tests.

    tests: A list of YAML tests to be parsed.

    parser_config: A common configuration for the tests to be parsed.

    options: A common set of options for the tests to be parsed.

    hooks: A configurable set of hooks to be called at various steps during
           parsing. It may may allow the callers to gain insights about the
           current parsing state.
    """
    tests: List[str] = field(default_factory=list)
    parser_config: TestParserConfig = field(default_factory=TestParserConfig)
    hooks: TestParserHooks = TestParserHooks()
    options: TestParserBuilderOptions = field(
        default_factory=TestParserBuilderOptions)


class TestParserBuilder:
    """
    TestParserBuilder is an iterator over a set of tests using a common configuration.
    """

    def __init__(self, config: TestParserBuilderConfig = TestParserBuilderConfig()):
        self.__tests = copy.copy(config.tests)
        self.__config = config
        self.__duration = 0
        self.done = False

    def __iter__(self):
        self.__config.hooks.parsing_start(len(self.__tests))
        return self

    def __next__(self):
        if len(self.__tests):
            return self.__get_test_parser(self.__tests.pop(0))

        if not self.done:
            self.__config.hooks.parsing_stop(round(self.__duration))
        self.done = True

        raise StopIteration

    def __get_test_parser(self, test_file: str) -> TestParser:
        start = time.time()

        parser = None
        exception = None
        try:
            self.__config.hooks.test_parsing_start(test_file)
            parser = TestParser(test_file, self.__config.parser_config)
        except Exception as e:
            exception = e

        duration = round((time.time() - start) * 1000, 0)
        self.__duration += duration
        if exception:
            self.__config.hooks.test_parsing_failure(exception, duration)
            if self.__config.options.stop_on_error:
                raise StopIteration
            return None

        self.__config.hooks.test_parsing_success(duration)
        return parser

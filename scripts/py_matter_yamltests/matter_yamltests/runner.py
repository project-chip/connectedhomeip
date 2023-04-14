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

import asyncio
import time
from abc import ABC, abstractmethod
from dataclasses import dataclass

from .adapter import TestAdapter
from .hooks import TestRunnerHooks
from .parser import TestParser
from .parser_builder import TestParserBuilder, TestParserBuilderConfig
from .pseudo_clusters.pseudo_clusters import PseudoClusters


@dataclass
class TestRunnerOptions:
    """
    TestRunnerOptions allows configuring the behavior of a TestRunner
    instance.

    stop_on_error: If set to False the runner will continue executing
                   the next test step instead of aborting if an error
                   is encountered while running a particular test file.

    stop_on_warning: If set to true the runner will stop running the
                     tests if a warning is encountered while running
                     a particular test file.

    stop_at_number: If set to any non negative number, the runner will
                    stop running the tests if a step index matches
                    the number. This is mostly useful when running
                    a single test file and for debugging purposes.
    """
    stop_on_error: bool = True
    stop_on_warning: bool = False
    stop_at_number: int = -1


@dataclass
class TestRunnerConfig:
    """
    TestRunnerConfig defines  a set of common properties that will be used
    by a TestRunner instance for running the given set of tests.

    adapter: An adapter to use to translate from the matter_yamltests format
             to the target format.

    pseudo_clusters: The pseudo clusters to use while running tests.

    options: A common set of options for the tests to be runned.

    hooks: A configurable set of hooks to be called at various steps while
           running. It may may allow the callers to gain insights about the
           current running state.
    """
    adapter: TestAdapter = None
    pseudo_clusters: PseudoClusters = PseudoClusters([])
    options: TestRunnerOptions = TestRunnerOptions()
    hooks: TestRunnerHooks = TestRunnerHooks()


class TestRunnerBase(ABC):
    """
    TestRunnerBase is an abstract interface that defines the set of methods a runner
    should implement.

    A runner is responsible for executing a test step.
    """
    @abstractmethod
    async def start(self):
        """
        This method is called before running the steps of a particular test file.
        It may allow the runner to perform some setup tasks.
        """
        pass

    @abstractmethod
    async def stop(self):
        """
        This method is called after running the steps of a particular test file.
        It may allow the runner to perform some cleanup tasks.
        """
        pass

    @abstractmethod
    async def execute(self, request):
        """
        This method executes a request using the adapter format, and returns a response
        using the adapter format.
        """
        pass

    @abstractmethod
    def run(self, config: TestRunnerConfig) -> bool:
        """
        This method runs a test suite.

        Returns
        -------
        bool
            A boolean indicating if the run has succeeded.
        """
        pass


class TestRunner(TestRunnerBase):
    """
    TestRunner is a default runner implementation.
    """
    async def start(self):
        return

    async def execute(self, request):
        return request

    async def stop(self):
        return

    def run(self, parser_builder_config: TestParserBuilderConfig, runner_config: TestRunnerConfig):
        if runner_config and runner_config.hooks:
            start = time.time()
            runner_config.hooks.start(len(parser_builder_config.tests))

        parser_builder = TestParserBuilder(parser_builder_config)
        for parser in parser_builder:
            if not parser or not runner_config:
                continue

            result = asyncio.run(self._run(parser, runner_config))
            if isinstance(result, Exception):
                raise (result)
            elif not result:
                return False

        if runner_config and runner_config.hooks:
            duration = round((time.time() - start) * 1000)
            runner_config.hooks.stop(duration)

        return True

    async def _run(self, parser: TestParser, config: TestRunnerConfig):
        status = True
        try:
            await self.start()

            hooks = config.hooks
            hooks.test_start(parser.name, parser.tests.count)

            test_duration = 0
            for idx, request in enumerate(parser.tests):
                if not request.is_pics_enabled:
                    hooks.step_skipped(request.label)
                    continue
                elif not config.adapter:
                    hooks.step_start(request.label)
                    hooks.step_unknown()
                    continue
                else:
                    hooks.step_start(request.label)

                start = time.time()
                if config.pseudo_clusters.supports(request):
                    responses, logs = await config.pseudo_clusters.execute(request)
                else:
                    encoded_request = config.adapter.encode(request)
                    encoded_response = await self.execute(encoded_request)
                    responses, logs = config.adapter.decode(encoded_response)
                duration = round((time.time() - start) * 1000, 2)
                test_duration += duration

                logger = request.post_process_response(responses)

                if logger.is_failure():
                    hooks.step_failure(logger, logs, duration,
                                       request.responses, responses)
                else:
                    hooks.step_success(logger, logs, duration)

                if logger.is_failure() and config.options.stop_on_error:
                    status = False
                    break

                if logger.warnings and config.options.stop_on_warning:
                    status = False
                    break

                if (idx + 1) == config.options.stop_at_number:
                    break

            hooks.test_stop(round(test_duration))

        except Exception as exception:
            status = exception
        finally:
            await self.stop()
            return status

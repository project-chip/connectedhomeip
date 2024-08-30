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

from typing import Optional

from .parser import TestStep


class TestParserHooks():
    __test__ = False

    def parsing_start(self, count: int):
        """
        This method is called when the parser starts parsing a set of files.

        Parameters
        ----------
        count: int
            The number of files that will be parsed.
        """
        pass

    def parsing_stop(self, duration: int):
        """
        This method is called when the parser is done parsing a set of files.

        Parameters
        ----------
        duration: int
            How long it took to parse the set of files, in milliseconds.
        """
        pass

    def test_parsing_start(self, name: str):
        """
        This method is called when the parser starts parsing a single file.

        Parameters
        ----------
        name: str
            The name of the file that will be parsed.
        """
        pass

    def test_parsing_failure(self, exception: Exception, duration: int):
        """
        This method is called when parsing a single file fails.

        Parameters
        ----------
        exception: Exception
            An exception describing why parsing the file has failed.
        duration: int
            How long it took to parse the file, in milliseconds.
        """
        pass

    def test_parsing_success(self, duration: int):
        """
        This method is called when parsing a single file succeeds.

        Parameters
        ----------
        duration: int
            How long it took to parse the file, in milliseconds.
        """
        pass


class TestRunnerHooks():
    __test__ = False

    def start(self, count: int):
        """
        This method is called when the runner starts running a set of tests.

        Parameters
        ----------
        count: int
            The number of files that will be run.
        """
        pass

    def stop(self, duration: int):
        """
        This method is called when the runner is done running a set of tests.

        Parameters
        ----------
        duration: int
            How long it took to run the set of tests, in milliseconds.
        """
        pass

    def test_start(self, filename: str, name: str, count: int, steps: list[str] = []):
        """
        This method is called when the runner starts running a single test.

        Parameters
        ----------
        filename: str
            The name of the file containing the test that is starting.

        name: str
            The name of the test that is starting.

        count: int
            The number of steps from the test that will be run.

        steps: list[str]
            The computed test step names
        """
        pass

    def test_stop(self, exception: Exception, duration: int):
        """
        This method is called when the runner is done running a single test.

        Parameters
        ----------
        exception: Exception
            An optional exception describing why running the test has failed.

        duration: int
            How long it took to run the test, in milliseconds.
        """
        pass

    def step_skipped(self, name: str, expression: str):
        """
        This method is called when running a step is skipped.

        Parameters
        ----------
        name: str
            The name of the test step that is skipped.

        expression: str
            The PICS expression that results in the test step being skipped.
        """
        pass

    def step_start(self, request: TestStep):
        """
        This method is called when the runner starts running a step from the test.

        Parameters
        ----------
        request: TestStep
            The original request as defined by the test step.
        """
        pass

    def step_success(self, logger, logs, duration: int, request: TestStep):
        """
        This method is called when running a step succeeds.

        Parameters
        ----------
        logger:
            An object containing details about the different checks against the response.

        logs:
            Optional logs from the adapter.

        duration: int
            How long it took to run the test step, in milliseconds.

        request: TestStep
            The original request as defined by the test step.
        """
        pass

    def step_failure(self, logger, logs, duration: int, request: TestStep, received):
        """
        This method is called when running a step fails.

        Parameters
        ----------
        logger:
            An object containing details about the different checks against the response.

        logs:
            Optional logs from the adapter.

        duration: int
            How long it took to run the test step, in milliseconds.

        request: TestStep
            The original request as defined by the test step.

        received:
            The received response.
        """
        pass

    def step_unknown(self):
        """
        This method is called when the result of running a step is unknown. For example during a dry-run.
        """
        pass

    async def step_manual(self):
        """
        This method is called when the step is executed manually.
        """
        pass

    def show_prompt(self,
                    msg: str,
                    placeholder: Optional[str] = None,
                    default_value: Optional[str] = None,
                    endpoint_id: Optional[int] = None,
                    ) -> None:
        """
        This method is called when the step needs to ask the user to perform some action or provide some value.
        """
        pass

    def test_skipped(self, filename: str, name: str):
        """
        This method is called when the test script determines that the test is not applicable for the DUT.
        """
        pass


class WebSocketRunnerHooks():
    def connecting(self, url: str):
        """
        This method is called when the websocket is attempting to connect to a remote.

        Parameters
        ----------
        url: str
            The url the websocket is trying to connect to.
        """
        pass

    def abort(self, url: str):
        """
        This method is called when the websocket connection fails and will not be retried.

        Parameters
        ----------
        url: str
            The url the websocket has failed to connect to.
        """
        pass

    def success(self, duration: int):
        """
        This method is called when the websocket connection is established.

        Parameters
        ----------
        duration: int
            How long it took to connect since the last retry, in milliseconds.
        """
        pass

    def failure(self, duration: int):
        """
        This method is called when the websocket connection fails and will be retried.

        Parameters
        ----------
        duration: int
            How long it took to fail since the last retry, in milliseconds.
        """
        pass

    def retry(self, interval_between_retries_in_seconds: int):
        """
        This method is called when the websocket connection will be retried in the given interval.

        Parameters
        ----------
        interval_between_retries_in_seconds: int
            How long we will wait before retrying to connect, in seconds.
        """
        pass

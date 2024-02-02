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

import asyncio
from dataclasses import dataclass
from logging import Logger
from typing import IO

from utils.log import print_and_write


@dataclass
class TextCause:
    """
    A representation of a possible problem cause in a log
    search_terms: terms to match in a log line (term 1 in line AND term 2 in line AND term N in line...)
    search_attr: analyze this attribute of an analysis object for this cause
    help_message: how to help the user if this cause is discovered
    follow_up_causes: more checks to continue narrowing down the root cause (depth first)
    """
    search_terms: [str]
    search_attr: [str]
    help_message: str
    follow_up_causes: []  # : [Cause] (Recursive reference does not work for type hint here)


class PrescriptiveTextAnalysis:

    def __init__(self, causes: [TextCause], output_file_name: str, logger: Logger):
        """
        An "analysis object" is an object which subclasses this class
        and contains log lines collected in instance variables
        """
        self.causes = causes
        self.output_file_name = output_file_name
        self.logger = logger

    def check_cause(self, cause: TextCause) -> None:
        if not hasattr(self, cause.search_attr):
            self.logger.error(f"{cause.search_attr} not found on this analysis object!")
            return
        to_search = getattr(self, cause.search_attr)
        found = True
        for search_term in cause.search_terms:
            found = found and search_term in to_search
        if found:
            with open(self.output_file_name, mode="w+") as analysis_file:
                print_and_write(cause.help_message, analysis_file, important=True)
        if found and cause.follow_up_causes:
            for follow_up in cause.follow_up_causes:
                self.check_cause(follow_up)

    def check_causes(self) -> None:
        for cause in self.causes:
            self.check_cause(cause)


class TextAnalysisObserver:
    """
    Subclasses should implement functions starting with _log to process lines into instance vars of any name
    """

    def __init__(self, log: str, logger: Logger):
        self.log = log
        self.logger = logger
        self.log_file: IO | None = None

    def process_line(self, line: str) -> None:
        for line_func in [s for s in dir(self) if s.startswith('_log')]:
            getattr(self, line_func)(line)

    def do_analysis(self, batch: [str]) -> None:
        for line in batch:
            self.process_line(line)

    async def analyze_capture(self) -> None:
        try:
            self.log_file = open(self.log, "r")
            while True:
                self.do_analysis(self.log_file.readlines())
                await asyncio.sleep(0.5)  # Releasing async event loop for other tasks
        except asyncio.CancelledError:
            self.logger.info(f"Closing log stream {self.log}")
            if self.log_file:
                self.log_file.close()

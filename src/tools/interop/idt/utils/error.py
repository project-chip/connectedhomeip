#
#    Copyright (c) 2024 Project CHIP Authors
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

import traceback
from typing import Dict, List
from dataclasses import dataclass

from utils.artifact import create_standard_log_name
from utils.log import get_logger, print_and_write, add_border

logger = get_logger(__file__)


@dataclass
class ErrorRecord:
    help_message: str
    stack_trace: str


_ERROR_RECORDS: Dict[str, List[ErrorRecord]] = {}


def log_error(context: str, help_message: str, stack_trace=True) -> None:
    stack_trace = "" if not stack_trace else traceback.format_exc().replace("\\n", "\n")
    logger.error(context)
    logger.error(help_message)
    if stack_trace:
        logger.error(stack_trace)
    if context not in _ERROR_RECORDS:
        _ERROR_RECORDS[context] = []
    error_record = ErrorRecord(help_message, stack_trace)
    _ERROR_RECORDS[context].append(error_record)


def write_error_report(artifact_dir: str):
    if _ERROR_RECORDS:
        logger.critical("DETECTED ERRORS THIS RUN!")
        error_report_file_name = create_standard_log_name("error_report", "txt", parent=artifact_dir)
        with open(error_report_file_name, "a+") as error_report_file:
            for context in _ERROR_RECORDS:
                print_and_write(add_border(f"Errors for {context}"), error_report_file, important=True)
                for record in _ERROR_RECORDS[context]:
                    print_and_write(str(record), error_report_file, important=True)
    else:
        logger.info("No errors seen this run!")

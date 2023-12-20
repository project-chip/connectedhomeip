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

from dataclasses import dataclass
from typing import Any, TextIO

from utils.log import print_and_write


@dataclass
class Cause:
    """
    A representation of a prescriptive log analysis
    search_terms: terms to match in a log line (term1 AND term2 AND term3 etc...)
    search_attr: analyze this attribute of an analysis object for this cause
    help_message: how to help the user if this cause is discovered
    follow_up_causes: more checks to continue narrowing down the root cause
    """
    search_terms: [str]
    search_attr: [str]
    help_message: str
    follow_up_causes: []  # : [Cause] (Recursive reference does not work for type hint here)


def check_cause(analysis_object: Any, cause: Cause, analysis_file: TextIO) -> None:
    """
    Check if an analysis object has revealed any problem causes
    analysis_object: the object containing log lines as search sources
    cause: the cause object to check for
    analysis_file: the open file to write discovered causes too
    """
    to_search = getattr(analysis_object, cause.search_attr)
    found = True
    for search_term in cause.search_terms:
        found = found and search_term in to_search
    if found:
        print_and_write(cause.help_message, analysis_file)
    if found and cause.follow_up_causes:
        for follow_up in cause.follow_up_causes:
            check_cause(analysis_object, follow_up, analysis_file)

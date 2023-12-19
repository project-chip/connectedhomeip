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

import os
from dataclasses import dataclass
from typing import TextIO

from capture.platform.android import Android
from utils.artifact import create_standard_log_name, log
from utils.log import add_border, print_and_write

logger = log.get_logger(__file__)


@dataclass(repr=True)
class Cause:
    search_terms: [str]
    search_attr: [str]
    help_message: str
    follow_up_causes: []  # : [Cause] (Recursive reference does not work for type hint here)


class PlayServicesAnalysis:

    def __init__(self, platform: Android, artifact_dir: str) -> None:
        self.logger = logger
        self.artifact_dir = artifact_dir
        self.analysis_file_name = os.path.join(
            self.artifact_dir, create_standard_log_name(
                'commissioning_logcat', 'txt'))

        self.platform = platform

        self.matter_commissioner_logs = ''
        self.failure_stack_trace = ''
        self.pake_logs = ''
        self.resolver_logs = ''
        self.sigma_logs = ''
        self.fail_trace_line_counter = -1
        self.causes = [
            Cause(["Failed to discover commissionable device."],
                  "failure_stack_trace",
                  "Play Services could not locate the device's initial advertisement. Use $ idt discover to check!",
                  []),
            Cause(["SetupDeviceViewModel", "Failed to discover operational device"],
                  "failure_stack_trace",
                  "All steps of PASE completed as expected, but we failed to establish a secure session on IP network",
                  [Cause(["AddressResolve_DefaultImpl", "Timeout"],
                         "matter_commissioner_logs",
                         "Play Services failed to locate end device via DNS-SD. Inspect pcaps / $ idt discover -t d",
                         []),
                   Cause(["CASESession", "Timeout"],
                         "matter_commissioner_logs",
                         "End device discovered via DNS-SD, but the secure session handshake failed! Try $ idt probe",
                         []),
                   ])
        ]

    def _log_proc_matter_commissioner(self, line: str) -> None:
        """Core commissioning flow"""
        if 'MatterCommissioner' in line:
            self.logger.info(line)
            self.matter_commissioner_logs += line

    def _log_proc_commissioning_failed(self, line: str) -> None:
        parsed_stack_trace_max_depth = 15
        if self.fail_trace_line_counter > parsed_stack_trace_max_depth:
            self.fail_trace_line_counter = -1
        if self.fail_trace_line_counter > -1 and 'SetupDevice' in line:
            self.failure_stack_trace += line
            self.fail_trace_line_counter += 1
        if 'SetupDeviceView' and 'Commissioning failed' in line:
            self.logger.info(line)
            self.fail_trace_line_counter = 0
            self.failure_stack_trace += line

    def _log_proc_pake(self, line: str) -> None:
        """Three logs for pake 1-3 expected"""
        if "Pake" in line and "chip_logging" in line:
            self.logger.info(line)
            self.pake_logs += line

    def _log_proc_mdns(self, line: str) -> None:
        if "_matter" in line and "ServiceResolverAdapter" in line:
            self.logger.info(line)
            self.resolver_logs += line

    def _log_proc_sigma(self, line: str) -> None:
        """Three logs expected for sigma 1-3"""
        if "Sigma" in line and "chip_logging" in line:
            self.logger.info(line)
            self.sigma_logs += line

    def check_cause(self, cause: Cause, analysis_file: TextIO):
        to_search = getattr(self, cause.search_attr)
        found = True
        for search_term in cause.search_terms:
            found = found and search_term in to_search
        if found:
            print_and_write(cause.help_message, analysis_file)
        if found and cause.follow_up_causes:
            for follow_up in cause.follow_up_causes:
                self.check_cause(follow_up, analysis_file)

    def do_prescriptive_analysis(self, analysis_file: TextIO) -> None:
        print_and_write(add_border("Prescriptive analysis"), analysis_file)
        for cause in self.causes:
            self.check_cause(cause, analysis_file)

    def show_analysis(self) -> None:
        analysis_file = open(self.analysis_file_name, mode="w+")
        print_and_write(add_border('Matter commissioner logs'), analysis_file)
        print_and_write(self.matter_commissioner_logs, analysis_file)
        print_and_write(
            add_border('Commissioning failure stack trace'),
            analysis_file)
        print_and_write(self.failure_stack_trace, analysis_file)
        print_and_write(add_border('PASE Handshake'), analysis_file)
        print_and_write(self.pake_logs, analysis_file)
        print_and_write(add_border('DNS-SD resolution'), analysis_file)
        print_and_write(self.resolver_logs, analysis_file)
        print_and_write(add_border('CASE handshake'), analysis_file)
        print_and_write(self.sigma_logs, analysis_file)
        self.do_prescriptive_analysis(analysis_file)
        analysis_file.close()

    def process_line(self, line: str) -> None:
        for line_func in [s for s in dir(self) if s.startswith('_log')]:
            getattr(self, line_func)(line)

    def do_analysis(self, batch: [str]) -> None:
        for line in batch:
            self.process_line(line)

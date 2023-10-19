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

from capture.file_utils import add_border, create_standard_log_name, print_and_write
from capture.platform.android import Android


class PlayServicesAnalysis:

    def __init__(self, platform: Android, artifact_dir: str) -> None:
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

    def _log_proc_matter_commissioner(self, line: str) -> None:
        """Core commissioning flow"""
        if 'MatterCommissioner' in line:
            self.matter_commissioner_logs += line

    def _log_proc_commissioning_failed(self, line: str) -> None:
        parsed_stack_trace_max_depth = 15
        if self.fail_trace_line_counter > parsed_stack_trace_max_depth:
            self.fail_trace_line_counter = -1
        if self.fail_trace_line_counter > -1 and 'SetupDevice' in line:
            self.failure_stack_trace += line
            self.fail_trace_line_counter += 1
        if 'SetupDeviceView' and 'Commissioning failed' in line:
            self.fail_trace_line_counter = 0
            self.failure_stack_trace += line

    def _log_proc_pake(self, line: str) -> None:
        """Three logs for pake 1-3 expected"""
        if "Pake" in line and "chip_logging" in line:
            self.pake_logs += line

    def _log_proc_mdns(self, line: str) -> None:
        if "_matter" in line and "ServiceResolverAdapter" in line:
            self.resolver_logs += line

    def _log_proc_sigma(self, line: str) -> None:
        """Three logs expected for sigma 1-3"""
        if "Sigma" in line and "chip_logging" in line:
            self.sigma_logs += line

    def _show_analysis(self) -> None:
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
        analysis_file.close()

    def process_line(self, line: str) -> None:
        for line_func in filter(lambda s: s.startswith('_log'), dir(self)):
            getattr(self, line_func)(line)

    def do_analysis(self) -> None:
        with open(self.platform.logcat_output_path, mode='r') as logcat_file:
            for line in logcat_file:
                self.process_line(line)
        self._show_analysis()

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

from features.capture.platform.android import Android
from features.capture.platform.android.streams.logcat import LogcatStreamer
from utils.analysis import TextCause, PrescriptiveTextAnalysis, TextAnalysisObserver
from utils.artifact import create_standard_log_name, log
from utils.log import add_border, print_and_write

logger = log.get_logger(__file__)


class PlayServicesAnalysis(PrescriptiveTextAnalysis, TextAnalysisObserver):

    def __init__(self, platform: Android, artifact_dir: str) -> None:

        self.logger = logger
        self.output_file_name = os.path.join(
            artifact_dir, create_standard_log_name(
                'commissioning_logcat', 'txt'))

        causes = [
            TextCause(["Failed to discover commissionable device."],
                      "failure_stack_trace",
                      "Play Services could not locate the device's initial advertisement. Use $ idt discover to check!",
                      []),
            TextCause(["Failed to discover operational device"],
                      "failure_stack_trace",
                      "All steps of PASE completed as expected, but the secure session setup on IP network failed.",
                      [
                          TextCause(["AddressResolve_DefaultImpl", "Timeout"],
                                    "matter_commissioner_logs",
                                    "Play Services failed to locate end device via DNS-SD. Inspect pcaps / $ idt "
                                    "discover -t d",
                                    []),
                          TextCause(["secure_channel/CASESession"],
                                    "matter_commissioner_logs",
                                    "End device discovered via DNS-SD, but session handshake failed! Try $ idt probe",
                                    []),
                      ])
        ]
        PrescriptiveTextAnalysis.__init__(self, causes, self.output_file_name, logger)

        logcat_stream: LogcatStreamer = platform.streams["LogcatStreamer"]
        TextAnalysisObserver.__init__(self, logcat_stream.logcat_artifact, logger)

        self.matter_commissioner_logs = ''
        self.failure_stack_trace = ''
        self.pake_logs = ''
        self.resolver_logs = ''
        self.sigma_logs = ''
        self.fail_trace_line_counter = -1

    def _log_proc_matter_commissioner(self, line: str) -> None:
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
            self.logger.warning("Failure detected, you may want to stop the capture now!")
            self.fail_trace_line_counter = 0
            self.failure_stack_trace += line

    def _log_proc_pake(self, line: str) -> None:
        if "Pake" in line and "chip_logging" in line:
            self.logger.info(line)
            self.pake_logs += line

    def _log_proc_mdns(self, line: str) -> None:
        if "_matter" in line and "ServiceResolverAdapter" in line:
            self.logger.info(line)
            self.resolver_logs += line

    def _log_proc_sigma(self, line: str) -> None:
        if "Sigma" in line and "chip_logging" in line:
            self.logger.info(line)
            self.sigma_logs += line

    # TODO: Option to analyze existing log
    def show_analysis(self) -> None:
        with open(self.output_file_name, mode="w+") as analysis_file:
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
            print_and_write(add_border('Prescriptive analysis findings'), analysis_file)
        self.check_causes()

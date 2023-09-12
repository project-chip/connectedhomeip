import os

from capture.file_utils import (add_border, create_standard_log_name,
                                print_and_write)
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
        """Core commissioning flow is covered by MatterCommissioner"""
        if 'MatterCommissioner' in line:
            self.matter_commissioner_logs += line

    def _log_proc_commissioning_failed(self, line: str) -> None:
        if self.fail_trace_line_counter > 15:
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
        """Find matter related names"""
        if "_matter" in line and "ServiceResolverAdapter" in line:
            self.resolver_logs += line

    def _log_proc_sigma(self, line: str) -> None:
        """Three logs expected for sigma 1-3"""
        # TODO: Upon failure, try ping and route to the timed out addr and
        # write to artifacts
        if "Sigma" in line and "chip_logging" in line:
            self.sigma_logs += line

    def _show_analysis(self) -> None:
        """Display the analysis data"""
        analysis_file = open(self.analysis_file_name, mode="w+")
        print_and_write(add_border('Matter commissioner logs'), analysis_file)
        print_and_write(self.matter_commissioner_logs, analysis_file)
        print_and_write(
            add_border('Commissioning failure stack trace'),
            analysis_file)
        print_and_write(self.failure_stack_trace, analysis_file)
        print_and_write(add_border('PASE Handshake'), analysis_file)
        print_and_write(self.pake_logs, analysis_file)
        print_and_write(add_border('mDNS resolution'), analysis_file)
        print_and_write(self.resolver_logs, analysis_file)
        print_and_write(add_border('CASE handshake'), analysis_file)
        print_and_write(self.sigma_logs, analysis_file)
        analysis_file.close()

    def process_line(self, line: str) -> None:
        """Run every _log prefixed function in this class against the log line"""
        for line_func in filter(lambda s: s.startswith('_log'), dir(self)):
            getattr(self, line_func)(line)

    def do_analysis(self) -> None:
        """Process every line of logcat once"""
        # TODO: Generic log functions (e.g. after, pattern...)
        with open(self.platform.logcat_output_path, mode='r') as logcat_file:
            for line in logcat_file:
                self.process_line(line)
        self._show_analysis()

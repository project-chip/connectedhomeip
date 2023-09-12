import os

from capture.base import EcosystemCapture, UnsupportedCapturePlatformException
from capture.file_utils import create_standard_log_name, print_and_write
from capture.platform.android.android import Android


class PlayServicesUser(EcosystemCapture):
    """
    Implementation of capture and analysis for Play Services 3P
    """

    def __init__(self, platform: Android, artifact_dir: str) -> None:

        self.artifact_dir = artifact_dir
        self.analysis_file = os.path.join(
            self.artifact_dir, create_standard_log_name(
                'commissioning_boundaries', 'txt'))

        if not isinstance(platform, Android):
            raise UnsupportedCapturePlatformException(
                'only platform=android is supported for '
                'ecosystem=PlayServicesUser')
        self.platform = platform

    def start_capture(self) -> None:
        self.platform.start_streaming()

    def stop_capture(self) -> None:
        self.platform.stop_streaming()

    def analyze_capture(self) -> None:
        """"Show the start and end times of commissioning boundaries"""
        analysis_file = open(self.analysis_file, mode='w+')
        with open(self.platform.logcat_output_path, mode='r') as logcat_file:
            for line in logcat_file:
                if "CommissioningServiceBin: Binding to service" in line:
                    print_and_write(
                        f"3P commissioner initiated GPS commissioning\n{line}",
                        analysis_file)
                elif "CommissioningServiceBin: Sending commissioning request to bound service" in line:
                    print_and_write(
                        f"GPS commissioning complete; passing back to 3P\n{line}",
                        analysis_file)
                elif "CommissioningServiceBin: Received commissioning complete from bound service" in line:
                    print_and_write(
                        f"3P commissioning complete!\n{line}",
                        analysis_file)
        analysis_file.close()

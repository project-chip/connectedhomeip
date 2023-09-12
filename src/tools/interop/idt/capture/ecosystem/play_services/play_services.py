import json
import os
from typing import Dict

from capture.base import EcosystemCapture, UnsupportedCapturePlatformException
from capture.file_utils import create_standard_log_name
from capture.platform.android import Android

from .analysis import PlayServicesAnalysis
from .command_map import properties_and_commands


class PlayServices(EcosystemCapture):
    """
    Implementation of capture and analysis for Play Services
    """

    def __init__(self, platform: Android, artifact_dir: str) -> None:

        self.artifact_dir = artifact_dir

        if not isinstance(platform, Android):
            raise UnsupportedCapturePlatformException(
                'only platform=android is supported for ecosystem=play_services')
        self.platform = platform

        self.standard_info_file_path = os.path.join(
            self.artifact_dir, create_standard_log_name(
                'phone_info', 'json'))
        self.standard_info_data: Dict[str, str] | None = None

        self.analysis = PlayServicesAnalysis(self.platform, self.artifact_dir)

        service_ids = ['336', '305', '168']
        for service_id in service_ids:
            verbose_command = f"shell setprop log.tag.gms_svc_id:{service_id} VERBOSE"
            self.platform.run_adb_command(verbose_command)

    def _write_standard_info_file(self) -> None:
        """Write env details to json file"""
        standard_info_data_json = json.dumps(self.standard_info_data, indent=2)
        with open(self.standard_info_file_path, mode='w+') as standard_info_file:
            standard_info_file.write(standard_info_data_json)

    def _get_standard_info(self) -> None:
        """Fetch helpful env details"""
        standard_info_data = {}
        for attr_name, command in properties_and_commands.items():
            command_r = self.platform.run_adb_command(
                command, capture_output=True)
            command_output = command_r.get_captured_output()
            standard_info_data[attr_name] = command_output
            print(f'{attr_name}: {command_output}')
        self.standard_info_data = standard_info_data
        self._write_standard_info_file()

    def start_capture(self) -> None:
        self._get_standard_info()
        self.platform.start_streaming()

    def stop_capture(self) -> None:
        self.platform.stop_streaming()

    def analyze_capture(self) -> None:
        self.analysis.do_analysis()

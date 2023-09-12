import os
import typing

import capture
from capture.base import EcosystemCapture, PlatformLogStreamer
from capture.file_utils import safe_mkdir

_PLATFORM_MAP: typing.Dict[str, PlatformLogStreamer] = {}


class CapturePlatforms:

    @staticmethod
    def list_available_platforms() -> typing.List[str]:
        return capture.platform.__all__

    @staticmethod
    def get_platform_impl(
            platform: str,
            artifact_dir: str) -> PlatformLogStreamer:
        """Fetches a singleton instance of the requested platform transport"""
        if platform in _PLATFORM_MAP:
            return _PLATFORM_MAP[platform]
        platform_class = getattr(capture.platform, platform)
        platform_artifact_dir = os.path.join(artifact_dir, platform)
        safe_mkdir(platform_artifact_dir)
        platform_inst = platform_class(platform_artifact_dir)
        _PLATFORM_MAP[platform] = platform_inst
        return platform_inst


class CaptureEcosystems:

    @staticmethod
    def list_available_ecosystems() -> typing.List[str]:
        return capture.ecosystem.__all__

    @staticmethod
    def get_ecosystem_impl(
            ecosystem: str,
            platform: str,
            artifact_dir: str) -> EcosystemCapture:
        ecosystem_class = getattr(capture.ecosystem, ecosystem)
        ecosystem_artifact_dir = os.path.join(artifact_dir, ecosystem)
        safe_mkdir(ecosystem_artifact_dir)
        platform_instance = CapturePlatforms.get_platform_impl(
            platform, artifact_dir)
        return ecosystem_class(platform_instance, ecosystem_artifact_dir)

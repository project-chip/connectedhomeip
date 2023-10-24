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
import copy
import multiprocessing
import os
import traceback
import typing
from multiprocessing import Process

import capture
from capture.utils.async_control import get_timeout
from capture.base import EcosystemCapture, PlatformLogStreamer, UnsupportedCapturePlatformException
from capture.utils.artifact import safe_mkdir, create_standard_log_name
from log import border_print
import log

_PLATFORM_MAP: typing.Dict[str, PlatformLogStreamer] = {}
_ECOSYSTEM_MAP: typing.Dict[str, PlatformLogStreamer] = {}
_ERROR_REPORT: typing.Dict[str, list[(str, str, str)]] = {}
_ANALYSIS_MAP: typing.Dict[str, Process] = {}

logger = log.get_logger(__file__)


class PlatformFactory:

    @staticmethod
    def list_available_platforms() -> typing.List[str]:
        return copy.deepcopy(capture.platform.__all__)

    @staticmethod
    async def get_platform_impl(
            platform: str,
            artifact_dir: str) -> PlatformLogStreamer:
        if platform in _PLATFORM_MAP:
            return _PLATFORM_MAP[platform]
        border_print(f"Initializing platform {platform}")
        platform_class = getattr(capture.platform, platform)
        platform_artifact_dir = os.path.join(artifact_dir, platform)
        safe_mkdir(platform_artifact_dir)
        platform_inst = platform_class(platform_artifact_dir)
        _PLATFORM_MAP[platform] = platform_inst
        await platform_inst.connect()
        return platform_inst


class EcosystemFactory:

    @staticmethod
    def list_available_ecosystems() -> typing.List[str]:
        return copy.deepcopy(capture.ecosystem.__all__)

    @staticmethod
    async def get_ecosystem_impl(
            ecosystem: str,
            platform: PlatformLogStreamer,
            artifact_dir: str) -> EcosystemCapture:
        if ecosystem in _ECOSYSTEM_MAP:
            return _ECOSYSTEM_MAP[ecosystem]
        ecosystem_class = getattr(capture.ecosystem, ecosystem)
        ecosystem_artifact_dir = os.path.join(artifact_dir, ecosystem)
        safe_mkdir(ecosystem_artifact_dir)
        ecosystem_instance = ecosystem_class(platform, ecosystem_artifact_dir)
        _ECOSYSTEM_MAP[ecosystem] = ecosystem_instance
        return ecosystem_instance

    @staticmethod
    async def init_ecosystems(platform, ecosystem, artifact_dir):
        async with asyncio.timeout_at(get_timeout()):
            platform = await PlatformFactory.get_platform_impl(
                platform, artifact_dir)
        ecosystems_to_load = EcosystemFactory.list_available_ecosystems() \
            if ecosystem == 'ALL' \
            else [ecosystem]
        for ecosystem in ecosystems_to_load:
            try:
                async with asyncio.timeout_at(get_timeout()):
                    await EcosystemFactory.get_ecosystem_impl(
                        ecosystem, platform, artifact_dir)
            except UnsupportedCapturePlatformException as e:
                logger.error(f"Unsupported platform {ecosystem} {platform}")
                track_error(ecosystem, "UNSUPPORTED_PLATFORM", str(e))
            except TimeoutError as e:
                logger.error(f"Timeout starting ecosystem {ecosystem} {platform}")
                track_error(ecosystem, "TIMEOUT", str(e))
            except Exception as e:
                logger.error(f"Unknown error instantiating ecosystem {ecosystem} {platform}")
                track_error(ecosystem, "UNEXPECTED", str(e))


def track_error(ecosystem: str, error_type: str, error_message: str) -> None:
    if ecosystem not in _ERROR_REPORT:
        _ERROR_REPORT[ecosystem] = []
    e = traceback.format_exc()
    logger.error(e)
    _ERROR_REPORT[ecosystem].append((error_type, error_message, e))


class EcosystemController:

    @staticmethod
    async def handle_capture(attr):
        attr = f"{attr}_capture"
        for ecosystem in _ECOSYSTEM_MAP:
            try:
                border_print(f"{attr} for {ecosystem}")
                async with asyncio.timeout_at(get_timeout()):
                    await getattr(_ECOSYSTEM_MAP[ecosystem], attr)()
            except TimeoutError as e:
                logger.error(f"Timeout {attr} {ecosystem}")
                track_error(ecosystem, "TIMEOUT", str(e))
            except Exception as e:
                logger.error(f"Unexpected error {attr} {ecosystem}")
                track_error(ecosystem, "UNEXPECTED", str(e))

    @staticmethod
    async def start():
        for platform_name, platform, in _PLATFORM_MAP.items():
            border_print(f"Starting streaming for platform {platform_name}")
            await platform.start_streaming()
        await EcosystemController.handle_capture("start")

    @staticmethod
    async def stop():
        for ecosystem_name, ecosystem in _ANALYSIS_MAP.items():
            border_print(f"Stopping analysis proc for {ecosystem_name}")
            ecosystem.kill()
        for platform_name, platform, in _PLATFORM_MAP.items():
            border_print(f"Stopping streaming for platform {platform_name}")
            await platform.stop_streaming()
        await EcosystemController.handle_capture("stop")

    @staticmethod
    def analyze():
        for ecosystem_name, ecosystem in _ECOSYSTEM_MAP.items():
            border_print(f"Starting analyze subproc for {ecosystem_name}")
            proc = multiprocessing.Process(target=ecosystem.analyze_capture)
            _ANALYSIS_MAP[ecosystem_name] = proc
            proc.start()

    @staticmethod
    async def probe():
        await EcosystemController.handle_capture("probe")

    @staticmethod
    def error_report(artifact_dir: str):
        error_report_file_name = create_standard_log_name("error_report", "txt", parent=artifact_dir)
        with open(error_report_file_name, 'a+') as error_report_file:
            for k, v in _ERROR_REPORT.items():
                log.print_and_write(f"{k}: {v}", error_report_file)

    @staticmethod
    def has_errors():
        return len(_ERROR_REPORT) > 0

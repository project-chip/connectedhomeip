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
import os
import sys
import traceback
import typing

import capture
from capture.base import EcosystemCapture, PlatformLogStreamer, UnsupportedCapturePlatformException
from utils.artifact import create_standard_log_name, log, safe_mkdir
from utils.log import border_print

from . import config

_PLATFORM_MAP: typing.Dict[str, PlatformLogStreamer] = {}
_ECOSYSTEM_MAP: typing.Dict[str, PlatformLogStreamer] = {}
_ERROR_REPORT: typing.Dict[str, list[(str, str, str)]] = {}

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
        async with asyncio.timeout(config.orchestrator_async_step_timeout_seconds):
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
        platform = await PlatformFactory.get_platform_impl(platform, artifact_dir)
        ecosystems_to_load = EcosystemFactory.list_available_ecosystems() \
            if ecosystem == 'ALL' \
            else [ecosystem]
        for ecosystem in ecosystems_to_load:
            try:
                await EcosystemFactory.get_ecosystem_impl(
                    ecosystem, platform, artifact_dir)
            except UnsupportedCapturePlatformException as e:
                logger.error(f"Unsupported platform {ecosystem} {platform}")
                track_error(ecosystem, "UNSUPPORTED_PLATFORM", str(e))
            except Exception as e:
                logger.error(f"Unknown error instantiating ecosystem {ecosystem} {platform}")
                track_error(ecosystem, "UNEXPECTED", str(e))


def track_error(ecosystem: str, error_type: str, error_message: str) -> None:
    if ecosystem not in _ERROR_REPORT:
        _ERROR_REPORT[ecosystem] = []
    e = traceback.format_exc()
    logger.error(e)
    _ERROR_REPORT[ecosystem].append((error_type, error_message, e))


class Orchestrator:

    @staticmethod
    async def handle_capture(attr):
        attr = f"{attr}_capture"
        for ecosystem in _ECOSYSTEM_MAP:
            try:
                border_print(f"{attr} for {ecosystem}")
                async with asyncio.timeout(config.orchestrator_async_step_timeout_seconds):
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
        await Orchestrator.handle_capture("start")

    @staticmethod
    async def stop():
        for platform_name, platform, in _PLATFORM_MAP.items():
            border_print(f"Stopping streaming for platform {platform_name}")
            await platform.stop_streaming()
        await Orchestrator.handle_capture("stop")

    @staticmethod
    async def run_analyzers():
        border_print("Starting real time analysis, press enter to stop!", important=True)
        analysis_tasks = []
        monitor_tasks = []
        for platform_name, platform in _PLATFORM_MAP.items():
            logger.info(f"Creating monitor task for {platform_name}")
            monitor_tasks.append(asyncio.create_task(platform.run_observers()))
        for ecosystem_name, ecosystem in _ECOSYSTEM_MAP.items():
            logger.info(f"Creating analysis task for {ecosystem_name}")
            analysis_tasks.append(asyncio.create_task(ecosystem.analyze_capture()))
        logger.info("Done creating analysis tasks")
        await asyncio.get_event_loop().run_in_executor(
            None, sys.stdin.readline)
        border_print("Cancelling monitor tasks")
        for task in monitor_tasks:
            task.cancel()
        logger.info("Done cancelling monitor tasks")
        border_print("Cancelling analysis tasks")
        for task in analysis_tasks:
            task.cancel()
        logger.info("Done cancelling analysis tasks")

    @staticmethod
    async def probe():
        await Orchestrator.handle_capture("probe")

    @staticmethod
    def error_report(artifact_dir: str):
        error_report_file_name = create_standard_log_name("error_report", "txt", parent=artifact_dir)
        with open(error_report_file_name, 'a+') as error_report_file:
            for k, v in _ERROR_REPORT.items():
                log.print_and_write(f"{k}: {v}", error_report_file)

    @staticmethod
    def has_errors():
        return len(_ERROR_REPORT) > 0

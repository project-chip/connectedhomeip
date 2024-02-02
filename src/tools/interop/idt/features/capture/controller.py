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
import typing
from asyncio import Task

from features.capture.base import EcosystemCapture, PlatformLogStreamer, UnsupportedCapturePlatformException
from utils.artifact import safe_mkdir
from utils.error import log_error
from utils.log import border_print, get_logger

from .. import capture
from . import config

_PLATFORM_MAP: typing.Dict[str, PlatformLogStreamer] = {}
_ECOSYSTEM_MAP: typing.Dict[str, EcosystemCapture] = {}

logger = get_logger(__file__)


def list_available_platforms() -> typing.List[str]:
    return copy.deepcopy(capture.platform.__all__)


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
    async with asyncio.timeout(config.ORCHESTRATOR_ASYNC_STEP_TIMEOUT_SECONDS):
        await platform_inst.connect()
    return platform_inst


def list_available_ecosystems() -> typing.List[str]:
    return copy.deepcopy(capture.ecosystem.__all__)


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


async def init_ecosystems(platform, ecosystem, artifact_dir):
    platform = await get_platform_impl(platform, artifact_dir)
    ecosystems_to_load = list_available_ecosystems() \
        if ecosystem == 'ALL' \
        else [ecosystem]
    for ecosystem in ecosystems_to_load:
        try:
            await get_ecosystem_impl(
                ecosystem, platform, artifact_dir)
        except UnsupportedCapturePlatformException:
            help_message = f"Unsupported platform {ecosystem} {platform}"
            logger.error(help_message)
            log_error(ecosystem, help_message)
        except Exception:
            help_message = f"Unknown error instantiating ecosystem {ecosystem} {platform}"
            logger.error(help_message)
            log_error(ecosystem, help_message)


async def handle_capture(attr):
    attr = f"{attr}_capture"
    for ecosystem in _ECOSYSTEM_MAP:
        try:
            border_print(f"{attr} for {ecosystem}")
            async with asyncio.timeout(config.ORCHESTRATOR_ASYNC_STEP_TIMEOUT_SECONDS):
                await getattr(_ECOSYSTEM_MAP[ecosystem], attr)()
        except TimeoutError:
            help_message = f"Timeout after {config.ORCHESTRATOR_ASYNC_STEP_TIMEOUT_SECONDS} seconds {attr} {ecosystem}"
            logger.error(help_message)
            log_error(ecosystem, help_message)
        except Exception:
            help_message = f"Unexpected error {attr} {ecosystem}"
            logger.error(help_message)
            log_error(ecosystem, help_message)


async def start():
    for platform_name, platform, in _PLATFORM_MAP.items():
        # TODO: Write error log if halt here
        border_print(f"Starting streaming for platform {platform_name}")
        await platform.start_streaming()
    await handle_capture("start")


async def stop():
    for platform_name, platform, in _PLATFORM_MAP.items():
        # TODO: Write error log if halt here
        border_print(f"Stopping streaming for platform {platform_name}")
        await platform.stop_streaming()
    await handle_capture("stop")


def sub_task_error_tracker_done_callback(task: asyncio.Task) -> None:
    try:
        task.result()
    except asyncio.CancelledError:
        pass
    except Exception:
        platform_or_ecosystem = "UNKNOWN"
        if hasattr(task, "error_tracking_name"):
            platform_or_ecosystem = getattr(task, "error_tracking_name")
        log_error(platform_or_ecosystem, "Unexpected error in sub task")


async def run_analyzers():
    border_print("Starting real time analysis, press enter to stop!", important=True)
    analysis_tasks: typing.Dict[str, Task] = {}
    monitor_tasks: typing.Dict[str, Task] = {}
    for platform_name, platform in _PLATFORM_MAP.items():
        logger.info(f"Creating monitor task for {platform_name}")
        monitor_tasks[platform_name] = asyncio.create_task(platform.run_observers())
        setattr(monitor_tasks[platform_name], "error_tracking_name", platform_name)
        monitor_tasks[platform_name].add_done_callback(sub_task_error_tracker_done_callback)
    for ecosystem_name, ecosystem in _ECOSYSTEM_MAP.items():
        logger.info(f"Creating analysis task for {ecosystem_name}")
        analysis_tasks[ecosystem_name] = asyncio.create_task(ecosystem.analyze_capture())
        setattr(analysis_tasks[ecosystem_name], "error_tracking_name", ecosystem_name)
        analysis_tasks[ecosystem_name].add_done_callback(sub_task_error_tracker_done_callback)
    logger.info("Done creating analysis tasks")
    await asyncio.get_event_loop().run_in_executor(
        None, sys.stdin.readline)
    border_print("Cancelling monitor tasks")
    for platform_name, monitor_task in monitor_tasks.items():
        monitor_task.cancel()
    logger.info("Done cancelling monitor tasks")
    border_print("Cancelling analysis tasks")
    for ecosystem_name, analysis_task in analysis_tasks.items():
        analysis_task.cancel()
    logger.info("Done cancelling analysis tasks")


async def probe():
    await handle_capture("probe")

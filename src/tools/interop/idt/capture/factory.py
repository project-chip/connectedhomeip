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
import traceback
import typing

import capture
from capture.base import EcosystemCapture, PlatformLogStreamer, UnsupportedCapturePlatformException
from capture.file_utils import border_print, safe_mkdir

_CONFIG_TIMEOUT = 45.0
_PLATFORM_MAP: typing.Dict[str, PlatformLogStreamer] = {}
_ECOSYSTEM_MAP: typing.Dict[str, PlatformLogStreamer] = {}


def _get_timeout():
    return asyncio.get_running_loop().time() + _CONFIG_TIMEOUT


class PlatformFactory:

    @staticmethod
    def list_available_platforms() -> typing.List[str]:
        return copy.deepcopy(capture.platform.__all__)

    @staticmethod
    def get_platform_impl(
            platform: str,
            artifact_dir: str) -> PlatformLogStreamer:
        if platform in _PLATFORM_MAP:
            return _PLATFORM_MAP[platform]
        platform_class = getattr(capture.platform, platform)
        platform_artifact_dir = os.path.join(artifact_dir, platform)
        safe_mkdir(platform_artifact_dir)
        platform_inst = platform_class(platform_artifact_dir)
        _PLATFORM_MAP[platform] = platform_inst
        return platform_inst


class EcosystemFactory:

    @staticmethod
    def list_available_ecosystems() -> typing.List[str]:
        return copy.deepcopy(capture.ecosystem.__all__)

    @staticmethod
    async def get_ecosystem_impl(
            ecosystem: str,
            platform: str,
            artifact_dir: str) -> EcosystemCapture:
        if ecosystem in _ECOSYSTEM_MAP:
            return _ECOSYSTEM_MAP[ecosystem]
        ecosystem_class = getattr(capture.ecosystem, ecosystem)
        ecosystem_artifact_dir = os.path.join(artifact_dir, ecosystem)
        safe_mkdir(ecosystem_artifact_dir)
        platform_instance = PlatformFactory.get_platform_impl(
            platform, artifact_dir)
        ecosystem_instance = ecosystem_class(platform_instance, ecosystem_artifact_dir)
        _ECOSYSTEM_MAP[ecosystem] = ecosystem_instance
        return ecosystem_instance

    @staticmethod
    async def init_ecosystems(platform, ecosystem, artifact_dir):
        ecosystems_to_load = EcosystemFactory.list_available_ecosystems() \
            if ecosystem == 'ALL' \
            else [ecosystem]
        for ecosystem in ecosystems_to_load:
            try:
                async with asyncio.timeout_at(_get_timeout()):
                    await EcosystemFactory.get_ecosystem_impl(
                        ecosystem, platform, artifact_dir)
            except UnsupportedCapturePlatformException:
                print(f"ERROR unsupported platform {ecosystem} {platform}")
            except TimeoutError:
                print(f"ERROR timeout starting ecosystem {ecosystem} {platform}")
            except Exception:
                print("ERROR unknown error instantiating ecosystem")
                print(traceback.format_exc())


class EcosystemController:

    @staticmethod
    async def handle_capture(attr):
        attr = f"{attr}_capture"
        for ecosystem in _ECOSYSTEM_MAP:
            try:
                border_print(f"{attr} capture for {ecosystem}")
                async with asyncio.timeout_at(_get_timeout()):
                    await getattr(_ECOSYSTEM_MAP[ecosystem], attr)()
            except TimeoutError:
                print(f"ERROR timeout {attr} {ecosystem}")
            except Exception:
                print(f"ERROR unexpected error {attr} {ecosystem}")
                print(traceback.format_exc())

    @staticmethod
    async def start():
        await EcosystemController.handle_capture("start")

    @staticmethod
    async def stop():
        await EcosystemController.handle_capture("stop")

    @staticmethod
    async def analyze():
        await EcosystemController.handle_capture("analyze")

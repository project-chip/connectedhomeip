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
import os
import traceback
import typing
from asyncio import get_running_loop

import capture
from capture.base import EcosystemCapture, PlatformLogStreamer, UnsupportedCapturePlatformException
from capture.file_utils import border_print, safe_mkdir

_PLATFORM_MAP: typing.Dict[str, PlatformLogStreamer] = {}
_ECOSYSTEM_MAP: typing.Dict[str, PlatformLogStreamer] = {}


class CapturePlatforms:

    @staticmethod
    def list_available_platforms() -> typing.List[str]:
        return capture.platform.__all__

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


class CaptureEcosystems:

    @staticmethod
    def list_available_ecosystems() -> typing.List[str]:
        return capture.ecosystem.__all__

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
        platform_instance = CapturePlatforms.get_platform_impl(
            platform, artifact_dir)
        ecosystem_instance = ecosystem_class(platform_instance, ecosystem_artifact_dir)
        _ECOSYSTEM_MAP[ecosystem] = ecosystem_instance
        return ecosystem_instance


async def init_ecosystems(platform, ecosystem, artifact_dir):
    ecosystems_to_load = CaptureEcosystems.list_available_ecosystems() \
        if ecosystem == 'ALL' \
        else [ecosystem]
    for ecosystem in ecosystems_to_load:
        try:
            async with asyncio.timeout(30):
                await CaptureEcosystems.get_ecosystem_impl(
                    ecosystem, platform, artifact_dir)
        except UnsupportedCapturePlatformException:
            print(f"ERROR unsupported platform {ecosystem} {platform}")
        except TimeoutError:
            print(f"ERROR timeout starting ecosystem {ecosystem} {platform}")
        except Exception:
            print("ERROR unknown error instantiating ecosystem")
            print(traceback.format_exc())


async def handle_capture(attr):
    for ecosystem in _ECOSYSTEM_MAP:
        try:
            border_print(f"{attr} capture for {ecosystem}")
            async with asyncio.timeout(get_running_loop().time() + 60):
                await getattr(_ECOSYSTEM_MAP[ecosystem], attr)()
        except TimeoutError:
            print(f"ERROR timeout {attr} {ecosystem}")
        except Exception:
            print(f"ERROR unexpected error {attr} {ecosystem}")
            print(traceback.format_exc())


async def start_captures():
    await handle_capture("start_capture")


async def stop_captures():
    await handle_capture("stop_capture")


async def analyze_captures():
    # TODO: Enable realtime but keep this post capture analysis step
    await handle_capture("analyze_capture")

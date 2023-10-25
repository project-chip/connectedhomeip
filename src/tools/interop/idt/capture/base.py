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

from abc import ABC, abstractmethod


class PlatformLogStreamer(ABC):
    """
    The abstract base class for a platform transport, subclassed by sub packages of platform
    """

    @abstractmethod
    def __init__(self, artifact_dir: str) -> None:
        """
         artifact_dir: the fully qualified path of the output directory. This directory already exists
        """
        raise NotImplementedError

    @abstractmethod
    async def start_streaming(self) -> None:
        """
        Begin streaming logs
        Start should be able to be called repeatedly without restarting streaming
        """
        raise NotImplementedError

    @abstractmethod
    async def stop_streaming(self) -> None:
        """
        Stop streaming logs
        Stop should not cause an error even if the stream is not running
        """
        raise NotImplementedError


class UnsupportedCapturePlatformException(Exception):
    """EcosystemCapture should raise this for unsupported platform"""

    def __init__(self, message: str):
        super().__init__(message)


class EcosystemCapture(ABC):

    @abstractmethod
    def __init__(
            self,
            platform: PlatformLogStreamer,
            artifact_dir: str) -> None:
        """
        platform: the instance of the log streamer for the selected platform
        artifact_dir: the fully qualified path of the output directory. This directory already exists.
        """
        raise NotImplementedError

    @abstractmethod
    async def start_capture(self) -> None:
        """
        Start the capture
        """
        raise NotImplementedError

    @abstractmethod
    async def stop_capture(self) -> None:
        """
        Stop the capture
        """
        raise NotImplementedError

    @abstractmethod
    async def analyze_capture(self) -> None:
        """
        Parse the capture and create + display helpful analysis artifacts that are unique to the ecosystem
        Write analysis artifacts to artifact_dir
        """
        raise NotImplementedError

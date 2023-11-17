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
    async def connect(self) -> None:
        """
        Establish connections to log targets for this platform
        """
        raise NotImplementedError

    @abstractmethod
    async def start_streaming(self) -> None:
        """
        Begin streaming logs
        """
        raise NotImplementedError

    @abstractmethod
    async def run_observers(self) -> None:
        """
        Observe log procs and restart as needed
        Must be async aware and not interact with stdin
        """
        raise NotImplementedError

    @abstractmethod
    async def stop_streaming(self) -> None:
        """
        Stop the capture and pull any artifacts from remote devices
        Write artifacts to artifact_dir passed on instantiation
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
        Platform is already started
        """
        raise NotImplementedError

    @abstractmethod
    async def stop_capture(self) -> None:
        """
        Stop the capture and pull any artifacts from remote devices
        Write artifacts to artifact_dir passed on instantiation
        Platform is already stopped
        """
        raise NotImplementedError

    @abstractmethod
    async def analyze_capture(self) -> None:
        """
        Parse the capture and create + display helpful analysis artifacts that are unique to the ecosystem
        Must be async aware and not interact with stdin
        """
        raise NotImplementedError

    @abstractmethod
    async def probe_capture(self) -> None:
        """
        Probe the local environment, e.g. ping relevant remote services and write respective artifacts
        """
        raise NotImplementedError

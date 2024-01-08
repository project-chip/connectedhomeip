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

from capture.base import EcosystemCapture


class DemoExtEcosystem(EcosystemCapture):

    def __init__(self, platform, artifact_dir: str) -> None:
        self.artifact_dir = artifact_dir
        self.platform = platform
        self.message = "in the demo external ecosystem"

    async def start_capture(self) -> None:
        print("Start capture " + self.message)

    async def stop_capture(self) -> None:
        print("Stop capture " + self.message)

    async def analyze_capture(self) -> None:
        print("Analyze capture " + self.message)

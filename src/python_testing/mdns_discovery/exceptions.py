#
#    Copyright (c) 2024 Project CHIP Authors
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


class DiscoveryNotPerformedError(Exception):
    """Exception raised when MDNS discovery has not been performed."""

    def __init__(self, message="MDNS Discovery has not been performed. Ensure to call and await the `discover` method on this instance first."):
        self.message = message
        super().__init__(self.message)

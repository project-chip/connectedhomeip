#
#    Copyright (c) 2025 Project CHIP Authors
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

import logging
from dataclasses import dataclass, field

from .json_serializable import JsonSerializable

log = logging.getLogger(__name__)


@dataclass
class PtrRecord(JsonSerializable):
    service_type: str
    service_name: str
    instance_name: str = field(init=False)

    def __post_init__(self):
        try:
            # Extract the base type (e.g., _matter._tcp.local.) from the service_type
            base_type = self.service_type
            if "._sub." in self.service_type:
                base_type = self.service_type.split("._sub.", 1)[1]

            # Remove the base_type from the service_name
            if self.service_name.endswith(base_type):
                self.instance_name = self.service_name[: -len(base_type)].rstrip('.')
            else:
                self.instance_name = self.service_name  # fallback
        except (AttributeError, IndexError, TypeError) as e:
            log.info("Failed to extract instance_name from PTR record: %s", e)
            self.instance_name = self.service_name  # final fallback

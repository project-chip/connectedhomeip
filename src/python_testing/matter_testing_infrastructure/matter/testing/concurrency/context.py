# Copyright (c) 2026 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import contextlib
import logging
from abc import ABC, abstractmethod
from types import TracebackType
from typing import Self

log = logging.getLogger(__name__)


class TerminableResource(contextlib.AbstractContextManager, ABC):
    """Abstract base class for resources that can be terminated."""

    def __enter__(self) -> Self:
        log.debug("Starting %s", self.__class__.__name__)
        self.resource_start()
        return self

    def __exit__(self, exc_type: type[BaseException] | None, exc_value: BaseException | None,
                 traceback: TracebackType | None) -> bool | None:
        log.error("Terminating %s", self.__class__.__name__)
        try:
            self.resource_terminate()
        except BaseException as e:
            e.add_note(f"Failure during termination of resource {self.__class__.__name__}")
            raise
        return None

    def resource_start(self) -> None:
        """Initialize or start the resource."""

    @abstractmethod
    def resource_terminate(self) -> None:
        """Terminate the resource."""

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

import dataclasses
import multiprocessing
import tempfile
from pathlib import Path
from typing import ClassVar, Self

from chiptest.log_config import LogConfig
from chiptest.test_definition import TestJobConfig


@dataclasses.dataclass
class ProcessConfig:
    """Process configuration template."""

    DEFAULT_START_TIMEOUT_SEC: ClassVar[float] = 4.0
    DEFAULT_STOP_TIMEOUT_SEC: ClassVar[float] = 4.0
    DEFAULT_TERMINATION_TIMEOUT_SEC: ClassVar[float] = 2.0

    process_id: int = 0
    name: str = "Process{process_id}"
    log_config: LogConfig = dataclasses.field(default_factory=LogConfig)
    start_timeout_sec: float = DEFAULT_START_TIMEOUT_SEC
    stop_timeout_sec: float = DEFAULT_STOP_TIMEOUT_SEC
    termination_timeout_sec: float = DEFAULT_TERMINATION_TIMEOUT_SEC

    def with_formatted_name(self, process_id: int | None = None) -> Self:
        """Format the name using the id and process hierarchy. Return a new instance with the formatted name."""
        if process_id is None:
            process_id = self.process_id
        name = self.name.format(process_id=process_id)

        # Indicate process hierarchy in the name so that it's easier to understand the dependencies in the logs.
        if (parent_proc_name := multiprocessing.current_process().name) != "MainProcess":
            name = f"{parent_proc_name}/{name}"

        return dataclasses.replace(self, process_id=process_id, name=name)


@dataclasses.dataclass
class WorkerConfig(ProcessConfig, TestJobConfig):
    """Configuration for a worker process based on a common test job config."""

    WORKER_START_TIMEOUT: ClassVar[float] = 15
    WORKER_STOP_TIMEOUT: ClassVar[float] = 10

    tmp_dir_clear: bool = False
    tmp_dir_default: ClassVar[Path] = Path(tempfile.gettempdir())
    tmp_dir_worker_base: ClassVar[Path] = Path(tempfile.gettempdir()) / "matter_test_suite"

    @classmethod
    def from_test_job_config(cls, log_config: LogConfig, config: TestJobConfig, *, tmp_dir_clear: bool):
        """Create a worker config from a test job config."""
        # Needs to be a shallow copy, so that we don't accidentally create unpicklable generators in the config.
        return cls(**{field.name: getattr(config, field.name) for field in dataclasses.fields(config)},
                   name=f"W{{process_id:0{len(str(config.concurrency))}}}",
                   log_config=log_config,
                   start_timeout_sec=WorkerConfig.WORKER_START_TIMEOUT,
                   stop_timeout_sec=WorkerConfig.WORKER_STOP_TIMEOUT,
                   tmp_dir_clear=tmp_dir_clear)

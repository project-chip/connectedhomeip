import dataclasses
import enum
import logging
import tempfile
from pathlib import Path
from typing import ClassVar


from chiptest.log_utils import LogConfig
from chiptest.mp_utils.process import ProcessConfigTemplate
from chiptest.test_definition import SubprocessInfoRepo, TestRunTime

log = logging.getLogger(__name__)


class TestSchedulerType(enum.StrEnum):
    FAST = enum.auto()
    REPRODUCIBLE = enum.auto()

@dataclasses.dataclass(frozen=True)
class TestJobConfig:
    """Worker configuration which is a subset of command line options."""
    wifi_required: bool
    thread_required: bool
    commissioning_method: str
    concurrency: int
    concurrency_status: float
    concurrenct_scheduler: TestSchedulerType
    dry_run: bool
    subproc_info_repo: SubprocessInfoRepo
    pics_file: Path
    runtime: TestRunTime
    test_timeout_seconds: int | None
    iterations: int
    keep_going: bool
    expected_failures: int


@dataclasses.dataclass(frozen=True)
class WorkerConfig(ProcessConfigTemplate, TestJobConfig):
    WORKER_START_TIMEOUT: ClassVar[float] = 15
    WORKER_STOP_TIMEOUT: ClassVar[float] = 10

    tmp_dir_default: ClassVar[Path] = Path(tempfile.gettempdir())
    tmp_dir_worker_base: ClassVar[Path] = Path(tempfile.gettempdir()) / "matter_test_suite"

    @classmethod
    def from_test_job_config(cls, log_config: LogConfig, config: TestJobConfig):
        return cls(**dataclasses.asdict(config),
                   name_short=f"W{{id:0{len(str(config.concurrency))}}}",
                   name_long="Worker {id}",
                   log_config=log_config,
                   start_timeout=WorkerConfig.WORKER_START_TIMEOUT,
                   stop_timeout=WorkerConfig.WORKER_STOP_TIMEOUT)

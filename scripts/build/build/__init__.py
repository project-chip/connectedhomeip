import logging
import math
import multiprocessing
import os
import shutil
import time
from enum import Enum, auto
from concurrent.futures import ThreadPoolExecutor
from typing import Sequence

from builders.builder import BuilderOptions, Builder, OutDirLock
from runner.runner import Runner

from .targets import BUILD_TARGETS

log = logging.getLogger(__name__)


class BuildSteps(Enum):
    GENERATED = auto()


class BuildTimer:
    def __init__(self):
        self._total_start_time = None
        self._total_end_time = None
        self._build_times = {}

    def time_builds(self, builders: list[Builder], concurrency: int) -> None:

        def _single_build(builder: Builder):
            start_time = time.time()
            builder.build()
            return builder.identifier, time.time() - start_time

        with ThreadPoolExecutor(thread_name_prefix="Builder", max_workers=concurrency) as pool:
            self._total_start_time = time.time()
            for identifier, build_time in pool.map(_single_build, builders):
                self._build_times[identifier] = build_time
            self._total_end_time = time.time()

    def print_timing_report(self):
        total_time = self._total_end_time - self._total_start_time
        log.info("Build Time Summary:")
        for target, duration in self._build_times.items():
            log.info(f"  - {target}: {self._format_duration(duration)}")
        log.info(f"Total build time: {self._format_duration(total_time)}")

    def _format_duration(self, seconds):
        minutes = int(seconds // 60)
        remaining_seconds = int(seconds % 60)
        if minutes > 0:
            return f"{minutes}m{remaining_seconds}s"
        return f"{remaining_seconds}s"


class Context:
    """Represents a grouped list of platform/board/app builders to use

         to generate make/ninja instructions and to compile.
      """

    def __init__(self, runner: Runner, repository_path: str, output_prefix: str, verbose: bool, quiet: bool, ninja_jobs: int,
                 concurrent_generation: int, concurrent_builders: int):
        self.builders: list[Builder] = []
        self.runner = runner
        self.repository_path = repository_path
        self.output_prefix = output_prefix
        self.verbose = verbose
        self.quiet = quiet
        self.out_dir_lock = OutDirLock()
        self.concurrent_generation = concurrent_generation
        self.concurrent_builders = concurrent_builders
        if concurrent_builders > 1 and (ninja_jobs is None or ninja_jobs == 0):
            self.ninja_jobs = max(1, math.floor(multiprocessing.cpu_count() / concurrent_builders))
        else:
            self.ninja_jobs = ninja_jobs
        self.completed_steps = set()

    def SetupBuilders(self, targets: Sequence[str], options: BuilderOptions):
        """
        Configures internal builders for the given platform/board/app
        combination.
        """

        self.builders = []
        unified_variants = None
        for target in targets:
            found_choice = None
            for choice in BUILD_TARGETS:
                builder = choice.Create(target, self.runner, self.repository_path, self.output_prefix, self.verbose, self.quiet,
                                        self.ninja_jobs, options, self.out_dir_lock)
                if builder:
                    self.builders.append(builder)
                    found_choice = choice
                    # assume a single match. we do not support wildcards
                    break

            if found_choice is None:
                log.error(f"Target '{target}' could not be found. Nothing executed for it")
                continue

            parts = found_choice.StringIntoTargetParts(target)
            if parts and found_choice.isUnifiedBuild(parts):
                # we want to ensure identical settings across builds. For now ensure that
                # variants are identical
                actual_modifiers = {x.name for x in found_choice.modifiers}
                variants = '-'.join([x.name for x in parts if x.name in actual_modifiers])
                if unified_variants is None:
                    unified_variants = variants
                elif unified_variants != variants:
                    raise Exception("Incompatible build variants: %s and %s" % (unified_variants, variants))

        # whenever builders change, assume generation is required again
        self.completed_steps.discard(BuildSteps.GENERATED)

    def Generate(self):
        """
        Performs a build generation IF code generation has not yet been
        performed.
        """
        if BuildSteps.GENERATED in self.completed_steps:
            return

        self.runner.StartCommandExecution()

        def _single_generate(builder: Builder):
            if not builder.quiet:
                log.info('Generating %s', builder.output_dir)
            builder.generate()

        max_workers = self.concurrent_generation if self.concurrent_generation > 0 else multiprocessing.cpu_count()
        with ThreadPoolExecutor(thread_name_prefix="Generator", max_workers=max_workers) as pool:
            list(pool.map(_single_generate, self.builders))

        self.completed_steps.add(BuildSteps.GENERATED)

    def Build(self):
        self.Generate()

        timer = BuildTimer()
        timer.time_builds(self.builders, self.concurrent_builders)
        if not self.quiet:
            timer.print_timing_report()

    def CleanOutputDirectories(self):
        for builder in self.builders:
            log.warning('Cleaning %s', builder.output_dir)
            if os.path.exists(builder.output_dir):
                shutil.rmtree(builder.output_dir)

        # any generated output was cleaned
        self.completed_steps.discard(BuildSteps.GENERATED)

    def CreateArtifactArchives(self, directory: str):
        log.info('Copying build artifacts to %s', directory)
        if not os.path.exists(directory):
            os.makedirs(directory)
        for builder in self.builders:
            # FIXME: builder subdir...
            builder.CompressArtifacts(os.path.join(
                directory, f'{builder.identifier}.tar.gz'))

    def CopyArtifactsTo(self, path: str):
        log.info('Copying build artifacts to %s', path)
        if not os.path.exists(path):
            os.makedirs(path)

        for builder in self.builders:
            # FIXME: builder subdir...
            builder.CopyArtifacts(os.path.join(path, builder.identifier))

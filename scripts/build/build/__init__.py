import logging
import os
import shutil
import time
from enum import Enum, auto
from multiprocessing.pool import ThreadPool
from typing import Optional, Sequence

from builders.builder import BuilderOptions

from .targets import BUILD_TARGETS, BuildTarget


class BuildSteps(Enum):
    GENERATED = auto()


class BuildTimer:
    def __init__(self):
        self._total_start_time = None
        self._total_end_time = None
        self._build_times = {}

    def time_builds(self, builders):
        self._total_start_time = time.time()
        for builder in builders:
            start_time = time.time()
            builder.build()
            end_time = time.time()
            self._build_times[builder.identifier] = end_time - start_time
        self._total_end_time = time.time()

    def print_timing_report(self):
        total_time = self._total_end_time - self._total_start_time
        logging.info("Build Time Summary:")
        for target, duration in self._build_times.items():
            logging.info(f"  - {target}: {self._format_duration(duration)}")
        logging.info(f"Total build time: {self._format_duration(total_time)}")

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

    def __init__(self, runner, repository_path: str, output_prefix: str, verbose: bool, ninja_jobs: int):
        self.builders = []
        self.runner = runner
        self.repository_path = repository_path
        self.output_prefix = output_prefix
        self.verbose = verbose
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
                builder = choice.Create(target, self.runner, self.repository_path,
                                        self.output_prefix, self.verbose, self.ninja_jobs,
                                        options)
                if builder:
                    self.builders.append(builder)
                    found_choice = choice
                    # assume a single match. we do not support wildcards
                    break

            if found_choice is None:
                logging.error(f"Target '{target}' could not be found. Nothing executed for it")
                continue

            if found_choice.isUnifiedBuild:
                # we want to ensure identical settings across builds. For now ensure that
                # variants are identical
                variants = '-'.join([x.name for x in found_choice.modifiers])
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

        for builder in self.builders:
            logging.info('Generating %s', builder.output_dir)
            builder.generate()

        self.completed_steps.add(BuildSteps.GENERATED)

    def Build(self):
        self.Generate()

        timer = BuildTimer()
        timer.time_builds(self.builders)
        timer.print_timing_report()

    def CleanOutputDirectories(self):
        for builder in self.builders:
            logging.warning('Cleaning %s', builder.output_dir)
            if os.path.exists(builder.output_dir):
                shutil.rmtree(builder.output_dir)

        # any generated output was cleaned
        self.completed_steps.discard(BuildSteps.GENERATED)

    def CreateArtifactArchives(self, directory: str):
        logging.info('Copying build artifacts to %s', directory)
        if not os.path.exists(directory):
            os.makedirs(directory)
        for builder in self.builders:
            # FIXME: builder subdir...
            builder.CompressArtifacts(os.path.join(
                directory, f'{builder.identifier}.tar.gz'))

    def CopyArtifactsTo(self, path: str):
        logging.info('Copying build artifacts to %s', path)
        if not os.path.exists(path):
            os.makedirs(path)

        for builder in self.builders:
            # FIXME: builder subdir...
            builder.CopyArtifacts(os.path.join(path, builder.identifier))

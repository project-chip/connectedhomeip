import logging
import os
import shutil

from enum import Enum, auto
from typing import Sequence

from .targets import Platform, Board, Application
from .factory import BuilderFactory, TargetRelations


def CommaSeparate(items) -> str:
    return ', '.join([x.ArgName for x in items])


# Supported platforms/boards/apps for generation/compilation
# note that not all combinations are supported, however command-line will
# accept all combinations
PLATFORMS = [x.ArgName for x in Platform]
BOARDS = [x.ArgName for x in Board]
APPLICATIONS = [x.ArgName for x in Application]


class BuildSteps(Enum):
    GENERATED = auto()


class Context:
    """Represents a grouped list of platform/board/app builders to use

         to generate make/ninja instructions and to compile.
      """

    def __init__(self, runner, repository_path: str, output_prefix: str):
        self.builders = []
        self.builder_factory = BuilderFactory(runner, repository_path,
                                              output_prefix)
        self.completed_steps = set()

    def SetupBuilders(self, platforms: Sequence[Platform],
                      boards: Sequence[Board],
                      applications: Sequence[Application],
                      enable_flashbundle: bool):
        """Configures internal builders for the given platform/board/app combination.

            Handles smart default selection, so that users only need to specify
            part of platform/board/application information and the method tries
            to automatically deduce the rest of the arguments.
            """
        if not platforms and not boards:
            if applications:
                platforms = set().union(*[
                    TargetRelations.PlatformsForApplication(app) for app in applications
                ])
            else:
                # when nothing is specified, start with a default host build
                platforms = [Platform.HOST]

        # at this point, at least one of 'platforms' or 'boards' is non-empty
        if not boards:
            boards = set().union(*[
                TargetRelations.BoardsForPlatform(platform) for platform in platforms
            ])
        elif not platforms:
            platforms = set().union(
                *[TargetRelations.PlatformsForBoard(board) for board in boards])

        if not applications:
            applications = set().union(*[
                TargetRelations.ApplicationsForPlatform(platform)
                for platform in platforms
            ])

        platforms = set(platforms)
        boards = set(boards)
        applications = set(applications)

        logging.info('Platforms being built: %s', CommaSeparate(platforms))
        logging.info('Boards being built: %s', CommaSeparate(boards))
        logging.info('Applications being built: %s',
                     CommaSeparate(applications))

        # Sanity check: ensure all input arguments generate at least an output
        platforms_with_builders = set()
        boards_with_builders = set()
        applications_with_builders = set()

        for platform in sorted(platforms):
            for board in sorted(boards):
                for application in sorted(applications):
                    builder = self.builder_factory.Create(
                        platform, board, application, enable_flashbundle=enable_flashbundle)
                    if not builder:
                        logging.debug('Builder not supported for tuple %s/%s/%s', platform,
                                      board, application)
                        continue

                    self.builders.append(builder)
                    platforms_with_builders.add(platform)
                    boards_with_builders.add(board)
                    applications_with_builders.add(application)

        if platforms != platforms_with_builders:
            logging.warn('Platforms without build output: %s',
                         CommaSeparate(platforms.difference(platforms_with_builders)))

        if boards != boards_with_builders:
            logging.warn('Boards without build output: %s',
                         CommaSeparate(boards.difference(boards_with_builders)))

        if applications != applications_with_builders:
            logging.warn(
                'Applications without build output: %s',
                CommaSeparate(applications.difference(applications_with_builders)))

        # whenever builders change, assume generation is required again
        self.completed_steps.discard(BuildSteps.GENERATED)

    def Generate(self):
        """Performs a build generation IFF code generation has not yet been performed."""
        if BuildSteps.GENERATED in self.completed_steps:
            return

        for builder in self.builders:
            logging.info('Generating %s', builder.output_dir)
            builder.generate()

        self.completed_steps.add(BuildSteps.GENERATED)

    def Build(self):
        self.Generate()

        for builder in self.builders:
            logging.info('Building %s', builder.output_dir)
            builder.build()

    def CleanOutputDirectories(self):
        for builder in self.builders:
            logging.warn('Cleaning %s', builder.output_dir)
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

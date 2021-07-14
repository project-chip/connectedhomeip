#!/usr/bin/env -S python3 -B

import coloredlogs
import click
import logging
import os
import sys

sys.path.append(os.path.abspath(os.path.dirname(__file__)))

import build

# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    'debug': logging.DEBUG,
    'info': logging.INFO,
    'warn': logging.WARN,
    'fatal': logging.FATAL,
}


def ValidateRepoPath(context, parameter, value):
  """Validates that the given path looks like a valid chip repository checkout."""
  for name in ['BUILD.gn', '.gn', os.path.join('scripts', 'bootstrap.sh')]:
    expected_file = os.path.join(value, name)
    if not os.path.exists(expected_file):
      raise click.BadParameter(
          "'%s' does not look like a valid repository path: %s not found." %
          (value, expected_file))
  return value


@click.group(chain=True)
@click.option(
    '--log-level',
    default='INFO',
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    help='Determines the verbosity of script output.')
@click.option(
    '--platform',
    default=[],
    type=click.Choice(build.PLATFORMS + ['all'], case_sensitive=False),
    multiple=True,
    help='Platform to use for compilation. Empty will default to a linux host build'
)
@click.option(
    '--board',
    default=[],
    type=click.Choice(build.BOARDS, case_sensitive=False),
    multiple=True,
    help='Specific board to compile for. Empty will use --platform to determine suitable boards.'
)
@click.option(
    '--app',
    default=[],
    type=click.Choice(build.APPLICATIONS, case_sensitive=False),
    multiple=True,
    help='What example application to build. Empty will find suitable applications.'
)
@click.option(
    '--repo',
    default='.',
    callback=ValidateRepoPath,
    help='Path to the root of the CHIP SDK repository checkout.')
@click.option(
    '--out-prefix',
    default='./out',
    type=click.Path(file_okay=False, resolve_path=True),
    help='Prefix for the generated file output.')
@click.option(
    '--clean',
    default=False,
    is_flag=True,
    help='Clean output directory before running the command')
@click.pass_context
def main(context, log_level, platform, board, app, repo, out_prefix, clean):
  # Ensures somewhat pretty logging of what is going on
  coloredlogs.install(
      level=__LOG_LEVELS__[log_level],
      fmt='%(asctime)s %(name)s %(levelname)-7s %(message)s')

  if not 'PW_PROJECT_ROOT' in os.environ:
    raise click.UsageError("""
PW_PROJECT_ROOT not in current environment.

Please make sure you `source scripts/bootstrap.sh` or `source scripts/activate.sh`
before running this script.
""".strip())

  # Support an 'all platforms' choice
  if 'all' in platform:
    platform = build.PLATFORMS

  context.obj = build.Context(repository_path=repo, output_prefix=out_prefix)
  context.obj.SetupBuilders(
      platforms=[build.Platform.FromArgName(name) for name in platform],
      boards=[build.Board.FromArgName(name) for name in board],
      applications=[build.Application.FromArgName(name) for name in app])

  if clean:
    context.obj.CleanOutputDirectories()


@main.command(
    'gen', help='Generate ninja/makefiles (but does not run the compilation)')
@click.pass_context
def cmd_generate(context):
  context.obj.Generate()


@main.command('build', help='generate and run ninja/make as needed to compile')
@click.option(
    '--copy-artifacts-to',
    default=None,
    type=click.Path(file_okay=False, resolve_path=True),
    help='Prefix for the generated file output.')
@click.pass_context
def cmd_build(context, copy_artifacts_to):
  context.obj.Build()

  if copy_artifacts_to:
    context.obj.CopyArtifactsTo(copy_artifacts_to)


if __name__ == '__main__':
  main()

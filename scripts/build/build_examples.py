#!/usr/bin/env -S python3 -B

# Copyright (c) 2021 Project CHIP Authors
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

import logging
import os
import sys

import click
import coloredlogs

import build
from builders.builder import BuilderOptions
from runner import PrintOnlyRunner, ShellRunner

sys.path.append(os.path.abspath(os.path.dirname(__file__)))


# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    'debug': logging.DEBUG,
    'info': logging.INFO,
    'warn': logging.WARN,
    'fatal': logging.FATAL,
}


def CommaSeparate(items) -> str:
    return ', '.join([x for x in items])


def ValidateRepoPath(context, parameter, value):
    """
    Validates that the given path looks like a valid chip repository checkout.
    """
    if value.startswith('/TEST/'):
        # Hackish command to allow for unit testing
        return value

    for name in ['BUILD.gn', '.gn', os.path.join('scripts', 'bootstrap.sh')]:
        expected_file = os.path.join(value, name)
        if not os.path.exists(expected_file):
            raise click.BadParameter(
                ("'%s' does not look like a valid repository path: "
                 "%s not found.") % (value, expected_file))
    return value


@click.group(chain=True)
@click.option(
    '--log-level',
    default='INFO',
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    help='Determines the verbosity of script output.')
@click.option(
    '--target',
    default=[],
    multiple=True,
    help='Build target(s)'
)
@click.option(
    '--enable-flashbundle',
    default=False,
    is_flag=True,
    help='Also generate the flashbundles for the app.'
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
    '--pregen-dir',
    default=None,
    type=click.Path(file_okay=False, resolve_path=True),
    help='Directory where generated files have been pre-generated.')
@click.option(
    '--clean',
    default=False,
    is_flag=True,
    help='Clean output directory before running the command')
@click.option(
    '--dry-run',
    default=False,
    is_flag=True,
    help='Only print out shell commands that would be executed')
@click.option(
    '--dry-run-output',
    default="-",
    type=click.File("wt"),
    help='Where to write the dry run output')
@click.option(
    '--no-log-timestamps',
    default=False,
    is_flag=True,
    help='Skip timestaps in log output')
@click.option(
    '--pw-command-launcher',
    help=(
        'Set pigweed command launcher. E.g.: "--pw-command-launcher=ccache" '
        'for using ccache when building examples.'))
@click.pass_context
def main(context, log_level, target, repo,
         out_prefix, pregen_dir, clean, dry_run, dry_run_output, enable_flashbundle,
         no_log_timestamps, pw_command_launcher):
    # Ensures somewhat pretty logging of what is going on
    log_fmt = '%(asctime)s %(levelname)-7s %(message)s'
    if no_log_timestamps:
        log_fmt = '%(levelname)-7s %(message)s'
    coloredlogs.install(level=__LOG_LEVELS__[log_level], fmt=log_fmt)

    if 'PW_PROJECT_ROOT' not in os.environ:
        raise click.UsageError("""
PW_PROJECT_ROOT not set in the current environment.

Please make sure you `source scripts/bootstrap.sh` or `source scripts/activate.sh`
before running this script.
""".strip())

    if dry_run:
        runner = PrintOnlyRunner(dry_run_output, root=repo)
    else:
        runner = ShellRunner(root=repo)

    requested_targets = set([t.lower() for t in target])
    logging.info('Building targets: %s', CommaSeparate(requested_targets))

    context.obj = build.Context(
        repository_path=repo, output_prefix=out_prefix, runner=runner)
    context.obj.SetupBuilders(targets=requested_targets, options=BuilderOptions(
        enable_flashbundle=enable_flashbundle,
        pw_command_launcher=pw_command_launcher,
        pregen_dir=pregen_dir,
    ))

    if clean:
        context.obj.CleanOutputDirectories()


@main.command(
    'gen', help='Generate ninja/makefiles (but does not run the compilation)')
@click.pass_context
def cmd_generate(context):
    context.obj.Generate()


@main.command(
    'targets',
    help=('List the targets that would be generated/built given '
          'the input arguments'))
@click.option(
    '--expand',
    default=False,
    is_flag=True,
    help='Expand all possible targets rather than the shorthand string')
@click.pass_context
def cmd_targets(context, expand):
    for target in build.targets.BUILD_TARGETS:
        if expand:
            build.target.report_rejected_parts = False
            for s in target.AllVariants():
                print(s)
        else:
            print(target.HumanString())


@main.command('build', help='generate and run ninja/make as needed to compile')
@click.option(
    '--copy-artifacts-to',
    default=None,
    type=click.Path(file_okay=False, resolve_path=True),
    help='Prefix for the generated file output.')
@click.option(
    '--create-archives',
    default=None,
    type=click.Path(file_okay=False, resolve_path=True),
    help='Prefix of compressed archives of the generated files.')
@click.pass_context
def cmd_build(context, copy_artifacts_to, create_archives):
    context.obj.Build()

    if copy_artifacts_to:
        context.obj.CopyArtifactsTo(copy_artifacts_to)

    if create_archives:
        context.obj.CreateArtifactArchives(create_archives)


if __name__ == '__main__':
    main(auto_envvar_prefix='CHIP')

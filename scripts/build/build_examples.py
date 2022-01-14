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

from glob_matcher import GlobMatcher
from runner import PrintOnlyRunner, ShellRunner

import build
import coloredlogs
import click
import logging
import os
import sys

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
    """Validates that the given path looks like a valid chip repository checkout."""
    if value.startswith('/TEST/'):
        # Hackish command to allow for unit testing
        return value

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
    '--target',
    default=['all'],
    type=click.Choice(
        ['all'] + [t.name for t in build.ALL_TARGETS], case_sensitive=False),
    multiple=True,
    help='Build target(s). Note that "all" includes glob blacklisted targets'
)
@click.option(
    '--target-glob',
    default=None,
    help='Glob matching for targets to include'
)
@click.option(
    '--skip-target-glob',
    default=None,
    help='Glob matching for targets to explicitly exclude'
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
@click.pass_context
def main(context, log_level, target, target_glob, skip_target_glob, repo, out_prefix, clean,
         dry_run, dry_run_output, enable_flashbundle, no_log_timestamps):
    # Ensures somewhat pretty logging of what is going on
    log_fmt = '%(asctime)s %(levelname)-7s %(message)s'
    if no_log_timestamps:
        log_fmt = '%(levelname)-7s %(message)s'
    coloredlogs.install(level=__LOG_LEVELS__[log_level], fmt=log_fmt)

    if not 'PW_PROJECT_ROOT' in os.environ:
        raise click.UsageError("""
PW_PROJECT_ROOT not set in the current environment.

Please make sure you `source scripts/bootstrap.sh` or `source scripts/activate.sh`
before running this script.
""".strip())

    if dry_run:
        runner = PrintOnlyRunner(dry_run_output, root=repo)
    else:
        runner = ShellRunner(root=repo)

    if 'all' in target:
        # NOTE: ALL includes things that are glob blacklisted (so that 'targets' works and
        # displays all)
        targets = build.ALL_TARGETS
    else:
        requested_targets = set([t.lower for t in target])
        targets = [
            target for target in build.ALL_TARGETS if target.name.lower in requested_targets]

        actual_targes = set([t.name.lower for t in targets])
        if requested_targets != actual_targes:
            logging.error('Targets not found: %s',
                          CommaSeparate(actual_targes))

    if target_glob:
        matcher = GlobMatcher(target_glob)
        targets = [t for t in targets if matcher.matches(
            t.name) and not t.IsGlobBlacklisted]

    if skip_target_glob:
        matcher = GlobMatcher(skip_target_glob)
        targets = [t for t in targets if not matcher.matches(t.name)]

    # force consistent sorting
    targets.sort(key=lambda t: t.name)
    logging.info('Building targets: %s',
                 CommaSeparate([t.name for t in targets]))

    context.obj = build.Context(
        repository_path=repo, output_prefix=out_prefix, runner=runner)
    context.obj.SetupBuilders(
        targets=targets, enable_flashbundle=enable_flashbundle)

    if clean:
        context.obj.CleanOutputDirectories()


@main.command(
    'gen', help='Generate ninja/makefiles (but does not run the compilation)')
@click.pass_context
def cmd_generate(context):
    context.obj.Generate()


@main.command(
    'targets', help='List the targets that would be generated/built given the input arguments')
@click.pass_context
def cmd_generate(context):
    for builder in context.obj.builders:
        if builder.target.IsGlobBlacklisted:
            print("%s (NOGLOB: %s)" %
                  (builder.target.name, builder.target.GlobBlacklistReason))
        else:
            print(builder.target.name)


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
    main()

#!/usr/bin/env python3

#
# Copyright (c) 2020 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import argparse
import coloredlogs
import csv
import datetime
import github
import github_fetch_artifacts
import io
import logging
import os
import re
import stat
import subprocess
import traceback
import zipfile

LOG_KEEP_DAYS = 3
BINARY_KEEP_DAYS = 30

# Count is reasonably large because each build has multiple artifacts
# Currently (Sep 2020) each build has 4 artifacts:
#   gn-nrf, gn-linux, examples-esp32, example-nrf
#
# We should eventually remove the non-gn version to save space.
BINARY_MAX_COUNT = 80


class SectionChange:
    """Describes delta changes to a specific section"""

    def __init__(self, section, fileChange, vmChange):
        self.section = section
        self.fileChange = fileChange
        self.vmChange = vmChange


class ComparisonResult:
    """Comparison results for an entire file"""

    def __init__(self, name):
        self.fileName = name
        self.sectionChanges = []


SECTIONS_TO_WATCH = set(
    ['.rodata', '.text', '.flash.rodata', '.flash.text', '.bss', '.data'])


def filesInDirectory(dirName):
    """Get all the file names in the specified directory."""
    for name in os.listdir(dirName):
        mode = os.stat(os.path.join(dirName, name)).st_mode
        if stat.S_ISREG(mode):
            yield name


def writeFileBloatReport(f, baselineName, buildName):
    """Generate a bloat report diffing a baseline file with a build output file."""
    logging.info('Running bloaty diff between %s and %s',
                 baselineName, buildName)
    f.write('Comparing %s and %s:\n\n' % (baselineName, buildName))

    result = subprocess.run(
        ['bloaty', '--csv', buildName, '--', baselineName],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )

    if result.returncode != 0:
        logging.warning('Bloaty execution failed: %d', result.returncode)
        f.write('BLOAT EXECUTION FAILED WITH CODE %d:\n' % result.returncode)

    content = result.stdout.decode('utf8')

    f.write(content)
    f.write('\n')

    result = ComparisonResult(os.path.basename(buildName))
    try:
        reader = csv.reader(io.StringIO(content))

        for row in reader:
            section, vm, f = row
            if (section in SECTIONS_TO_WATCH) or (vm not in ['0', 'vmsize']):
                result.sectionChanges.append(
                    SectionChange(section, int(f), int(vm)))
    except Exception:
        pass

    return result


def generateBloatReport(outputFileName,
                        baselineDir,
                        buildOutputDir,
                        title='BLOAT REPORT'):
    """Generates a bloat report fo files betwen two diferent directories."""
    logging.info('Generating bloat diff report between %s and %s', baselineDir,
                 buildOutputDir)
    with open(outputFileName, 'wt') as f:
        f.write(title + '\n\n')

        baselineNames = set([name for name in filesInDirectory(baselineDir)])
        outputNames = set([name for name in filesInDirectory(buildOutputDir)])

        baselineOnly = baselineNames - outputNames
        if baselineOnly:
            logging.warning(
                'Some files only exist in the baseline: %r', baselineOnly)
            f.write('Files found only in the baseline:\n    ')
            f.write('\n    %s'.join(baselineOnly))
            f.write('\n\n')

        outputOnly = outputNames - baselineNames
        if outputOnly:
            logging.warning('Some files only exist in the build output: %r',
                            outputOnly)
            f.write('Files found only in the build output:\n    ')
            f.write('\n    %s'.join(outputOnly))
            f.write('\n\n')

        results = []
        for name in (baselineNames & outputNames):
            results.append(
                writeFileBloatReport(f, os.path.join(baselineDir, name),
                                     os.path.join(buildOutputDir, name)))
        return results


def sendFileAsPrComment(job_name, filename, gh_token, gh_repo, gh_pr_number,
                        compare_results, base_sha):
    """Generates a PR comment containing the specified file content."""

    logging.info('Uploading report to "%s", PR %d', gh_repo, gh_pr_number)

    rawText = open(filename, 'rt').read()

    # a consistent title to help identify obsolete comments
    titleHeading = 'Size increase report for "{jobName}"'.format(
        jobName=job_name)

    api = github.Github(gh_token)
    repo = api.get_repo(gh_repo)
    pull = repo.get_pull(gh_pr_number)

    for comment in pull.get_issue_comments():
        if not comment.body.startswith(titleHeading):
            continue
        logging.info(
            'Removing obsolete comment with heading "%s"', (titleHeading))

        comment.delete()

    if all(len(file.sectionChanges) == 0 for file in compare_results):
        logging.info('No results to report')
        return

    compareTable = 'File | Section | File | VM\n---- | ---- | ----- | ---- \n'
    for file in compare_results:
        for change in file.sectionChanges:
            compareTable += '{0} | {1} | {2} | {3}\n'.format(file.fileName,
                                                             change.section,
                                                             change.fileChange,
                                                             change.vmChange)

    # NOTE: PRs are issues with attached patches, hence the API naming
    pull.create_issue_comment("""{title} from {baseSha}

  {table}

<details>
  <summary>Full report output</summary>

```
{rawReportText}
```

</details>
""".format(title=titleHeading, baseSha=base_sha, table=compareTable, rawReportText=rawText))


def getPullRequestBaseSha(githubToken,  githubRepo, pullRequestNumber):
    """Figure out the SHA for the base of a pull request"""
    api = github.Github(githubToken)
    repo = api.get_repo(githubRepo)
    pull = repo.get_pull(pullRequestNumber)

    return pull.base.sha


def cleanDir(name):
    """Ensures a clean directory with the given name exists. Only handles files"""
    if os.path.exists(name):
        for fname in os.listdir(name):
            path = os.path.join(name, fname)
            if os.path.isfile(path):
                os.unlink(path)
    else:
        os.mkdir(name)


def downloadArtifact(artifact, dirName):
    """Extract an artifact into a directory."""
    zipFile = zipfile.ZipFile(io.BytesIO(artifact.downloadBlob()), 'r')
    logging.info('Extracting zip file to %r' % dirName)
    zipFile.extractall(dirName)


def main():
    """Main task if executed standalone."""
    parser = argparse.ArgumentParser(
        description='Fetch master build artifacts.')
    parser.add_argument(
        '--output-dir',
        type=str,
        default='.',
        help='Where to download the artifacts')
    parser.add_argument(
        '--github-api-token',
        type=str,
        help='Github API token to upload the report as a comment')
    parser.add_argument(
        '--github-repository', type=str, help='Repository to use for PR comments')
    parser.add_argument(
        '--log-level',
        default=logging.INFO,
        type=lambda x: getattr(logging, x),
        help='Configure the logging level.')
    args = parser.parse_args()

    # Ensures somewhat pretty logging of what is going on
    logging.basicConfig(
        level=args.log_level,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    coloredlogs.install()

    if not args.github_api_token:
        logging.error(
            'Required arguments missing: github api token is required.')
        return

    # all known artifacts
    artifacts = [a for a in github_fetch_artifacts.getAllArtifacts(
        args.github_api_token, args.github_repository)]

    # process newest artifacts first
    artifacts.sort(key=lambda x: x.created_at, reverse=True)

    current_time = datetime.datetime.now()
    seen_names = set()
    pull_artifact_re = re.compile('^(.*)-pull-(\\d+)$')
    binary_count = 0
    for a in artifacts:
        # logs cleanup after 3 days
        is_log = a.name.endswith('-logs')

        if not is_log:
            binary_count = binary_count + 1

        need_delete = False
        if (current_time - a.created_at).days > BINARY_KEEP_DAYS:
            # Do not keep binary builds forever
            need_delete = True
        elif not is_log and binary_count > BINARY_MAX_COUNT:
            # Keep a maximum number of binary packages
            need_delete = True
        elif is_log and (current_time - a.created_at).days > LOG_KEEP_DAYS:
            # Logs are kept even shorter
            need_delete = True

        if need_delete:
            logging.info('Old artifact: %s from %r' % (a.name, a.created_at))
            a.delete()
            continue

        if a.name.endswith('-logs'):
            # logs names are duplicate, however that is fine
            continue

        if a.name in seen_names:
            logging.info('Artifact name already seen before: %s' % a.name)
            a.delete()
            continue

        seen_names.add(a.name)

        m = pull_artifact_re.match(a.name)
        if not m:
            logging.info('Non-PR artifact found: %r from %r' %
                         (a.name, a.created_at))
            continue

        prefix = m.group(1)
        pull_number = int(m.group(2))

        logging.info('Processing PR %s via artifact %r' %
                     (pull_number, a.name))

        try:
            base_sha = getPullRequestBaseSha(
                args.github_api_token, args.github_repository, pull_number)

            base_artifact_name = '%s-%s' % (prefix, base_sha)

            base_artifacts = [
                v for v in artifacts if v.name == base_artifact_name]
            if len(base_artifacts) != 1:
                raise Exception('Did not find exactly one artifact for %s: %r' % (
                    base_artifact_name, [v.name for v in base_artifacts]))

            b = base_artifacts[0]

            logging.info('Diff will be against artifact %r' % b.name)

            aOutput = os.path.join(args.output_dir, 'pull_artifact')
            bOutput = os.path.join(args.output_dir, 'master_artifact')

            cleanDir(aOutput)
            cleanDir(bOutput)

            downloadArtifact(a, aOutput)
            downloadArtifact(b, bOutput)

            report_name = os.path.join(aOutput, 'report.csv')

            results = generateBloatReport(report_name, bOutput, aOutput)

            sendFileAsPrComment(prefix, report_name, args.github_api_token,
                                args.github_repository, pull_number, results, base_sha)

            # If running over a top level directory, ensure git sees no output
            cleanDir(aOutput)
            cleanDir(bOutput)

            # Output processed.
            a.delete()

        except Exception as e:
            tb = traceback.format_exc()
            logging.warning('Failed to process bloat report: %s', tb)


if __name__ == '__main__':
    # execute only if run as a script
    main()

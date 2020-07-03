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
import github
import io
import logging
import os
import re
import stat
import subprocess


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
  logging.info('Running bloaty diff between %s and %s', baselineName, buildName)
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
        result.sectionChanges.append(SectionChange(section, int(f), int(vm)))
  except:
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
      logging.warning('Some files only exist in the baseline: %r', baselineOnly)
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
                        compare_results):
  """Generates a PR comment conaining the specified file content."""

  logging.info('Uploading report to "%s", PR %d', gh_repo, gh_pr_number)

  rawText = open(filename, 'rt').read()

  # a consistent title to help identify obsolete comments
  titleHeading = 'Size increase report for "{jobName}"'.format(jobName=job_name)

  api = github.Github(gh_token)
  repo = api.get_repo(gh_repo)
  pull = repo.get_pull(gh_pr_number)

  for comment in pull.get_issue_comments():
    if not comment.body.startswith(titleHeading):
      continue
    logging.info('Removing obsolete comment with heading "%s"', (titleHeading))

    comment.delete()

  compareTable = 'File | Section | File | VM\n---- | ---- | ----- | ---- \n'
  for file in compare_results:
    for change in file.sectionChanges:
      compareTable += '{0} | {1} | {2} | {3}\n'.format(file.fileName,
                                                       change.section,
                                                       change.fileChange,
                                                       change.vmChange)

  # NOTE: PRs are issues with attached patches, hence the API naming
  pull.create_issue_comment("""{title}

  {table}

<details>
  <summary>Full report output</summary>

```
{rawReportText}
```

</details>
""".format(title=titleHeading, table=compareTable, rawReportText=rawText))


def extractPrNumberFromRef(refStr):
  logging.info('EXTRACTING REF FROM "%s"', refStr)
  match = re.compile('^refs/pull/(\\d*)/merge').match(refStr)

  if match:
    return int(match.group(1))

  logging.warning('Cannot extract PR number from ref: "%s"', refStr)

  return None


def main():
  """Main task if executed standalone."""
  parser = argparse.ArgumentParser(description='Fetch master build artifacts.')
  parser.add_argument('--token', type=str, help='API token to use')
  parser.add_argument(
      '--job', type=str, help='Name of the job generating the report')
  parser.add_argument(
      '--artifact-download-dir',
      type=str,
      default='.',
      help='Where to download the artifacts')
  parser.add_argument(
      '--build-output-dir',
      type=str,
      default='.',
      help='Generated build files directory to use to compare for bloat')
  parser.add_argument(
      '--report-file',
      type=str,
      default='report.csv',
      help='Report file output name')
  parser.add_argument(
      '--github-api-token',
      type=str,
      help='Github API token to upload the report as a comment')
  parser.add_argument(
      '--github-repository', type=str, help='Repository to use for PR comments')
  parser.add_argument(
      '--github-ref',
      type=str,
      default=None,
      help='Github action ref, of format refs/pull/:prNumber/merge')
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

  if not args.token:
    logging.error('Required arguments missing: token is required.')
    return

  if not args.job:
    logging.error('Required arguments missing: job is required.')
    return

  comment_pr_number = extractPrNumberFromRef(args.github_ref)

  # TODO(andreilitvin): this needs refactoring and cleanup to fetch artifacts
  # and process as needed

  #
  # try:
  #   ci_fetch_artifacts.fetchArtifactsForJob(args.token, args.job,
  #                                           args.artifact_download_dir)
  # except Exception as e:
  #   logging.warning('Failed to fetch artifacts: %r', e)
  #
  # compareResults = generateBloatReport(
  #     args.report_file,
  #     args.artifact_download_dir,
  #     args.build_output_dir,
  #     title="Bloat report for job '%s'" % args.job)
  #
  if args.github_api_token and args.github_repository and comment_pr_number:
    logging.warning('SEND REPORT DISABLED FOR NOW.')
  #   sendFileAsPrComment(args.job, args.report_file, args.github_api_token,
  #                       args.github_repository,
  #                       comment_pr_number, compareResults)

  logging.warning('NOT YET PORTED OVER/IMPLEMENTED')


if __name__ == '__main__':
  # execute only if run as a script
  main()

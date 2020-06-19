#!/usr/bin/env python3

import argparse
import attr
import coloredlogs
import github
import logging
import os
import stat
import subprocess

import ci_fetch_artifacts


def filesInDirectory(dirName):
  """Get all the file names in the specified directory."""
  for name in os.listdir(dirName):
    mode = os.stat(os.path.join(dirName, name)).st_mode
    if stat.S_ISREG(mode):
      yield name


def writeFileBloatReport(f, baselineName, buildName):
  """Generate a bloat report diffing a baseline file with a build output file."""
  logging.info('Running bloaty diff between %s and %s', baselineName, buildName)
  f.write('Bloat difference between %s and %s:\n\n' % (baselineName, buildName))

  result = subprocess.run(
      ['bloaty', buildName, '--', baselineName],
      stdout=subprocess.PIPE,
      stderr=subprocess.STDOUT,
  )

  if result.returncode != 0:
    logging.warning('Bloaty execution failed: %d', result.returncode)
    f.write('BLOAT EXECUTION FAILED WITH CODE %d:\n' % result.returncode)

  f.write(result.stdout.decode('utf8'))
  f.write('\n')


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

    for name in (baselineNames & outputNames):
      writeFileBloatReport(f, os.path.join(baselineDir, name),
                           os.path.join(buildOutputDir, name))


def sendFileAsPrComment(job_name, filename, gh_token, gh_repo, gh_pr_number):
  """Generates a PR comment conaining the specified file content."""

  logging.info('Uploading report to "%s", PR %d' % (gh_repo, gh_pr_number))

  rawText = open(filename, 'rt').read()

  # a consistent title to help identify obsolete comments
  titleHeading = 'Size increase report for "{jobName}"'.format(jobName = job_name)

  api = github.Github(gh_token)
  repo = api.get_repo(gh_repo)
  pull = repo.get_pull(gh_pr_number)

  # TODO: 
  #   - close out obsolete comments (no more duplication). Use title to search through things.
  #   - Add a clear summary of size increase, by parsing the rawReportText

  # NOTE: PRs are issues with attached patches, hence the API naming
  pull.create_issue_comment('''{title}

<details>
  <summary>Full report output</summary>

```
{rawReportText}
```

</details>
'''.format(title=titleHeading, jobName=job_name, rawReportText=rawText))


def main():
  """Main task if executed standalone."""
  parser = argparse.ArgumentParser(description='Fetch master build artifacts.')
  parser.add_argument('--token', type=str, help='API token to use')
  parser.add_argument(
      '--job', type=str, help='From what job to fetch artifacts from')
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
      default='report.txt',
      help='From what job to fetch artifacts from')
  parser.add_argument(
      '--github-api-token',
      type=str,
      help='Github API token to upload the report as a comment')
  parser.add_argument(
      '--github-repository', type=str, help='Repository to use for PR comments')
  parser.add_argument(
      '--github-comment-pr-number',
      type=str,
      default=None,
      help='To what PR to comment in github')
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

  if not args.token or not args.job:
    logging.error(
        'Required arguments missing. Please specify at least job and token.')
    return

  try:
    ci_fetch_artifacts.fetchArtifactsForJob(args.token, args.job,
                                            args.artifact_download_dir)
  except Exception as e:
    logging.warning('Failed to fetch artifacts: %r' % e)

  generateBloatReport(
      args.report_file,
      args.artifact_download_dir,
      args.build_output_dir,
      title="Bloat report for job '%s'" % args.job)

  if args.github_api_token and args.github_repository and args.github_comment_pr_number:
    sendFileAsPrComment(
       args.job,
       args.report_file,
       args.github_api_token,
       args.github_repository,
       int(args.github_comment_pr_number),
    )

if __name__ == '__main__':
  # execute only if run as a script
  main()

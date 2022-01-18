#!/usr/bin/env python3

"""
Set up environment variables used to generate size report artifacts.

Takes a single argument, a JSON dictionary of the `github` context.
Typically run as:

```
    - name: Set up environment for size reports
      if: ${{ !env.ACT }}
      env:
        GH_CONTEXT: ${{ toJson(github) }}
      run: gh_sizes_environment.py "${GH_CONTEXT}"
```

Sets the following environment variables:

- `GH_EVENT_PR`         For a pull request, the PR number; otherwise 0.
- `GH_EVENT_HASH`       SHA of the commit under test.
- `GH_EVENT_PARENT`     SHA of the parent commit to which the commit under
                        test is applied.
- `GH_EVENT_REF`        The branch or tag ref that triggered the workflow run.
- `GH_EVENT_TIMESTAMP`  For `push` events only, the timestamp of the commit.
"""

import json
import os
import re
import subprocess
import sys

import dateutil.parser

github = json.loads(sys.argv[1])

commit = None
timestamp = None
ref = github['ref']

if github['event_name'] == 'pull_request':

    pr = github['event']['number']
    commit = github['event']['pull_request']['head']['sha']

    # Try to find the actual commit against which the current PR compares
    # by scraping the HEAD commit message.
    r = subprocess.run(['git', 'show', '--no-patch', '--format=%s', 'HEAD'],
                       capture_output=True, text=True, check=True)
    m = re.fullmatch('Merge [0-9a-f]+ into ([0-9a-f]+)', r.stdout)
    if m:
        parent = m.group(1)
    else:
        parent = github['event']['pull_request']['base']['sha']

elif github['event_name'] == 'push':

    commit = github['sha']
    parent = github['event']['before']
    timestamp = dateutil.parser.isoparse(
        github['event']['head_commit']['timestamp']).timestamp()

    # Try to find the PR being committed by scraping the commit message.
    m = re.search(r'\(#(\d+)\)', github['event']['head_commit']['message'])
    if m:
        pr = m.group(1)
    else:
        pr = 0

# Environment variables for subsequent workflow steps are set by
# writing to the file named by `$GITHUB_ENV`.

if commit is not None:
    env = os.environ.get('GITHUB_ENV')
    assert env
    with open(env, 'at') as out:
        print(f'GH_EVENT_PR={pr}', file=out)
        print(f'GH_EVENT_HASH={commit}', file=out)
        print(f'GH_EVENT_PARENT={parent}', file=out)
        print(f'GH_EVENT_REF={ref}', file=out)
        if timestamp:
            print(f'GH_EVENT_TIMESTAMP={timestamp}', file=out)

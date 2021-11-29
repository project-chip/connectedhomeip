#!/usr/bin/env python3

"""
Set up environment variables used to generate size report artifacts.

Takes a single argument, a JSON dictionary of the `github` context.
Typically run as:

```
    - name: Set up environment for size reports
      env:
        GH_CONTEXT: ${{ toJson(github) }}
      run: gh_sizes_environment.py "${GH_CONTEXT}"
```

Sets the following environment variables:

- `GH_EVENT_PR`     For a pull request, the PR number; otherwise 0.
- `GH_EVENT_HASH`   SHA of the commit under test.
- `GH_EVENT_PARENT` SHA of the parent commit to which the commit under
                    test is applied.
"""

import json
import os
import re
import subprocess
import sys

github = json.loads(sys.argv[1])

if github['event_name'] == 'pull_request':
    pr = github['event']['number']
    commit = github['event']['pull_request']['head']['sha']
    r = subprocess.run(['git', 'show', '--no-patch', '--format=%s', 'HEAD'],
                       capture_output=True, text=True, check=True)
    m = re.fullmatch('Merge [0-9a-f]+ into ([0-9a-f]+)', r.stdout)
    if m:
        parent = m.group(1)
    else:
        parent = github['event']['pull_request']['base']['sha']
else:
    pr = 0
    commit = github['sha']
    parent = github['event']['before']

# Environment variables for subsequent workflow steps are set by
# writing to the file named by `$GITHUB_ENV`.

env = os.environ.get('GITHUB_ENV')
assert env
with open(env, 'at') as out:
    print(f'GH_EVENT_PR={pr}', file=out)
    print(f'GH_EVENT_HASH={commit}', file=out)
    print(f'GH_EVENT_PARENT={parent}', file=out)

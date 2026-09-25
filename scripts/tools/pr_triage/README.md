# PR triage tools

Two scripts behind two agent skills, `.agents/skills/matter-pr-triage/` and
`.agents/skills/matter-pr-triage-issues/`. Each script gathers evidence from
GitHub, and from the checkout when there is one; the agent following the skill
reads it and writes the verdict. Neither script posts, comments, closes, labels
or changes anything on GitHub, and neither touches the working tree.

-   `matter_pr_triage.py` works through a window of old pull requests and, per
    pull request, records whether its goal is already met on the base branch and
    what reviving it would cost. The agent's report lands in
    `~/matter-pr-triage-reports/<owner>/<repo>/`.
-   `matter_pr_triage_issues.py` takes one pull request and finds the issues
    related to it, linked on GitHub or not, from a corpus of the repository's
    issues; for a pull request the triage found safe to close, also which of
    them close with it. It imports the first script, so the two stay in this
    folder together.

Both need only Python 3.8 or newer and an authenticated `gh`; no build
environment. Every flag is in [CLI.md](CLI.md); measured accuracy and the
recipes to repeat it are in [BENCHMARKS.md](BENCHMARKS.md).

Unit tests, no network:

```
python3 -m unittest discover scripts/tools/pr_triage/tests
```

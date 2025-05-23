# Guidelines for writing pull requests

## Short-form checklist

Within the matter SDK, we are prioritizing long term code maintainability and
ease of reviews of PRs. To create an easy to review pull request, ensure the
following items are met (and see below for details)

-   [ ] Pull request size
    -   Change focuses on a single aspect/bug/feature
    -   Strongly prefer small PRs (even if several of them are needed to achieve
        final goal)
-   [ ] Change is well tested
    -   testing is described in `### Testing` section in the pull request
        summary
    -   Strongly prefer automated tests to manual tests
-   [ ] Good Pull request summary/description
    -   Summary contains an accurate description of changes
    -   Summary contains sufficient context for reviewers
    -   Summary contains flash/ram overhead if changing core/common parts
-   [ ] CI should pass (i.e. green or yellow/waiting for review)
-   [ ] Avoid/limit force-pushes/merges with master

## Details and background on requirements

### Pull request size

Smaller patches are easier to review and force maintainability by enforcing
decoupling (i.e. they require code to be sufficiently separate for small changes
to be possible to begin with). The downside is that a full CI run is required
before merging (typically 2+ hours for a full run). At this time we prioritize
reviewer time and prefer small patches.

Patch size generally consider "updated" code, however we do not consider test
file changes (it is common for test files to be larger than the implementation)
nor generated files (generally files in `zzz_generated`, `*.matter` files or
files under a `/generated/` folder like for darwin/kotlin/java/python)

Change things that are related (e.g. code and documentation and tests), however
do not combine unrelated changes (like unrelated documentation typo fixes,
fixing multiple issues in one PR, implementing a full app withing first having
the skeleton app).

### Testing

We strongly prefer automated unit testing. Historically there are areas of the
SDK that lack sufficient testing and adding more test coverage is being worked
on. However this also means that a justification of "other code that is similar
has no tests, so this code needs no tests" does not apply. We require unit
testing for new code unless it is impossible to add unit tests (e.g. platform
drivers/integration and even then consider if test abstractions are possible in
some way).

Add testing details in a `### Testing` heading in the pull request summary -
there is a CI bot that checks for this. Within this section add the following
details:

-   if automated unit tests, brief mention like `added/updated unit tests` is
    sufficient. Thank you for adding automated unit tests and we accept this
    area to be brief

-   if automated integration tests, this can be brief as well saying
    `TC_*.yaml/py` tests this, also add a brief text on why unit testing was not
    possible as well as unit tests are faster to iterate on and execute.

-   if manual testing was done, include **detailed** information about the tests
    run (e.g. what chip-tool or `repl` commands were run) and the observed
    results. Also include an explanation why automated testing was NOT possible.
    This requirement is intentionally tedious to strongly encourage writing of
    automated tests. It is insufficient to reference an existing
    PR/document/plan/link and say "tested as described in XYZ".

#### Coverage

We are working on automated coverage, however in the mean time we would
appreciate some automated test coverage information in the PR summary: are only
happy paths covered? Any corner cases that are not/could not be covered?

We aim for around 85-90% coverage for automated testing.

### PR Summary/Description

Reviewers are likely to have less context than someone actively working on a PR.
Provide sufficient information for a reviewer to understand the change. Include:

-   brief information on what changed
-   brief information on **WHY** the change was made. Avoid just saying "Fix
    compile error" but rather add a example of the error seen and under want
    command. Avoid `Fixes #1234` as that requires the reviewer to open another
    issue and hope that the issue is well described. Have a brief description of
    the problem and fix anyway, even with an issue link.
-   Review context:
    -   if updating based on a test plan or spec issue, include the test plan or
        issue PR that this depends on
    -   clearly explain if the PR is based on in progress work (often for Spec
        issues)
-   If changing common code, check where any RAM/FLASH overhead comes from. You
    can use [size tooling](../scripts/tools/ELF_SIZE_TOOLING.md) to gather this
    information.

### PR updates post review

Try to make changes based on reviewer comment in one or few commits without any
"squash" or "merge with master". This makes it easier for reviewers to validate
that their comments were addressed.

Force-updates when squashing changes make review comments harder to track. Avoid
them as in the SDK we merge with "squash" for every PR so the history will stay
clean.

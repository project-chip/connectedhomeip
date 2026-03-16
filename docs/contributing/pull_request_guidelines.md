# Guidelines for writing pull requests

## Short-form checklist

Within the matter SDK, we are prioritizing long term code maintainability and
ease of reviews of PRs. To create an easy to review pull request, ensure the
following items are met (and see below for details)

-   [ ] Descriptive/clear title
-   [ ] Pull request size
    -   Change focuses on a single aspect/bug/feature
    -   Strongly prefer small PRs (even if several of them are needed to achieve
        final goal)
-   [ ] Change is well tested
    -   testing is described in `### Testing` section in the pull request
        summary (please include instructions and commands to test your PR)
    -   Strongly prefer automated tests to manual tests
-   [ ] Good Pull request summary/description
    -   Summary contains sufficient context for reviewers including links to
        relevant test plan and / or specification changes
    -   Summary contains flash/ram overhead if changing core/common parts
-   [ ] CI should pass (i.e. green or yellow/waiting for review)
-   [ ] Avoid force-pushes and squashing commits. By keeping your PR commit
        history you make it easier for reviewers to see the diff between
        versions and confirm the requested changes have been addressed. -
        `merge with master` is fine, however it is _NOT_ a requirement for PR
        merging. Only do this when needed (e.g. to fix a conflict) and not too
        frequently as it triggers a 2+ hour CI run every time.
-   [ ] Consistent style
    -   The overall rule is "make the code feel consistent" (i.e. when changing
        files keep existing rules consistent)
    -   can use the [style guilde](./style/CODING_STYLE_GUIDE.md) for reference
        (note that
        [keep style constistent rule](https://en.wikipedia.org/wiki/When_in_Rome,_do_as_the_Romans_do)
        is the first rule in this guide)

## Details and background on requirements

### Title formatting

Describe the change as a one-line in some descriptive manner. Add sufficient
context for a reader to understand what is improved. If platform-specific
consider adding the platform as a prefix, like `[Android]` or any other tags
that may be useful for quick filtering like `[TC-ABC-1.2]` to tag test changes.

Examples of descriptive titles:

-   `[Silabs] Fix compile of SiWx917 if LED and BUTTON are disabled`
-   `[Telink] Update build Dockerfile with new Zephyr SHA: c05c4.....`
-   `General Commissioning Cluster: use AttributeAccessInterface/CommandHandlerInterface for processing`
-   `Scenes Management/CopyScene: set access as manage instead of default to match the spec`
-   `Fix build errors due to ChipDeviceEvent default constructor not being available`
-   `Fix crash during DNSSD processing due to malformed packet`
-   `[NRF] Fix crash due to stack overflow during logging for PW-RPC builds`
-   `[TC-ABC-2.3] added new python test case based on test plan`
-   `[TC-ABC] migrate tests from yaml to python`

Examples of titles that are vague (not clear what the change is, one would need
to open the pull request for details or open additional issue in GitHub)

-   `Work on issue 1234`
-   `Fix android JniTypeWrappers`
-   `Fix segfault in BLE`
-   `Fix TC-ABC-1.2`
-   `Update Readme`

### Pull request size

Smaller patches are easier to review and force maintainability by enforcing
decoupling (i.e. they require code to be sufficiently separate for small changes
to be possible to begin with). The downside is that a full CI run is required
before merging (typically 2+ hours for a full run). At this time we prioritize
reviewer time and prefer small patches.

Patch size generally considers "updated" code, however we do not consider test
file changes (it is common for test files to be larger than the implementation)
nor generated files (generally files in `zzz_generated`, `*.matter` files or
files under a `/generated/` folder like for darwin/kotlin/java/python)

Change things that are related (e.g. code and documentation and tests), however
do not combine unrelated changes (like unrelated documentation typo fixes,
fixing multiple issues in one PR, implementing a full app without first having
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

-   If automated unit tests, a brief mention like `added/updated unit tests` is
    sufficient. Thank you for adding automated unit tests and we accept this
    area to be brief.

-   If automated integration tests, this can be brief as well saying
    `TC_*.yaml/py` tests this, also add a brief text on why unit testing was not
    possible as well as unit tests are faster to iterate on and execute.

-   If manual testing was done, include **detailed** information about the tests
    run (e.g. what chip-tool or `repl` commands were run) and the observed
    results. Also include an explanation why automated testing was NOT possible.
    This requirement is intentionally tedious to strongly encourage writing of
    automated tests. It is insufficient to reference an existing
    PR/document/plan/link and say "tested as described in XYZ".

-   Trivial/obvious change

    In rare cases the change is trivial (e.g. fixing a typo in a `Readme.md`).
    Scripts still require a `#### Testing` section however you can be brief like
    `N/A` or `checked new URL opens`. Note that these cases are rare - e.g.
    fixing a typo in an ID still requires some description on how you checked
    that the new ID takes effect.

#### Coverage

We are working on automated coverage, however in the meantime we would
appreciate some automated test coverage information in the PR summary: are only
happy paths covered? Any corner cases that are not/could not be covered?

We aim for around 85-90% coverage for automated testing.

### PR Summary/Description

Ensure that there is sufficient detail in issue summaries to make the content of
the PR clear:

Reviewers are likely to have less context than someone actively working on a PR.
Provide sufficient information for a reviewer to understand the change. Include:

-   a `TLDR` of the change content. This is a judgment call on details,
    generally you should include what was changed and why. If the change is
    trivial/short, this can be very short (i.e. "fixed typos" is perfectly
    acceptable, however if changing 100-1000s of lines, the areas of changes
    should be explained)
-   If a crash/error is fixed, explain the root cause and if the fix is not
    obvious (again, judgment call), explain why the given approach was taken.
-   Help the reviewer out with any notable information (specific platform
    issues, extra thoughts or requests for feedback or gotchas on tricky code,
    followup work or PR dependencies)
-   brief information on **WHY** the change was made. Avoid just saying "Fix
    compile error" but rather add a example of the error seen and under what
    command. Avoid `Fixes #1234` as that requires the reviewer to open another
    issue and hope that the issue is well described. Have a brief description of
    the problem and fix anyway, even with an issue link.
-   Review context:
    -   If updating based on a test plan or spec issue, include the test plan or
        issue PR that this depends on.
    -   Clearly explain if the PR is based on in progress work (often for Spec
        issues).
    -   Larger changes/features should include some design document link.
        Reviewers may not be familiar with discussions from the many tiger teams
        that work on the matter SDK.
-   If changing common code, check where any RAM/FLASH overhead comes from. You
    can use [size tooling](../scripts/tools/ELF_SIZE_TOOLING.md) to gather this
    information.
-   TIP: use the syntax of `Fixes #....` to mark issues completed on PR merge or
    use `#...` to reference issues that are addressed.
-   TIP: prefer adding some brief description (especially about the content of
    the changes) instead of just referencing an issue (helps reviewers get
    context faster without extra clicks).

### PR updates post review

Try to make changes based on reviewer comments in one or few commits without any
"squash" or "merge with master". This makes it easier for reviewers to validate
that their comments were addressed.

Force-updates when squashing changes make review comments harder to track. Avoid
them as in the SDK we merge with "squash" for every PR so the history will stay
clean.

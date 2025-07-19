# Reporting bugs

## Writing an effective bug report

When reporting a bug, start with the question
`What does a bug report need to tell the developer`.

Generally you want the following parts covered:

-   What is the problem

-   How can the developer reproduce the problem (to see it for themselves), to
    bisect when it was introduced or to find if it got fixed already.

-   At what point does the problem occur

-   What environment did this occur in

Make sure the above items are covered and the bug is easy to review and parse:

-   **Title** should clearly describe the problem. Bugs are often sorted from
    the issue list which only contains the title

-   **Logs** should generally be attachments (drag & drop or click on bottom bar
    when entering issue text) and not inline with the issue.

-   **Reproduction steps** and **environment** should be clearly highlighted. If
    running commands reproduce the issue (very common), the commands should be
    in a code block/script format.

### Describing the problem

Make sure the issue is obvious or provide a link explaining why the expected
result is not met.

Examples:

-   `(Core dump) seen` is obvious since there should be no core dumps

-   `Failure trying to read attribute X in cluster Y which is marked MANDATORY in the spec`
    references the spec and describes why attribute read should succeed.

-   `Failure trying to write attribute X in cluster Y, which is enabled since cluster FeatureMap enabled X and spec describes as writable.`
    references the spec and explicitly states that an optional attribute is
    enabled based on device status

-   `Running certification test TC-A-B-C (link included) fails at step 3: test case asks for command to succeed, I get ACCESS_DENIED instead`
    describes a pre-defined test case that is expected to pass but fails. Note
    that full link to the test description is needed (and should be covered by
    'how to reproduce' part)

Unless manually curated (e.g. few lines showing the problem), logs should be
always attachments and not inlined in the bug as the make the bug report too
long.

### Reproduction steps and when does the issue occur

Include all steps needed to reproduce the problem. Link any supporting
documentation.

If stating something of the form `TC-A-B-C step 4 fails` then there should be a
link to TC-A-B-C and ideally a list of the commands of each step since test
cases may change over time.

The bug report should contain all the information for a developer to reproduce
the issue without needing to have access to CHIP Test case repository (not
everyone does)

### Environment for reproduction

Assume that the developer will want to reproduce the issue and will try to
mirror your environment to a reasonable degree. For this, at a minimum the
platforms on which everything is running is needed.

Try to provide as much information as seems relevant. At a minimum this could
look like
`Failed to commission nrf board using chip-tool running on linux. Used build on SHA abcde...`.
This provides basic information (use nrf board, use chip-tool on linux, default
build) to get started. Beyond that, you can refine if more items seem relevant:

-   `Tested on TE9` or `Tested on interop branch xyz` gives a build reference
    point. Useful when branches/tags are used instead of master branch as
    development happens on master branch.

-   `Thread devices fail, tested with qpg and efr32` shows that this seems to be
    a general thread issue and developer can investigate on multiple of them

*   `Tested with avahi-build and it passes/fails` helps the developer with
    information of non-default builds that pass/fail to narrow down the problem

*   `Passes with darwin-framework-tool and repl but fails with chip-tool` helps
    the developer in narrowing down the problem

### Additional information

Providing additional information that can be helpful is encouraged. Each bug
report is different here. Some examples:

-   `This worked last week (around Jan 5) but started failing in recent master builds`

-   `Specification changed this attribute from optional to mandatory so this may be the cause of the issue`

-   `This issue may be related to #1234 as the same error is seen, however the reproduction steps seemed distinct enough that I opened a new issue`

-   `While running this, I observed 100% CPU before the operation finally timed out`

-   `While running this test, I observed device under test rebooting, logs attached.`

-   `This only happens intermittently - I see it about 30% of the time`

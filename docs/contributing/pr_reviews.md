# How Pull Request Reviews Work in the Matter SDK

## Who reviews PRs?

In general, **all reviews are welcome and encouraged**. The decision to merge a
pull request will take into account all feedback from reviews, including
comments and approvals.

The weight of an approval towards merging a PR depends on the reviewer's GitHub
handle:

-   Anyone can approve a PR. If the approver is not a member of the
    `project-chip` GitHub organization, their approval checkmark will be gray.
-   If the approver is part of the `project-chip` organization, the checkmark
    will be green; however, this does not inherently carry additional weight.
-   If the approver is part of one of the `reviewers-<company>` groups in
    [project-chip teams](https://github.com/orgs/project-chip/teams?query=reviewers-),
    their approval generally counts as a "company approval" (see "When do PRs
    merge" below).
-   A separate group,
    [matter-sdk-maintainers](https://github.com/orgs/project-chip/teams/matter-sdk-maintainers),
    exists, and its members are specifically empowered to merge PRs that meet
    the SDK quality bar.

While PRs are generally reviewed, contributions to the SDK are mostly made by
CSA members. We encourage you to ask for reviews in the appropriate CSA Slack
channels once a PR is ready.

To facilitate reviews, remember that your own review time is valuable.
Performing a quality review takes time. You are encouraged to help with the
review load, which will encourage others to review your PRs.

## When do PRs merge?

-   Pull requests must meet the
    [pull request guidelines](pull_request_guidelines.md). Reviewers will
    enforce this.
-   Pull requests must pass CI. A PR should have a green CI status before it is
    reviewed.
    -   We use several linters to catch common errors. Make sure to fix any
        linter errors.
-   Pull requests in `draft` status may not be reviewed, though some informal
    reviews may still occur.
-   Merge blocks are always respected:
    -   All comments must be resolved before a PR can be merged.
    -   A `Changes requested` status on a PR is always respected and must be
        removed before a PR can be merged. This is typically done by the person
        who requested the changes.

### Additional Rules

Some directories and platforms have additional requirements, some of which are
automated. All will be validated by reviewers:

-   Changes to `darwin` require a reviewer from Apple (an approval from
    `reviewers-apple`).
-   `/examples` can have their own reviewer list. An approval from a reviewer on
    that list can lead to a merge. For example, PRs for `examples/chef` will be
    merged if a member of `reviewers-google` approves it.

### SDK Maintainers

[Matter SDK Maintainers](https://github.com/orgs/project-chip/teams/matter-sdk-maintainers)
are explicitly empowered to merge a PR when it meets the quality bar.

SDK maintainers will:

-   Validate that the guidelines and quality bar are met.
-   Ensure sufficient review coverage (including review by the maintainer, if
    applicable) to maintain a high-quality bar.
-   Explicitly **push back on time pressure and will not lower the quality bar
    to meet deadlines**.
-   Prioritize quality over the PR author's convenience.
    -   Historically, more code has been written than there is bandwidth to
        review.
    -   They may ask for PRs to be split, sequenced, or for tests to be added,
        even if this is extra work for the PR author.
    -   They will use their best judgment on what needs to be addressed
        immediately versus what can be deferred to a follow-up.

SDK maintainers are explicitly empowered to mark any PR for merging via an
`sdk-maintainer-approved` label. This includes PRs authored by someone from the
same company, with some exceptions:

-   Large architectural changes should be reviewed by several companies or
    discussed in advance.
-   A completely new approach may require reviews from several companies.

### Multi-Company Reviews

Currently, a PR will be merged if two independent company representatives have
approved it and all other conditions (such as CI passing, no open comments, and
no blockers) are met.

### `fast-track`

A `fast-track` label exists for SDK leads to use for merging trivial changes or
good changes that have not yet received reviews.

This label was created before `matter-sdk-maintainers` existed, and its usage is
now expected to be **very infrequent**.

### Admin Merges

In some cases, administrator-forced merges may be appropriate or required. These
should be infrequent. Some examples include:

-   CI is broken and is either unfixable in a timely manner or waiting for a
    multi-hour CI run is not practical.
-   Hotfixes to unblock other users.
-   Rollbacks of breakages where the rollback is clean and we do not want to
    wait for CI.

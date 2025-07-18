# How Pull Requests reviews work in the Matter SDK

## Who reviews PRs

In general **all reviews are welcome and encouraged**! Other reviewers and decision to
merge a pull requests will take into account all feedback and signals from reviews
(both comments on improvements and checkmarks of approval as a signal that someone has
looked at a particular PR).

When someone reviews a PR, the weight of the approval towards PR merging will depend
on github handle:

- anyone can approve a PR. if not part of `project-chip` organization, the checkmark
  in github will appear as grey
- If the approver is part of project-chip organization, the checkmark will appear
  as `green` however by default it does not carry additional weight
- if the approver is part of one of the `reviewers-<company>` groups in
  [project-chip teams](https://github.com/orgs/project-chip/teams?query=reviewers-)
  they generally count as "company approals" (see below on when PRs merge)
- a separate [matter-sdk-maintainers](https://github.com/orgs/project-chip/teams/matter-sdk-maintainers)
  exist which are specifically empowered to merge PRs if they meet the SDK quality
  bar (see below on when PRs merge)

PRs will generally be reviewed, however since contributions to the SDK are mostly
made by CSA members, people are encouraged to ask for reviews in CSA Slack channels
once a PR is ready for review.

To facilitate reviews, consider that your own review time is valuable! Performing
a quality review takes time - you are encouraged to help with the review load and
this will encourage others to help your PRs as well.

## When do PRs merge

- Pull requests must meet the [pull request guidelines](./contributing/pull_request_guidelines.md). Reviwers will require this.
- Pull requests must pass CI (CI should be green for a PR to be reviewed)
  - we have several linters for common errors. Make sure linter errors are fixed
- Pull requests in `draft` may not be reviewed (however some drive-by reviews may happen)
- Merge blocks are always respected:
  - All comments must be resolved before a PR can merge
  - `Changes requested` on a PR is always respected and must be removed before a PR merges.
    This removal is often done by whoever asked for the changes with infrequent exceptions
    (e.g. done by AI, or very obviously stale like in the case of vacations)

### Explicit extra rules

Some directories/platforms have additional requirements (some are automated, all will be
validate by reviewers):

- changes to `darwin` require a reviewer from Apple (approval from `reviewers-apple`)
- `/examples` can have their own reviewer list, reviewers from that list will result in
  PR merge or automatic approval from a SDK reviewer. This is in progress, currently
  `examples/chef` will merge if `reviewers-google` checkmarks it.

### SDK Maintainers

[Matter SDK Maintainers](https://github.com/orgs/project-chip/teams/matter-sdk-maintainers) are
explicitly empowered to make decisions when a PR meets the quality bar to merge. SDK
maintainers will:

- validate that the guidelines and quality bar is met
- validate that sufficient review coverage (including review by the maintainer if applicable)
  to maintain a high quality bar
- explicitly **push back on time-pressure (will not lower quality bar due to deadlines)**
- prioritize quality over PR author convenience
  - historically we can write more code than review bandwidth exists
  - will ask for PR splitting/sequencing/add testing, even if this is extra work for the
    PR author
  - will use best judgement on what needs addressing now vs what should be deferred
    as a followup

SDK maintainers are explicitly empowered to mark any PR towards merging via a
`sdk-maintainer-approved` label, including for same-company authored commits in most cases, with
some exceptions (judgement call here as well):

- large architectural changes should be reviewed by several companies / discussed in advance
- anything that feels "completely new approach" may consider several company reviews

### Multi-company reviews

PRs will currently merge if 2 independent company representatives have approved the PR
(and all other conditions like CI passing, no open comments, no blockers) are met.

### `Fast-track`

A separate `fast-track` label exists for SDK leads usage, to merge something that seems
trivial or is a good change without being able to receive reviews.

This label was created before `matter-sdk-maintainers` existed and at this point its
usage is expected to be **very infrequent**

### Admin merges

In some cases administrator merges may be appropriate/required. These should be infrequent,
some cases can be:

- CI is broken and either is unfixable (docker builds are currently like this) or we
  do not want to wait for the multi-hour CI for a commit to unblock things
- Hotfixes to unblock other users
- Rollbacks of breakates (where the rollback is clean and we do not want to wait for CI)

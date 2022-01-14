## Matter Project Flow

This section is intended to cover how Matter uses GitHub Projects, Issues,
Milestones, Releases, and Branches for program/project management in the code
repository.

### Issues

Matter uses issues as simple problem descriptions or feature requests. In
general, all work contributed to the repository in the form of pull requests
(PR) should be under the auspices of some open issue. This may seem onerous and
in some cases duplicative, so consider these guidelines when deciding whether
you can get away with not creating an issue:

-   Trivial fixes: issues can function as TODO lists, simple reminders that
    something should be addressed. Sometimes, though, the work required to fix
    is smaller than the work required to write the issue.
-   Issues intended to be addressed by a PR may not actually be fixed or may
    regress.
-   Issues can span PRs (as PRs should be as small as possible, but no smaller).
-   Issues help form an important basis for release notes. Any PR that addresses
    a problem that should have release visibility, please do open an issue.

### Pull requests

Pull requests should be small and address a single, specific change to the code
base. They should be easy to review, as a "yes, that's better". Refrain from
requesting review until all PR checks have completed successfully, lest you tire
your reviewers.

PR Don'ts:

-   Don't combine unrelated changes. E.g. if the PR addresses a bug in some C
    code, an update to the top-level .gitignore doesn't belong.
-   Don't make stacks. E.g. if a change in a component requires a new feature or
    even a small tweak in one or more of its dependencies, each dependency
    change belongs in its own separate PR.

### Milestones

In Matter parlance, a milestone is simply a tag for an expected due date or
release. Milestones are intended to help contributors and their managers to
prioritize work. There are 2 types: Date-based and Release-based.

#### Date-based

Date-based milestones are named for their due date, typically a Friday of some
week. Date-based milestones are normally assigned based on a guess about when
something's likely to bubble up and get done based on current work load and
resourcing. They are wishes, guesses.

#### Release-based

Release-based milestones are named for the release version and may have flexible
or subject-to-change due dates. Release-based milestones are intended to track
release blockers.

A special "Not sure when" milestone is a marker for issues whose priority,
scope, or blocking status have not been determined. Monthly review of these is a
project goal.

Issues without milestones are those that have yet to be considered for one of
the above. Weekly review of new issues is a project goal.

### Projects

Projects are collections of issues, pull requests, and notes intended to capture
larger efforts that don't fit in issues, have multiple-subsystems involved, or
may span multiple milestones. We use projects 2 ways:

1. To track burn down on a larger task. When constructing such a project, it's
   important to think in terms of something that will eventually have an end,
   i.e. a definite scope.
2. To categorize issues, denote broader efforts without a definite time scope.
   These projects might reflect or show burndown or percent complete, but are
   mostly used to view where effort is going.

Issues can belong to any number of projects, but should generally only belong to
one of the task-tracking projects (the first type).

### Branches, releases, and general development flow

Master should always be Matter's best branch. Release branches, once cut, are
closed for any feature work. Software fixes for release branches must first land
on master unless demonstrably infeasible.

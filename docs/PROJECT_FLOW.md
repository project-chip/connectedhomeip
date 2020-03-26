## CHIP Project Flow

This section is intended to cover how CHIP uses GitHub Projects,
Issues, Milestones, Releases, and Branches for program/project
management in the code repository.

### Issues

CHIP uses issues as simple problem descriptions or feature requests.  All work
contributed to the repository in the form of pull requests should be under the
auspices of some open issue.

### Pull requests

Pull requests should be small and address a single, specific change to the code
base. They should be easy to review, as a "yes, that's better".  Refrain from
requesting review until all PR checks have completed successfully, lest you tire
your reviewers.

### Milestones

In CHIP parlance, a milestone is simply an expected due date or release.

Date-based milestones start with an "M" and have the form M3, M3.2, M4,
etc. corresponding to the number of months since January 1, 2020, and generally
mean the last Friday of that month.  E.g. M1 corresponds to EOB January 24,
2020.  The Morrow milestone is a special milestone that means "unprioritized" or 
"unscheduled" or "unscoped".

The form of the names of release-based milestone names are TBD, and may have
flexible or changing dates.

### Projects

Projects are collections of issues, pull requests, and notes intended to capture
larger efforts that don't fit in issues, have multiple-subsystems involved, or
may span multiple milestones.  We use projects 2 ways:

1. To track burn down on a larger task.  When constructing such a project, it's
important to think in terms of something that will eventually have
an end, i.e. a definite scope.
2. To categorize issues, denote broader efforts without a definite time scope. 
These projects might reflect or show burndown or percent complete, but are mostly 
used to view where effort is going.

Issues can belong to any number of projects, but should generally only belong to 
one of the task-tracking projects (the first type).

### Branches, releases, and general development flow

Master should always be CHIPs best branch.  Release branches, once cut, are
closed for any feature work.  Software fixes for release branches must first
land on master unless demonstrably infeasible.


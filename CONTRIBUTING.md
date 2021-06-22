# Contributing to Matter (formerly Project CHIP)

Want to contribute? Great! First, read this page (including the small print at
the end). By submitting a pull request, you represent that you have the right to
license your contribution to the Connectivity Standards Alliance and the community, and agree by
submitting the patch that your contributions are licensed under the
[Apache 2.0 license](./LICENSE). Before submitting the pull request, please make
sure you have tested your changes and that they follow the project guidelines
for contributing code.

# Contributing as an Open Source Contributor

As an open source contributor you can report bugs and request features in the
[Issue Tracker](https://github.com/project-chip/connectedhomeip/issues), as well
as contribute bug fixes and features that do not impact Matter
specification as a pull request. For example: ports of Matter to add APIs
to alternative programming languages (e.g. Java, JS), hardware ports, or an
optimized implementation of existing functionality. For features that impact the
specification, please join Matter work group within the Connectivity Standards Alliance.
The requirements to become an open source contributor of the
[Project CHIP Repository](https://github.com/project-chip/connectedhomeip) are:

-   Agree to the [Code of Conduct](./CODE_OF_CONDUCT.md)
-   Agree to the [License](./LICENSE)
-   Have signed the
    [Zigbee Project CHIP Working Group CLA](https://gist.github.com/clapre/65aa9fc63981da765039e0bb7e8701be)

# Contributing as a Connectivity Standards Alliance Project CHIP Working Group Member

As a participant of the Connectivity Standards Alliance Project CHIP Working Group, you can
attend Working Group meetings, propose changes to the Matter
specification, and contribute code for approved updates to the specification.
The requirements to become a member of the
[Project CHIP Repository](https://github.com/project-chip/connectedhomeip) are:

-   Must be a [Participant member](http://www.zigbeealliance.org/join) or higher
    of the Connectivity Standards Alliance
-   Must be a Project CHIP Working Group member
-   Have signed the Alliance Project CHIP Working Group CLA
-   Have approval from your company's official approver

# Bugs

If you find a bug in the source code, you can help us by
[submitting a GitHub Issue](https://github.com/project-chip/connectedhomeip/issues/new).
The best bug reports provide a detailed description of the issue and
step-by-step instructions for predictably reproducing the issue. Even better,
you can
[submit a Pull Request](https://github.com/project-chip/connectedhomeip/blob/master/CONTRIBUTING.md#submitting-a-pull-request)
with a fix.

# New Features

You can request a new feature by
[submitting a GitHub Issue](https://github.com/project-chip/connectedhomeip/issues/new).
If you would like to implement a new feature, please consider the scope of the
new feature:

-   _Large feature_: first
    [submit a GitHub Issue](https://github.com/project-chip/connectedhomeip/issues/new)
    and communicate your proposal so that the community can review and provide
    feedback. Getting early feedback will help ensure your implementation work
    is accepted by the community. This will also allow us to better coordinate
    our efforts and minimize duplicated effort.
-   _Small feature_: can be implemented and directly
    [submitted as a Pull Request](https://github.com/project-chip/connectedhomeip/blob/master/CONTRIBUTING.md#submitting-a-pull-request).

# Contributing Code

Project CHIP follows the "Fork-and-Pull" model for accepting contributions.

### Initial Setup

Setup your GitHub fork and continuous-integration services:

1. Fork the
   [Project CHIP repository](https://github.com/project-chip/connectedhomeip) by
   clicking "Fork" on the web UI.

2. All contributions must pass all checks and reviews to be accepted.

Setup your local development environment:

```bash
# Clone your fork
git clone git@github.com:<username>/connectedhomeip.git

# Configure upstream alias
git remote add upstream git@github.com:project-chip/connectedhomeip.git
```

### Submitting a Pull Request

#### Branch

For each new feature, create a working branch:

```bash
# Create a working branch for your new feature
git branch --track <branch-name> origin/master

# Checkout the branch
git checkout <branch-name>
```

#### Create Commits

```bash
# Add each modified file you'd like to include in the commit
git add <file1> <file2>

# Create a commit
git commit
```

This will open up a text editor where you can craft your commit message.

#### Upstream Sync and Clean Up

Prior to submitting your pull request, you might want to do a few things to
clean up your branch and make it as simple as possible for the original
repository's maintainer to test, accept, and merge your work.

If any commits have been made to the upstream master branch, you should rebase
your development branch so that merging it will be a simple fast-forward that
won't require any conflict resolution work.

```bash
# Fetch upstream master and merge with your repository's master branch
git checkout master
git pull upstream master

# If there were any new commits, rebase your development branch
git checkout <branch-name>
git rebase master
```

Now, it may be desirable to squash some of your smaller commits down into a
small number of larger more cohesive commits. You can do this with an
interactive rebase:

```bash
# Rebase all commits on your development branch
git checkout <branch-name>
git rebase -i master
```

This will open up a text editor where you can specify which commits to squash.

#### Push and Test

```bash
# Checkout your branch
git checkout <branch-name>

# Push to your GitHub fork:
git push origin <branch-name>
```

This will trigger the continuous-integration checks. You can view the results in
the respective services. Note that the integration checks will report failures
on occasion.

### Review Requirements

#### Documentation Best Practices

Project CHIP uses Doxygen to markup (or markdown) all C, C++, Objective C,
Objective C++, Perl, Python, and Java code. Read our
[Doxygen Best Practices, Conventions, and Style](https://github.com/project-chip/connectedhomeip/blob/master/docs/style/DOXYGEN.adoc)

#### Submit Pull Request

Once you've validated the CI results, go to the page for your fork on GitHub,
select your development branch, and click the pull request button. If you need
to make any adjustments to your pull request, just push the updates to GitHub.
Your pull request will automatically track the changes on your development
branch and update.

#### Merge Requirements

-   Github Workflows pass
-   Builds pass
-   Tests pass
-   Linting passes
-   Code style passes

When can I merge? After these have been satisfied, a reviewer will merge the PR
into master

#### Documentation

Documentation undergoes the same review process as code See the
[Documentation Style Guide](https://github.com/project-chip/connectedhomeip/blob/master/docs/STYLE_GUIDE.md)
for more information on how to author and format documentation for contribution.

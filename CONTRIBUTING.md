# Contributing to CHIP

Want to contribute? Great! First, read this page (including the small print at
the end). By submitting a pull request, you represent that you have the right to
license your contribution to Zigbee and the community, and agree by submitting
the patch that your contributions are licensed under the
[Apache 2.0 license](./LICENSE).

Before submitting the pull request, please make sure you have tested your
changes and that they follow the project
[guidelines for contributing code](#pull-request-requirements).

## Becoming a Member

Currently these are the requirements to becoming a member of the
[Project-CHIP Repository](https://github.com/project-chip/connectedhomeip)

-   Must be a member of the Zigbee Alliance CHIP TSG Working Group
-   Have signed the Zigbee CHIP WG CLA
-   Have signed up as a Tiger team member in one of the follow roles: Lead, Spec
    Writer, Developer (or exception granted), Code Approvers, or Support Staff
-   Have approval from your company's official approver

## Becoming a Contributor

Currently these are the requirements to becoming a member of the
[Project-CHIP Repository](https://github.com/project-chip/connectedhomeip)

-   Must be a member of the Zigbee Alliance CHIP TSG Working Group
-   Have signed the Zigbee CHIP WG CLA
-   Agree to the [Code of Conduct](./CODE_OF_CONDUCT.md)
-   Agree to the [License](./LICENSE)
-   Have signed up as a Tiger team member in one of the follow roles: Lead, Spec
    Writer, Developer (or exception granted), Code Approvers, or Support Staff
-   Have approval from your company's official approver

## Getting Started

This repository contains the source code that implements the CHIP specification.
It scalably implements the specification that may be used on a wide range of
platforms including Android/iOS and Darwin/Linux down to embedded MCU-based
platforms running FreeRTOS and LwIP.

The source code can be built to generate:

-   **Libraries** that can be built for iOS, Android or desktop (Linux/Mac)
    targets. These libraries could further be integrated into applications that
    talk _CHIP_.
-   **Firmwares/Embedded Applications** that can be built for the supported
    embedded platforms.
-   **Desktop Application** that can be used in conjunction with the embedded
    applications above to validate the end-to-end CHIP workflow.

### Building your first application

-   Building the firmware: This repository implements the CHIP specification on
    3 transports: 802.15.4 Thread, BLE and Wi-Fi. The examples/ directory
    contains example applications for all these 3 transports using 3 embedded
    platforms. Please visit their respective directories for instructions on how
    to build and deploy on these platforms.
    -   NRF5 (for 802.15.4 Thread): in
        [examples/lock-app/nrf5](examples/lock-app/nrf5)
    -   EFR32 (for BLE): in [examples/lock-app/efr32](examples/lock-app/efr32)
    -   ESP32 (for Wi-Fi or BLE): in
        [examples/wifi-echo/server/esp32](examples/wifi-echo/server/esp32)
-   Building the host utility: The host utility can be used in conjunction with
    the embedded platform for end-to-end validation. Please visit the
    [examples/chip-tool](examples/chip-tool) directory for further instructions.

### Where should I begin?

-   Good First Issue: Certain issues are marked with a label
    [Good First Issue](https://github.com/project-chip/connectedhomeip/issues?q=is%3Aissue+is%3Aopen+label%3A%22good+first+issue%22).
    These issues are what we believe may be good starting points for getting
    your hands dirty.
-   TODOs: Most items that will require work are captured in the GitHub issues
    of this project. This serves as a good TODO list for the next steps.
-   Milestones: A list of
    [milestones](https://github.com/project-chip/connectedhomeip/milestones) are
    maintained in this project. This should provide some idea of where things
    are headed. Note that given the early days of this project, most of these
    are not hard deadlines.

### Where is the spec?

-   The specification is evolving in various tiger teams. Members belonging to
    various tiger teams may contribute experimental code for the general
    direction where the spec is headed. Once a specification is fairly
    finalised, it will be available/committed in the [docs/specs](docs/specs).

## Bugs

If you find a bug in the source code, you can help us by
[submitting a GitHub Issue](https://github.com/project-chip/connectedhomeip/issues/new).
The best bug reports provide a detailed description of the issue and
step-by-step instructions for predictably reproducing the issue. Even better,
you can [submit a Pull Request](#submitting-a-pull-request) with a fix.

## New Features

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
    [submitted as a Pull Request](#submitting-a-pull-request).

## Contributing Code

CHIP follows the "Fork-and-Pull" model for accepting contributions.

### Initial Setup

Setup your GitHub fork and continuous-integration services:

1. Fork the [CHIP repository](https://github.com/project-chip/connectedhomeip)
   by clicking "Fork" on the web UI.

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
git checkout
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

#### Pull Request Requirements

CHIP considers there to be a few different types of pull requests:

-   Trivial bug fix
-   -   Decription 1
-   -   Decription 2
-   Small Bug fix
-   -   Decription 1
-   -   Decription 2
-   Bug Fix
-   -   Decription 1
-   -   Decription 2
-   Significiant Change
-   -   Decription 1
-   -   Decription 2
-   Feature
-   -   Decription 1
-   -   Decription 2
-   Architecture Change
-   -   Decription 1
-   -   Decription 2

### Prior to review, all changes require:

-   [GitHub Workflows](.github/workflows) pass
-   [Certification Tests](tests/certification/README.md) pass
-   [Fuzz Tests](tests/fuzz/README.md) pass
-   [Integration Tests](tests/integration/README.md) pass
-   Linting passes
-   Code style passes

### Review Requirements

Each type of change has unique additional requirements, here's a table of those:

| Type                | Reviewer Requirements                  | New Unit Tests | New Certification Tests | New Fuzz Tests | New Integration Tests |
| ------------------- | -------------------------------------- | -------------- | ----------------------- | -------------- | --------------------- |
| Trivial bug fix     | 3 [approved reviewers](./REVIEWERS.md) |                |                         |                |                       |
| Small Bug fix       | 3 [approved reviewers](./REVIEWERS.md) |                |                         |                |                       |
| Bug Fix             | 3 [approved reviewers](./REVIEWERS.md) |                |                         |                |                       |
| Significiant Change | 3 [approved reviewers](./REVIEWERS.md) |                |                         |                |                       |
| Feature             | 3 [approved reviewers](./REVIEWERS.md) |                |                         |                |                       |
| Architecture Change | 3 [approved reviewers](./REVIEWERS.md) |                |                         |                |                       |

Note: Where multiple reviewers are required, each reviewer must be from a
different member company.

#### Submit Pull Request

Once you've validated the CI results, go to the page for your fork on GitHub,
select your development branch, and click the pull request button. If you need
to make any adjustments to your pull request, just push the updates to GitHub.
Your pull request will automatically track the changes on your development
branch and update.

#### Merge Requirements

-   Passes [Review Requirements](#review-requirements)
-   [GitHub Workflows](.github/workflows) pass
-   [Certification Tests](tests/certification/README.md) pass
-   [Fuzz Tests](tests/fuzz/README.md) pass
-   [Integration Tests](tests/integration/README.md) pass
-   Linting passes
-   Code style passes

**When can I merge?** After these have been satisfied, any reviewer, or the
originator can merge the PR into master.

### Documentation

Documentation undergoes the same review process as code

See the [Documentation Style Guide](docs/STYLE_GUIDE.md) for more information on
how to author and format documentation for contribution.

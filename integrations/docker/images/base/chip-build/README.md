# Docker connectedhomeip:chip-build

project-chip/chip-build is the name of the Docker image used by CHIP for
continuous integration and other builds.

Contents of this directory:

-   build.sh - utility for building (and optionally) tagging and pushing the
    chip-build Docker image
-   version - the semver-style version of the image in use for this branch of
    CHIP
-   Dockerfile - description of the image

Please update version when any required tooling is updated. Some rough
guidelines:

-   Updating a tool? Increment dot version unless the tool has a major version
    delta or a backward incompatibility
-   Adding a tool? Increment minor version: e.g. 0.2.1 -> 0.3.0
-   Removing a tool? Increment major version: e.g. 1.2 -> 2.0

Note, you must have privileged access to the connectedhomeip:chip-build on
Docker Hub

Typical use:

1.  new build tool dependency identified
2.  add tool to Dockerfile
3.  update version
4.  `$ ./build.sh`, which installs the image locally as the new version
5.  update the image version in the devcontainer.json
6.  verify that the build works locally in the new image
7.  `$ docker login`
8.  `$ ./build.sh --push --latest`, _*Note:*_ omit `--latest` unless on the
    master branch

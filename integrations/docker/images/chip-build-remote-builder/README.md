# Docker `gcr.io:\$PROJECT_ID:remote-builder`

`gcr.io:\$PROJECT_ID:remote-builder` is the name of the Docker image used by
Matter for continuous integration and coverage statistics using Google Cloud
Platform.

Contents of this directory:

-   build.sh - utility for building (and optionally) tagging and pushing the
    remote-builder Docker image
-   cloudbuild.yaml - build config file contains instructions for Cloud Build to
    build, package, and push the remote-builder Docker image.
-   Dockerfile - description of the image

In order to use remote-builder, you need to first build the builder:

`./build.sh`

When using the remote-builder image, the following will happen:

1. A temporary SSH key will be created in your Container Builder workspace
2. SSH into a virtual machine instance with your configured flags
3. Your command will be run inside that compute engine instance's workspace
4. The workspace will be copied back to your Container Builder workspace

# ZAP

- This Dockerfile builds https://github.com/project-chip/zap.git, which gets copied into `chip-build-vscode`
- This image must be built in a working tree of the `CHIP` repo with a commit found in https://github.com/project-chip/connectedhomeip
- This image must be built using the included `build.sh`.
  - `build.sh` will check for the precense of `buildcontext.sh` in the build context and will execute the script before calling `docker build` if it exists.
- In the case of `zap`, `buildcontext.sh` will get the commit of the current working tree and store it in `context.txt`.
- At build time:
  - The `CHIP` repo will be cloned from https://github.com/project-chip/connectedhomeip
  - The commit found in the working tree by `buildcontext.sh` will be checked out.
  - The hash of the `zap` submodule and the hash written in `chip-build-zap/Dockerfile` will be extracted from the cloned repo.
  - `zapverify.sh` will compare whether the hashes match and log what was extracted.
    - If there is a discrepancy, it will provide instructions to update origin and fails the build.

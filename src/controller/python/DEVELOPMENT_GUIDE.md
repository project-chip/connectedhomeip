# Admin Flow

* CHIP SDK is maintained on a main branch `master` and release branches cut out for specific deliverables like `test_event2.2`
* CSG fork of the CHIP SDK is maintained with all existing main and release branches left untouched and pristine. 
* All CSG specific changes should be done on newly created branches (with a `csg/` prefix) based on these release branches. eg: `test_event2.2` -> `csg/test_event2.2`
* Only admins on the repo have access to pull in changes from the upstream repo and update all local main and release branches using the following commands or equivalent:
```bash
git fetch upstream
git pull upstream 
git push origin
``` 
* Once updated create a local version of the release branch with a csg prefix to add our changes. This branch will be the main merge target for all the PRs in this dev period.

# Development Flow

* Developers must create a branch with the following prefix `csg/fix` or `csg/feature` and submit their PRs to be merged to only the csg version of the release branch.
* If the admins upstream the branches, the developers must rebase their changes resolve their conflicts locally before submitting the PR
* Changes should be only additive, isolated and minimal, effort should be made to minimize the footprint on changes in the existing code.

# Building

* At the root directory:
```bash
source scripts/activate.sh
gn gen out/debug
ninja -C out/debug
```
* A .whl file is generate at `out/debug/controller/python/chip-0.0-cp37-abi3-linux_aarch64.whl`
* Copy this wheel file into the sample test client folder or into the test harness controller dockerfile folder.

# Testing with a real accessory
# Testing with virtual BLE accessory

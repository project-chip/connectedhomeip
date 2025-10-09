---
orphan: true
---

# Data model XML files

This folder contains a machine-readable representation of matter clusters.

These files are currently used for certification testing and documentation. The
data model files represent the official specification data model at a certain
revision. The files in these directories are maintained by the data model tiger
team (DMTT) and are updated when new official specifications or specification
ballots are released.

The data model files in this directory are not used with zap or for SDK codegen
and should not be updated manually by cluster or device type implementers.

## Updating the data model files

### Updating current spec revision directories

If the files exist in the data_model directory already and you are trying to
update either the revision or the alchemy release, use
scripts/spec_xml/generate_spec_xml.py. More information on the use of the script
can be found by using the --help flag for that script.

Perform the following steps

-   Check out the specification repo at the desired sha/tag/branch
    (https://github.com/CHIP-Specifications/connectedhomeip-spec)
-   Determine the desired include level (current ballot includes, no in-progress
    includes or all in-progress includes). If you are generating files using
    "Current", be sure to double-check the include list inside the script
    against the ballot email.
-   Run the script and check in all the file changes.

    -   Example script invocation (adjust environment variables to reflect your
        proper paths and spec revisions)

        ```
        ALCHEMY=/path/to/alchemy
        SPEC_ROOT=/path/to/spec/repo
        DM_DIR=1.3
        IN_PROGRESS=None
        ./scripts/spec_xml/generate_spec_xml.py --scraper $ALCHEMY --spec-root $SPEC_ROOT --include-in-progress $IN_PROGRESS --output-dir data_model/$DM_DIR
        ```

-   You can use the scripts/spec_xml/spec_revision_diff_summary.py script to
    create a summary with the differences between the data model files for two
    different directories
-   Be sure to double-check the summary and generated ID files to ensure the
    provisional markings are as expected
-   If you are updating both the spec SHA and the scraper version, it is best to
    do this in two separate commits so reviewers can see which changes come from
    the spec and which changes come from the scraper additions
-   Before pushing the PR, run build_python.sh, activate the venv and run all
    the TestSpec\* tests in src/python_testing. These are unit tests of the data
    model files.

    -   To do this (set VENV to your desired path), paths set as if you're
        running from the chip root

        ```
        . scripts/activate.sh
        VENV=out/py
        ./scripts/build_python.sh -i $VENV
        source $VENV/bin/activate
        python3 src/python_testing/TestSpecParsingSupport.py
        python3 src/python_testing/TestSpecParsingSelection.py
        python3 src/python_testing/TestSpecParsingDeviceType.py
        ```

### Adding data model files for a new revision

As with existing files, new data model files are created using
scripts/spec_xml/generate_spec_xml.py. More information on the use of the script
can be found by using the --help flag for that script.

Best practices for creating PRs for new data model revisions

-   Include the last revision (copied to the new directory name) as the first
    commit so reviewers can see the baseline changes
-   Add a commit using the same alchemy release as the prior revision
-   If the alchemy revision needs to be changed, if possible, do this in a
    separate commit
-   The scripts/spec_xml/spec_revision_diff_summary.py script to create a
    summary with the differences between the data model files for two different
    directories. This can be used as the PR description to give reviewers an
    understanding of the changes between the spec revisions

To do this, perform the following steps:

-   Copy the last revision data model directory into the new directory name and
    commit the changes. This allows reviewers to see a diff between the last and
    current spec revision.
-   Check out the specification repo at the desired sha/tag/branch
    (https://github.com/CHIP-Specifications/connectedhomeip-spec)
-   Determine the desired include level (current ballot includes, no in-progress
    includes or all in-progress includes). If you are generating files using
    "Current", be sure to double-check the include list inside the script
    against the ballot email.
-   Create the new data model files using scripts/spec_xml/generate_spec_xml.py.
    More information on the use of the script can be found by using the --help
    flag for that script.

    -   Example script invocation (adjust environment variables to reflect your
        proper paths and spec revisions)

        ```
        ALCHEMY=/path/to/alchemy
        SPEC_ROOT=/path/to/spec/repo
        DM_DIR=1.5
        IN_PROGRESS=Current
        ./scripts/spec_xml/generate_spec_xml.py --scraper $ALCHEMY --spec-root $SPEC_ROOT --include-in-progress $IN_PROGRESS --output-dir data_model/$DM_DIR
        ```

-   Run build_python.sh, activate the venv and run all the TestSpec\* tests in
    src/python_testing. These are unit tests of the data model files.
-   If there are errors, prefer to correct them in the spec, or by updating the
    errata file in the spec repo
-   Once the files have been properly generated, commit all the changes
-   Generate a PR description using
    scripts/spec_xml/spec_revision_diff_summary.py script to create a summary
    with the differences between the data model files for two different
    directories.
-   In the summary generated from spec_revision_diff_summary.py, be sure to
    double-check the following
    -   All expected provisional elements are still marked as provisional
    -   There are no unexpected provisional elements
    -   Changes to clusters and device types are accompanied by a revision
        update
    -   All the changes between the two revisions are expected
-   Be sure to double-check the summary and generated ID files to ensure the
    provisional markings are as expected
-   To update the parsing support to use the new data model files:
    -   Update src/python_testing/matter_testing_infrastructure/BUILD.gn to
        create and include a zip of the new files. The gni with the file list
        (src/python_testing/matter_testing_infrastructure/data_model_xmls.gni)
        is updated as a part of the generate_spec_xml.py script, so it is just
        the zip file inclusion in this file that needs to be updated
    -   In
        src/python_testing/matter_testing_infrastructure/matter/testing/spec_parsing.py
        update the PrebuiltDataModelDirectory enum to add the new directory and
        dm_from_spec_version
    -   Add unit tests for the new data model files to the data model file unit
        tests in src/python_testing:
        -   TestSpecParsingDeviceType.py
        -   TestSpecParsingSelection.py
        -   TestSpecParsingSupport.py
-   To update the SDK to the new spec revision, update
    src/app/SpecificationDefinedRevisions.h
-   Add the new data model files to the github CI data model revision checker in
    .github/workflows/check-data-model-directory-updates.yaml

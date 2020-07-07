# CHIP on-device test dispatch

This document expands on and provides design for on-device test dispatch. The
CHIP on-device testing document states that dispatching should involve a HTTPS
based RESTful protocol that could be integrated with CircleCI.

## Definitions

**Test run**: Tests instantiation on a test host, consisting of host-side test
binaries and test scripts as well as one or more device-side binaries.

**Test host**: A computing execution environment provided for the purposes of
running tests by a party participating in the scheme.

## Scope

The scope of this proposal is to support running tests against a known set of
canonical devices and platforms for the purposes of developing core common code
in the CHIP project GitHub repository.

This proposal does not preclude a stakeholder running their own tests against
their own hardware or lab in any way they see fit. The goal is merely to provide
a common way for volunteer organizations to register test infrastructure and
dispatch capabilities to be used by the wider CHIP developer community.

Authentication is not considered strictly part of the test dispatch protocol.
However it is mandated that some form of authentication takes place before any
of the test dispatch operations are issued. Throughout this document it is
assumed that proper authentication and authorization is ensured by the server
hosting the test dispatch service.

## Objectives

-   **Provide a centralized API** for the dispatching of tests against
    potentially distributed lab infrastructure, which CI systems (i.e. CircleCI)
    or individual developers may not be directly configured to access.
-   **Decouple test execution from the CI environment**, especially for tests
    requiring complex setups, such as radio traffic capture.
-   **Enable** common adoption of **aligned methodologies** for both
    certification-style tests and development-support tests (pre/post-submit
    testing in pull requests).

### Certification or pre-certification tests

Certification tests are required to have canonical test suite names.

Here the host side test binaries and scripts are fixed and the device-side
binary can vary by device type. The objective of certification testing is to run
a known fixed set of tests against new and existing devices. Dispatching of
certification tests involves specifying the canonical test suite name and
providing the requisite arguments, such as device-side binary and device type.

### Development support tests

Development support test suites are required to have canonical names, but they
may, during execution, check-out the actual test script from a given PR, or from
the artifacts uploaded for the test job.

The test is executed against a pull request and may target many device types.
Therefore, both host-side and device-side artifacts may vary and have to be
uploaded in the respective argument to test dispatch operation. Dispatching of
development support test suites therefore involves specifying a canonical test
suite name, the PR URL, pre-built artifacts (host side and device-side) and
optional test-specific arguments.

### Common constraints for dispatch

In order to support running tests, some common arguments are required to
determine during dispatch whether a given combination of targets can be
supported.

These constraints include:

-   A canonical device type list to determine whether a target runner has all
    the targets needed. (Note that new hardware developers may provide a
    non-canonical device type for running their own certification on their own
    lab. Canonical device types exist for development support tests.)
-   An optional node ID (unique UUID) to force execution on a given registered
    infrastructure for test purposes.

Example of canonical test suite names:

-   RendezVousTest: loads binaries on HW, validates that assumptions about
    RendezVous advertising payload are still valid.
-   BasicCHIPRegression: loads binaries on HW, validates against regressions on
    multiple axes of the test. Potentially runs updated tests scripts from the
    PR itself.

## Aggregator Dispatch Interface API

We conceptualize an aggregator service where all the tests are sent to be
further dispatched to or pulled by participating infrastructure/lab providers.

For the prototype phase the aggregator may be the same service that runs tests
as well, i.e. further dispatch/pull/registration may not happen in the
prototype.

This is the API which CircleCI and individual test developers can use. There may
be other APIs (e.g. administrator API) to the aggregator that provide richer
functionality. For now we don't discuss those. The API for communication between
the aggregator and test labs is to be specified at a later time.

The goal of decoupling dispatch from execution is to avoid coupling running the
tests to a given lab’s or organization’s infrastructure. The dispatch interface
API provides a separation of concerns of “what to run”/“what happened” versus
“how to run”/“where to run”.

### Resources and operations

/available_test_suites - Collection resource, all the canonical test suite
names.

-   GET gets the list of known canonical test suite names

/dispatch - Collection resource, all the currently running test executions.

-   POST dispatches a new test, returning its URI with the test run identifier
    'job_id'. - Arguments - Canonical Test Suite name e.g.
    "CHIP_basic_test_suite" - ZIP file upload of artifacts (device-side and, if
    needed, host-side) - Some common required inputs (see section below) -
    Test-suite-specific configuration/contents may also be provided for the test
    suite executor to use. - Maximum time the client is willing to wait for
    results (seconds) - In case of execution timed out, the test job would be
    considered FAILED, due to time-out. - Maximum time the client is willing to
    wait for test to start (seconds) - In case of time-out to dispatch, the test
    job would be considered ABORTED in the results as opposed to FAILED -
    Authentication requirements may cause a given requestor to be throttled by
    internal logic.

/status/<job_id> - Member resource, an individual test.

-   GET returns the status of the test: scheduled, running, finished, aborted.
-   DELETE cancels the test.

/status/<job_id>/results - Collection resource, all the files resulting from the
test run.

-   GET returns the list of (file_name, file_id) pairs.
    -   Only mandatory file:
        -   test_suite_results.json
            -   Normalized test results, see section below.

/status/<job_id>/results/<file_id> - Member resource, and individual result
file.

-   GET returns the contents of the file.

### /dispatch arguments

**test_suite_name**: _Required_. Name of the test suite.

**host_artifacts**: _Only required for development support tests_. A file (most
likely a ZIP file) corresponding to all the scripts and binaries to be run by
the test host. The test host must be able to unzip, recognize and execute
contents.

**device_artifacts**: _Required_. A file (most likely a ZIP file) corresponding
to all the binaries to be flashed on the device. The test host must be able to
unzip, recognize and flash contents.

**timeout_for_results_seconds**: _Required_. The maximum amount of time in
seconds the client is willing to wait for results. After this much time the test
can be killed by the test host.

**timeout_for_start_seconds**: _Required_. The maximum amount of time in seconds
the client is willing to wait for the test to start. If after dispatch the test
does not start after this much time the test can be descheduled by the
aggregator.

### test_suite_results.json contents

TBD.

### Aggregator-to-lab dispatch API

TBD.

### Lab Registration Interface API

Once agreement on the dispatch API is cemented, the API to register a given
executor with certain tests and devices capabilities can be defined.

TBD.
